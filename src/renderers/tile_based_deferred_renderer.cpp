//
// Tile-Based Deferred Renderer (TBDR)
// -----------------------------------
// 本文件实现 CPU 侧的基于 Tile 的延迟着色(TBDR)。整体思路与现代 TBDR 硬件类似：
// 1) 先将三角形按屏幕划分到 Tile（binning）；
// 2) 对每个 Tile，进行“两阶段”光栅化：
//    - 阶段A：仅进行深度决胜（Z 预通过）——找出每个像素的“胜出三角形”和其最小深度，并缓存透视矫正后的重心权重；
//    - 阶段B：仅对胜出的像素执行一次片元着色（FragmentShader），写回 Tile 局部缓冲，然后整 Tile 拷贝到全局。
//
// 与现有 TBR（Tile-Based 前向渲染）相比，TBDR 避免了“对被随后覆盖的像素进行无用的着色”，在overdraw较多时显著减少
// FragmentShader 调用次数；同时保持 Tile‑major 的访问局部性与单份全局 frame buffer 的并发安全写回。

#include "renderers/tile_based_deferred_renderer.hpp"

#include <omp.h>

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstring>
#include <limits>

#include "config.h"
#include "log_system.h"

namespace simple_renderer {

bool TileBasedDeferredRenderer::Render(const Model& model, const Shader& shader_in,
                                       uint32_t* buffer) {
  auto total_start_time = std::chrono::high_resolution_clock::now();
  auto shader = std::make_shared<Shader>(shader_in);
  shader->PrepareUniformCaches();

  // 顶点阶段（SoA）
  // - 统一进行裁剪空间->NDC->屏幕空间的变换，并将结果写入 SoA。
  // - 此阶段与 TBR 完全一致。
  auto vertex_start = std::chrono::high_resolution_clock::now();
  const auto& input_vertices = model.GetVertices();
  VertexSoA soa; soa.resize(input_vertices.size());

#pragma omp parallel for num_threads(kNProc) schedule(static) shared(shader, soa, input_vertices)
  for (size_t i = 0; i < input_vertices.size(); ++i) {
    const auto& v = input_vertices[i];
    auto clip = shader->VertexShader(v);
    soa.pos_clip[i] = clip.GetPosition();
    auto ndc = PerspectiveDivision(clip);
    auto screen = ViewportTransformation(ndc);
    soa.pos_screen[i] = screen.GetPosition();
    soa.normal[i] = screen.GetNormal();
    soa.uv[i] = screen.GetTexCoords();
    soa.color[i] = screen.GetColor();
  }
  auto vertex_end = std::chrono::high_resolution_clock::now();
  double vertex_ms = std::chrono::duration_cast<std::chrono::microseconds>(vertex_end - vertex_start).count() / 1000.0;

  // Tile Binning
  // - 将三角形按屏幕空间包围盒映射到 Tile 网格；
  // - 使后续处理以 Tile 为并行单元，避免跨 Tile 写冲突；
  // - 仍复用现有 TBR 的数据结构与Binning逻辑。
  auto setup_start = std::chrono::high_resolution_clock::now();
  const size_t TILE_SIZE = tile_size_ > 0 ? tile_size_ : 64;
  const size_t tiles_x = (width_ + TILE_SIZE - 1) / TILE_SIZE;
  const size_t tiles_y = (height_ + TILE_SIZE - 1) / TILE_SIZE;
  const size_t total_tiles = tiles_x * tiles_y;
  std::vector<std::vector<TileTriangleRef>> tile_triangles(total_tiles);
  auto setup_end = std::chrono::high_resolution_clock::now();
  double setup_ms = std::chrono::duration_cast<std::chrono::microseconds>(setup_end - setup_start).count() / 1000.0;

  auto bin_start = std::chrono::high_resolution_clock::now();
  TileGridContext grid_ctx{soa, tiles_x, tiles_y, TILE_SIZE};
  TriangleTileBinning(model, grid_ctx, tile_triangles);
  auto bin_end = std::chrono::high_resolution_clock::now();
  double bin_ms = std::chrono::duration_cast<std::chrono::microseconds>(bin_end - bin_start).count() / 1000.0;

  // 全局 framebuffer（单份）
  // - 每个 Tile 完成后，整行拷贝到这份全局缓冲；
  // - 不同 Tile 不重叠，省去同步/锁开销。
  auto buf_alloc_start = std::chrono::high_resolution_clock::now();
  std::unique_ptr<float[]> depthBuffer = std::make_unique<float[]>(width_ * height_);
  std::unique_ptr<uint32_t[]> colorBuffer = std::make_unique<uint32_t[]>(width_ * height_);
  std::fill_n(depthBuffer.get(), width_ * height_, kDepthClear);
  std::fill_n(colorBuffer.get(), width_ * height_, kColorClear);
  auto buf_alloc_end = std::chrono::high_resolution_clock::now();
  double buf_alloc_ms = std::chrono::duration_cast<std::chrono::microseconds>(buf_alloc_end - buf_alloc_start).count() / 1000.0;

  // 并行按 tile 渲染：两阶段（Z 决胜 -> 着色）
  // - OpenMP 以 Tile 为单位并行；
  // - Tile 内先进行“Z 预通过”（不着色），再统一“按像素胜者着色”。
  auto raster_start = std::chrono::high_resolution_clock::now();
  std::vector<TileMaskStats> tile_stats(total_tiles);

#pragma omp parallel num_threads(kNProc) default(none) \
    shared(tile_triangles, grid_ctx, depthBuffer, colorBuffer, shader, total_tiles, tile_stats)
  {
    std::unique_ptr<float[]> tile_depth_buffer = std::make_unique<float[]>(grid_ctx.tile_size * grid_ctx.tile_size);
    std::unique_ptr<uint32_t[]> tile_color_buffer = std::make_unique<uint32_t[]>(grid_ctx.tile_size * grid_ctx.tile_size);

#pragma omp for schedule(static)
    for (size_t tile_id = 0; tile_id < total_tiles; ++tile_id) {
      uint64_t tested = 0, covered = 0, winners = 0, shaded = 0;
      // 2-pass 的核心逻辑在 RasterizeTileDeferred 内：
      //   A) 仅计算覆盖与深度，确定每像素胜者（三角形索引）并缓存透视矫正重心；
      //   B) 对胜者像素一次性着色写回，最后整 Tile 拷贝到全局。
      RasterizeTileDeferred(tile_id, tile_triangles[tile_id], grid_ctx,
                            tile_depth_buffer.get(), tile_color_buffer.get(),
                            depthBuffer, colorBuffer, *shader,
                            &tested, &covered, &winners, &shaded);
      tile_stats[tile_id].tested = tested;
      tile_stats[tile_id].covered = covered;
      tile_stats[tile_id].zpass = winners; // 在 TBDR 中 zpass≈winner 数
      tile_stats[tile_id].shaded = shaded;
    }
  }
  auto raster_end = std::chrono::high_resolution_clock::now();
  double raster_ms = std::chrono::duration_cast<std::chrono::microseconds>(raster_end - raster_start).count() / 1000.0;

  // 汇总统计
  uint64_t sum_tested = 0, sum_covered = 0, sum_winners = 0, sum_shaded = 0;
  for (const auto& s : tile_stats) {
    sum_tested += s.tested;
    sum_covered += s.covered;
    sum_winners += s.zpass;
    sum_shaded += s.shaded;
  }
  auto rate = [](uint64_t num, uint64_t den) -> double { return (den == 0) ? 0.0 : double(num) / double(den) * 100.0; };
  SPDLOG_DEBUG("TBDR Stats: tested={}, covered={} ({:.1f}%), winners={} ({:.1f}%), shaded={} ({:.1f}%)",
               sum_tested, sum_covered, rate(sum_covered, sum_tested),
               sum_winners, rate(sum_winners, sum_covered),
               sum_shaded, rate(sum_shaded, sum_covered));

  // 拷贝到输出
  auto present_start = std::chrono::high_resolution_clock::now();
  std::memcpy(buffer, colorBuffer.get(), width_ * height_ * sizeof(uint32_t));
  auto present_end = std::chrono::high_resolution_clock::now();
  double present_ms = std::chrono::duration_cast<std::chrono::microseconds>(present_end - present_start).count() / 1000.0;

  auto total_end_time = std::chrono::high_resolution_clock::now();
  double total_ms = std::chrono::duration_cast<std::chrono::microseconds>(total_end_time - total_start_time).count() / 1000.0;

  SPDLOG_DEBUG("=== TILE-BASED DEFERRED RENDERING PERFORMANCE ===");
  double sum_ms = vertex_ms + (total_ms - vertex_ms);
  SPDLOG_DEBUG("Vertex Shader:    {:8.3f} ms ({:5.1f}%)", vertex_ms, vertex_ms / sum_ms * 100);
  SPDLOG_DEBUG("Setup:            {:8.3f} ms", setup_ms);
  SPDLOG_DEBUG("Binning:          {:8.3f} ms", bin_ms);
  SPDLOG_DEBUG("Buffer Alloc:     {:8.3f} ms", buf_alloc_ms);
  SPDLOG_DEBUG("Tile Raster:      {:8.3f} ms", raster_ms);
  SPDLOG_DEBUG("Copy:             {:8.3f} ms", present_ms);
  SPDLOG_DEBUG("Total:            {:8.3f} ms", vertex_ms + (setup_ms + bin_ms + buf_alloc_ms + raster_ms + present_ms));
  SPDLOG_DEBUG("===============================================");

  return true;
}

void TileBasedDeferredRenderer::TriangleTileBinning(
    const Model& model, const TileGridContext& grid,
    std::vector<std::vector<TileTriangleRef>>& tile_triangles) {
  const size_t total_triangles = model.GetFaces().size();
  SPDLOG_DEBUG("Starting triangle-tile binning (SoA) for {} triangles", total_triangles);
  SPDLOG_DEBUG("Screen dimensions: {}x{}, Tile size: {}, Tiles: {}x{}", width_, height_, grid.tile_size, grid.tiles_x, grid.tiles_y);

  std::vector<size_t> tile_counts(grid.tiles_x * grid.tiles_y, 0);
  for (size_t tri_idx = 0; tri_idx < total_triangles; ++tri_idx) {
    ProcessTriangleForTileBinning(tri_idx, true, model, grid, tile_counts, tile_triangles);
  }
  for (size_t tile_id = 0; tile_id < tile_triangles.size(); ++tile_id) {
    if (tile_counts[tile_id] > 0) tile_triangles[tile_id].reserve(tile_counts[tile_id]);
  }
  for (size_t tri_idx = 0; tri_idx < total_triangles; ++tri_idx) {
    ProcessTriangleForTileBinning(tri_idx, false, model, grid, tile_counts, tile_triangles);
  }

  size_t total_refs = 0, non_empty = 0;
  for (const auto& v : tile_triangles) { total_refs += v.size(); if (!v.empty()) non_empty++; }
  SPDLOG_DEBUG("  (SoA) Total triangle references: {}", total_refs);
  SPDLOG_DEBUG("  (SoA) Non-empty tiles: {}", non_empty);
  SPDLOG_DEBUG("  (SoA) Average triangles per tile: {:.2f}", total_refs > 0 ? float(total_refs) / tile_triangles.size() : 0.0f);
}

void TileBasedDeferredRenderer::ProcessTriangleForTileBinning(
    size_t tri_idx, bool count_only, const Model& model, const TileGridContext& grid,
    std::vector<size_t>& tile_counts,
    std::vector<std::vector<TileTriangleRef>>& tile_triangles) {
  const auto& f = model.GetFaces()[tri_idx];
  size_t i0 = f.GetIndex(0), i1 = f.GetIndex(1), i2 = f.GetIndex(2);

  // 视锥体裁剪（裁剪空间保守裁剪）
  const Vector4f &c0 = grid.soa.pos_clip[i0];
  const Vector4f &c1 = grid.soa.pos_clip[i1];
  const Vector4f &c2 = grid.soa.pos_clip[i2];
  bool frustum_cull =
      (c0.x > c0.w && c1.x > c1.w && c2.x > c2.w) ||
      (c0.x < -c0.w && c1.x < -c0.w && c2.x < -c0.w) ||
      (c0.y > c0.w && c1.y > c1.w && c2.y > c2.w) ||
      (c0.y < -c0.w && c1.y < -c0.w && c2.y < -c0.w) ||
      (c0.z > c0.w && c1.z > c1.w && c2.z > c2.w) ||
      (c0.z < -c0.w && c1.z < -c0.w && c2.z < -c0.w);
  if (frustum_cull) return;

  const Vector4f &pos0 = grid.soa.pos_screen[i0];
  const Vector4f &pos1 = grid.soa.pos_screen[i1];
  const Vector4f &pos2 = grid.soa.pos_screen[i2];

  // 背面剔除（屏幕空间叉积）
  Vector2f screen0(pos0.x, pos0.y), screen1(pos1.x, pos1.y), screen2(pos2.x, pos2.y);
  Vector2f edge1 = screen1 - screen0, edge2 = screen2 - screen0;
  float cross_product = edge1.x * edge2.y - edge1.y * edge2.x;
  if (cross_product > 0.0f) return;

  // tile 覆盖范围
  float min_x = std::min({pos0.x, pos1.x, pos2.x});
  float max_x = std::max({pos0.x, pos1.x, pos2.x});
  float min_y = std::min({pos0.y, pos1.y, pos2.y});
  float max_y = std::max({pos0.y, pos1.y, pos2.y});

  int start_tile_x = std::max(0, static_cast<int>(min_x) / static_cast<int>(grid.tile_size));
  int end_tile_x   = std::min(static_cast<int>(grid.tiles_x - 1), static_cast<int>(max_x) / static_cast<int>(grid.tile_size));
  int start_tile_y = std::max(0, static_cast<int>(min_y) / static_cast<int>(grid.tile_size));
  int end_tile_y   = std::min(static_cast<int>(grid.tiles_y - 1), static_cast<int>(max_y) / static_cast<int>(grid.tile_size));
  if (start_tile_x > end_tile_x || start_tile_y > end_tile_y) return;

  if (count_only) {
    for (int ty = start_tile_y; ty <= end_tile_y; ++ty)
      for (int tx = start_tile_x; tx <= end_tile_x; ++tx)
        tile_counts[ty * grid.tiles_x + tx]++;
  } else {
    TileTriangleRef tri_ref{i0, i1, i2, &f.GetMaterial(), tri_idx};
    for (int ty = start_tile_y; ty <= end_tile_y; ++ty)
      for (int tx = start_tile_x; tx <= end_tile_x; ++tx)
        tile_triangles[ty * grid.tiles_x + tx].push_back(tri_ref);
  }
}

void TileBasedDeferredRenderer::RasterizeTileDeferred(
    size_t tile_id, const std::vector<TileTriangleRef>& triangles,
    const TileGridContext& grid, float* tile_depth_buffer, uint32_t* tile_color_buffer,
    std::unique_ptr<float[]>& global_depth_buffer, std::unique_ptr<uint32_t[]>& global_color_buffer,
    const Shader& shader, uint64_t* out_tested, uint64_t* out_covered, uint64_t* out_winners, uint64_t* out_shaded) {
  // 计算本 Tile 覆盖的屏幕区域（半开区间对齐到闭区间扫描）
  size_t tile_x = tile_id % grid.tiles_x;
  size_t tile_y = tile_id / grid.tiles_x;
  size_t screen_x_start = tile_x * grid.tile_size;
  size_t screen_y_start = tile_y * grid.tile_size;
  size_t screen_x_end = std::min(screen_x_start + grid.tile_size, width_);
  size_t screen_y_end = std::min(screen_y_start + grid.tile_size, height_);

  size_t tile_width = screen_x_end - screen_x_start;
  size_t tile_height = screen_y_end - screen_y_start;

  // 阶段缓冲：Z 最小、胜者三角形索引、重心缓存（b0c/b1c）
  // - zmin：本 Tile 每像素的当前最小深度；
  // - winner：本 Tile 每像素的“胜出三角形”的局部索引（-1 表示尚未命中任何三角形）；
  // - b0c/b1c：缓存透视矫正后的重心权重（b2c = 1 - b0c - b1c），用于阶段B避免重复计算。
  std::vector<float> zmin(tile_width * tile_height, kDepthClear);
  std::vector<int32_t> winner(tile_width * tile_height, -1);
  std::vector<float> b0c(tile_width * tile_height, 0.0f);
  std::vector<float> b1c(tile_width * tile_height, 0.0f);

  // 初始化 tile 局部 color/depth 缓冲
  std::fill_n(tile_depth_buffer, tile_width * tile_height, kDepthClear);
  std::fill_n(tile_color_buffer, tile_width * tile_height, kColorClear);

  constexpr int kLane = 8;
  uint64_t tested_pixels = 0, covered_pixels = 0, winner_pixels = 0, shaded_pixels = 0;

  auto cross2 = [](float ax, float ay, float bx, float by) { return ax * by - ay * bx; };

  // 阶段 A：Z 决胜（仅更新 zmin / winner / b0c/b1c）
  // - 使用边函数进行半空间内点测试，行优先 + kLane 批处理，利于 cache 与自动向量化；
  // - 对覆盖像素进行透视矫正重心计算（先插 1/w，再还原权重），并据此插值 z；
  // - 若 z 更小，则更新该像素的胜者信息与缓存的重心；此阶段不执行着色。
  for (const auto& tri : triangles) {
    const size_t i0 = tri.i0, i1 = tri.i1, i2 = tri.i2;
    const Vector4f &p0 = grid.soa.pos_screen[i0];
    const Vector4f &p1 = grid.soa.pos_screen[i1];
    const Vector4f &p2 = grid.soa.pos_screen[i2];

    // 屏幕空间 AABB 与 tile 相交
    const float tri_minx = std::min({p0.x, p1.x, p2.x});
    const float tri_miny = std::min({p0.y, p1.y, p2.y});
    const float tri_maxx = std::max({p0.x, p1.x, p2.x});
    const float tri_maxy = std::max({p0.y, p1.y, p2.y});

    int sx = std::max<int>(static_cast<int>(screen_x_start), static_cast<int>(std::floor(std::max(0.0f, tri_minx))));
    int sy = std::max<int>(static_cast<int>(screen_y_start), static_cast<int>(std::floor(std::max(0.0f, tri_miny))));
    int ex = std::min<int>(static_cast<int>(screen_x_end - 1), static_cast<int>(std::floor(std::min<float>(width_ - 1, tri_maxx))));
    int ey = std::min<int>(static_cast<int>(screen_y_end - 1), static_cast<int>(std::floor(std::min<float>(height_ - 1, tri_maxy))));
    if (sx > ex || sy > ey) continue;

    // 边向量、面积及朝向
    const float e01x = p1.x - p0.x, e01y = p1.y - p0.y;
    const float e12x = p2.x - p1.x, e12y = p2.y - p1.y;
    const float e20x = p0.x - p2.x, e20y = p0.y - p2.y;
    // 面积 area2 = cross(p1 - p0, p2 - p0)；用于重心计算与正负朝向判别。
    const float area2 = cross2(e01x, e01y, p2.x - p0.x, p2.y - p0.y);
    if (std::abs(area2) < 1e-6f) continue;
    const bool positive = (area2 > 0.0f);

    // 深度与 1/w 插值准备
    // 透视校正思路：在屏幕空间中 1/w 线性，先插值 1/w，再将各顶点属性乘以 1/w 并归一。
    const float z0 = p0.z, z1 = p1.z, z2 = p2.z;
    const float w0_inv = 1.0f / p0.w, w1_inv = 1.0f / p1.w, w2_inv = 1.0f / p2.w;

    for (int y = sy; y <= ey; ++y) {
      const float yf = static_cast<float>(y);
      float E01_base = cross2(e01x, e01y, static_cast<float>(sx) - p0.x, yf - p0.y);
      float E12_base = cross2(e12x, e12y, static_cast<float>(sx) - p1.x, yf - p1.y);
      float E20_base = cross2(e20x, e20y, static_cast<float>(sx) - p2.x, yf - p2.y);
      const float dE01dx = -e01y;
      const float dE12dx = -e12y;
      const float dE20dx = -e20y;

      // 行扫描 + kLane 批处理：利于 cache 与自动向量化
      for (int xb = sx; xb <= ex; xb += kLane) {
        const int lane = std::min(kLane, ex - xb + 1);
        float E01[kLane], E12[kLane], E20[kLane];
#pragma omp simd
        for (int j = 0; j < lane; ++j) {
          E01[j] = E01_base + dE01dx * static_cast<float>(xb - sx + j);
          E12[j] = E12_base + dE12dx * static_cast<float>(xb - sx + j);
          E20[j] = E20_base + dE20dx * static_cast<float>(xb - sx + j);
        }

        // 内点测试，如果三角形在像素内，则将该像素加入覆盖掩码
        unsigned mask_cover = 0u; int cover_count = 0;
        for (int j = 0; j < lane; ++j) {
          bool inside = positive ? (E01[j] >= 0.0f && E12[j] >= 0.0f && E20[j] >= 0.0f)
                                 : (E01[j] <= 0.0f && E12[j] <= 0.0f && E20[j] <= 0.0f);
          if (inside) { mask_cover |= (1u << j); ++cover_count; }
        }
        tested_pixels += static_cast<uint64_t>(lane);
        covered_pixels += static_cast<uint64_t>(cover_count);
        if (mask_cover == 0u) continue;

        for (int j = 0; j < lane; ++j) {
          if (((mask_cover >> j) & 1u) == 0u) continue;
          const float b0 = E12[j] / area2;
          const float b1 = E20[j] / area2;
          const float b2 = E01[j] / area2;
          const float w_inv = b0 * w0_inv + b1 * w1_inv + b2 * w2_inv; // 透视校正
          const float b0c_ = (b0 * w0_inv) / w_inv;
          const float b1c_ = (b1 * w1_inv) / w_inv;
          const float b2c_ = (b2 * w2_inv) / w_inv;
          const float z = z0 * b0c_ + z1 * b1c_ + z2 * b2c_;

          const int sx_pix = xb + j;
          const int local_x = sx_pix - static_cast<int>(screen_x_start);
          const int local_y = y - static_cast<int>(screen_y_start);
          const size_t idx = static_cast<size_t>(local_x + local_y * static_cast<int>(tile_width));
          // 用极小 epsilon 防止抖动
          if (z < zmin[idx] - 1e-8f) {
            if (winner[idx] < 0) winner_pixels++;
            zmin[idx] = z;
            // 记录本 Tile 内的“局部三角形索引”，便于阶段B无需再次查找
            winner[idx] = static_cast<int32_t>(&tri - &triangles[0]);
            b0c[idx] = b0c_;
            b1c[idx] = b1c_;
          }
        }
      }
    }
  }

  // 阶段 B：仅对胜者像素着色并写入 tile 局部缓冲
  // - 对于 winner[idx] >= 0 的像素，从 SoA 插值 normal/uv/color，构造 Fragment；
  // - 每像素仅进行一次 FragmentShader 调用，随后写回 tile 局部 color/depth。
  for (size_t y = 0; y < tile_height; ++y) {
    for (size_t x = 0; x < tile_width; ++x) {
      const size_t idx = x + y * tile_width;
      int32_t win = winner[idx];
      if (win < 0) continue;

      const auto& tri = triangles[static_cast<size_t>(win)];
      const size_t i0 = tri.i0, i1 = tri.i1, i2 = tri.i2;
      const float b0c_ = b0c[idx];
      const float b1c_ = b1c[idx];
      const float b2c_ = 1.0f - b0c_ - b1c_;

      Fragment frag;
      frag.screen_coord = {static_cast<int32_t>(screen_x_start + x), static_cast<int32_t>(screen_y_start + y)};
      frag.depth = zmin[idx];
      frag.material = tri.material;

      // 插值属性
      const Vector3f &n0 = grid.soa.normal[i0];
      const Vector3f &n1 = grid.soa.normal[i1];
      const Vector3f &n2 = grid.soa.normal[i2];
      frag.normal = n0 * b0c_ + n1 * b1c_ + n2 * b2c_;

      const Vector2f &uv0 = grid.soa.uv[i0];
      const Vector2f &uv1 = grid.soa.uv[i1];
      const Vector2f &uv2 = grid.soa.uv[i2];
      frag.uv = uv0 * b0c_ + uv1 * b1c_ + uv2 * b2c_;

      const Color &c0 = grid.soa.color[i0];
      const Color &c1 = grid.soa.color[i1];
      const Color &c2 = grid.soa.color[i2];
      auto color_r = FloatToUint8_t(static_cast<float>(c0[Color::kColorIndexRed]) * b0c_ +
                                    static_cast<float>(c1[Color::kColorIndexRed]) * b1c_ +
                                    static_cast<float>(c2[Color::kColorIndexRed]) * b2c_);
      auto color_g = FloatToUint8_t(static_cast<float>(c0[Color::kColorIndexGreen]) * b0c_ +
                                    static_cast<float>(c1[Color::kColorIndexGreen]) * b1c_ +
                                    static_cast<float>(c2[Color::kColorIndexGreen]) * b2c_);
      auto color_b = FloatToUint8_t(static_cast<float>(c0[Color::kColorIndexBlue]) * b0c_ +
                                    static_cast<float>(c1[Color::kColorIndexBlue]) * b1c_ +
                                    static_cast<float>(c2[Color::kColorIndexBlue]) * b2c_);
      frag.color = Color(color_r, color_g, color_b);

      auto out_color = shader.FragmentShader(frag);
      tile_depth_buffer[idx] = frag.depth;
      tile_color_buffer[idx] = uint32_t(out_color);
      shaded_pixels++;
    }
  }

  // 写回全局缓冲（tile 行拷贝）
  // 不同 Tile 区域不重叠，行拷贝无需锁
  for (size_t y = 0; y < tile_height; ++y) {
    const size_t tile_row_off = y * tile_width;
    const size_t global_row_off = (screen_y_start + y) * width_ + screen_x_start;
    // 将局部 tile 的 color/depth 复制到全局 framebuffer 中对应位置。
    std::memcpy(global_color_buffer.get() + global_row_off,
                tile_color_buffer + tile_row_off,
                tile_width * sizeof(uint32_t));
    std::memcpy(global_depth_buffer.get() + global_row_off,
                tile_depth_buffer + tile_row_off,
                tile_width * sizeof(float));
  }

  if (out_tested) *out_tested = tested_pixels;
  if (out_covered) *out_covered = covered_pixels;
  if (out_winners) *out_winners = winner_pixels;
  if (out_shaded) *out_shaded = shaded_pixels;
}

}  // namespace simple_renderer
