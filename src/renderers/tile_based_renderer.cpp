#include "renderers/tile_based_renderer.hpp"

#include <omp.h>

#include <algorithm>
#include <chrono>
#include <cstring>
#include <limits>
#include <cmath>

#include "config.h"
#include "log_system.h"

namespace simple_renderer {

bool TileBasedRenderer::Render(const Model &model, const Shader &shader_in,
                               uint32_t *buffer) {
  auto total_start_time = std::chrono::high_resolution_clock::now();
  auto shader = std::make_shared<Shader>(shader_in);
  shader->PrepareUniformCaches();

  // 顶点变换（SoA）
  auto vertex_start = std::chrono::high_resolution_clock::now();
  const auto &input_vertices = model.GetVertices();
  VertexSoA soa;
  soa.resize(input_vertices.size());

#pragma omp parallel for num_threads(kNProc) schedule(static) \
    shared(shader, soa, input_vertices)
  for (size_t i = 0; i < input_vertices.size(); ++i) {
    const auto &v = input_vertices[i];
    auto clipSpaceVertex = shader->VertexShader(v);
    soa.pos_clip[i] = clipSpaceVertex.GetPosition();
    auto ndcVertex = PerspectiveDivision(clipSpaceVertex);
    auto screenSpaceVertex = ViewportTransformation(ndcVertex);
    soa.pos_screen[i] = screenSpaceVertex.GetPosition();
    soa.normal[i] = screenSpaceVertex.GetNormal();
    soa.uv[i] = screenSpaceVertex.GetTexCoords();
    soa.color[i] = screenSpaceVertex.GetColor();
  }
  auto vertex_end = std::chrono::high_resolution_clock::now();
  auto vertex_ms = std::chrono::duration_cast<std::chrono::microseconds>(
                       vertex_end - vertex_start)
                       .count() /
                   1000.0;

  // 1. Setup
  auto setup_start = std::chrono::high_resolution_clock::now();
  const size_t TILE_SIZE = tile_size_ > 0 ? tile_size_ : 64;
  const size_t tiles_x = (width_ + TILE_SIZE - 1) / TILE_SIZE;
  const size_t tiles_y = (height_ + TILE_SIZE - 1) / TILE_SIZE;
  const size_t total_tiles = tiles_x * tiles_y;

  // 为每个tile创建三角形列表（SoA 引用）
  std::vector<std::vector<TileTriangleRef>> tile_triangles(total_tiles);
  auto setup_end = std::chrono::high_resolution_clock::now();
  auto setup_ms = std::chrono::duration_cast<std::chrono::microseconds>(
                      setup_end - setup_start)
                      .count() /
                  1000.0;

  // 2. Binning
  auto binning_start = std::chrono::high_resolution_clock::now();
  TileGridContext grid_ctx{soa, tiles_x, tiles_y, TILE_SIZE};
  TriangleTileBinning(model, grid_ctx, tile_triangles);
  auto binning_end = std::chrono::high_resolution_clock::now();
  auto binning_ms = std::chrono::duration_cast<std::chrono::microseconds>(
                        binning_end - binning_start)
                        .count() /
                    1000.0;

  // 3. 单份全局 framebuffer
  // 直接让每个 tile 写入这份全局缓冲区，避免末端 O(W*H*kNProc) 合并开销

  auto buffer_alloc_start = std::chrono::high_resolution_clock::now();
  std::unique_ptr<float[]> depthBuffer =
      std::make_unique<float[]>(width_ * height_);
  std::unique_ptr<uint32_t[]> colorBuffer =
      std::make_unique<uint32_t[]>(width_ * height_);

  // 深度初始化为最远值，颜色清零
  std::fill_n(depthBuffer.get(), width_ * height_, kDepthClear);
  std::fill_n(colorBuffer.get(), width_ * height_, kColorClear);
  auto buffer_alloc_end = std::chrono::high_resolution_clock::now();
  auto buffer_alloc_ms = std::chrono::duration_cast<std::chrono::microseconds>(
                             buffer_alloc_end - buffer_alloc_start)
                             .count() /
                         1000.0;

  // 4. 并行光栅化每个 tile（SoA + early-z）
  auto raster_start = std::chrono::high_resolution_clock::now();
  std::vector<TileMaskStats> tile_stats(total_tiles);
#pragma omp parallel num_threads(kNProc) default(none)                        \
    shared(tile_triangles, shader, depthBuffer, colorBuffer, total_tiles,     \
               grid_ctx, early_z_, tile_stats)
  {
    // 为每个 tile 分配局部深度和颜色缓冲
    std::unique_ptr<float[]> tile_depth_buffer =
        std::make_unique<float[]>(grid_ctx.tile_size * grid_ctx.tile_size);
    std::unique_ptr<uint32_t[]> tile_color_buffer =
        std::make_unique<uint32_t[]>(grid_ctx.tile_size * grid_ctx.tile_size);

#pragma omp for schedule(static)
    for (size_t tile_id = 0; tile_id < total_tiles; ++tile_id) {
      // 按照 tile 进行光栅化（SoA）
      // 直接写入单份全局 framebuffer；不同 tile 不重叠，无需加锁
      RasterizeTile(tile_id, tile_triangles[tile_id], grid_ctx,
                    tile_depth_buffer.get(), tile_color_buffer.get(),
                    depthBuffer, colorBuffer, *shader, early_z_,
                    &tile_stats[tile_id]);
    }
  }
  auto raster_end = std::chrono::high_resolution_clock::now();
  auto raster_ms = std::chrono::duration_cast<std::chrono::microseconds>(
                       raster_end - raster_start)
                       .count() /
                   1000.0;

  // 汇总并打印掩码收益统计
  uint64_t sum_tested = 0, sum_covered = 0, sum_zpass = 0, sum_shaded = 0;
  for (const auto& s : tile_stats) {
    sum_tested += s.tested;
    sum_covered += s.covered;
    sum_zpass   += s.zpass;
    sum_shaded  += s.shaded;
  }
  auto rate = [](uint64_t num, uint64_t den) -> double {
    return (den == 0)?0.0:double(num) / double(den) * 100.0;
  };
  SPDLOG_DEBUG(
      "TBR Mask Stats: tested={}, covered={} ({:.1f}%), zpass={} ({:.1f}%), shaded={} ({:.1f}%)",
      sum_tested, sum_covered, rate(sum_covered, sum_tested),
      sum_zpass, rate(sum_zpass, sum_covered),
      sum_shaded, rate(sum_shaded, sum_covered));

  // 5. 直接将单份全局 colorBuffer 拷贝到输出
  auto present_start = std::chrono::high_resolution_clock::now();
  std::memcpy(buffer, colorBuffer.get(), width_ * height_ * sizeof(uint32_t));
  auto present_end = std::chrono::high_resolution_clock::now();
  auto present_ms = std::chrono::duration_cast<std::chrono::microseconds>(
                        present_end - present_start)
                        .count() /
                    1000.0;

  auto total_end_time = std::chrono::high_resolution_clock::now();
  double total_ms = std::chrono::duration_cast<std::chrono::microseconds>(
                        total_end_time - total_start_time)
                        .count() /
                    1000.0;

  SPDLOG_DEBUG("=== TILE-BASED RENDERING PERFORMANCE ===");
  double sum_ms = vertex_ms + (total_ms - vertex_ms);
  SPDLOG_DEBUG("Vertex Shader:    {:8.3f} ms ({:5.1f}%)", vertex_ms,
              vertex_ms / sum_ms * 100);
  SPDLOG_DEBUG("Setup:            {:8.3f} ms", setup_ms);
  SPDLOG_DEBUG("Binning:          {:8.3f} ms", binning_ms);
  SPDLOG_DEBUG("Buffer Alloc:     {:8.3f} ms", buffer_alloc_ms);
  SPDLOG_DEBUG("Rasterization:    {:8.3f} ms", raster_ms);
  SPDLOG_DEBUG("Copy:             {:8.3f} ms", present_ms);
  SPDLOG_DEBUG("Total:            {:8.3f} ms",
              vertex_ms + (setup_ms + binning_ms + buffer_alloc_ms + raster_ms +
                          present_ms));
  SPDLOG_DEBUG("==========================================");

  return true;
}

void TileBasedRenderer::TriangleTileBinning(
    const Model& model,
    const TileGridContext& grid,
    std::vector<std::vector<TileTriangleRef>> &tile_triangles) {
  const size_t total_triangles = model.GetFaces().size();

  SPDLOG_DEBUG("Starting triangle-tile binning (SoA) for {} triangles",
              total_triangles);
  SPDLOG_DEBUG("Screen dimensions: {}x{}, Tile size: {}, Tiles: {}x{}", width_,
              height_, grid.tile_size, grid.tiles_x, grid.tiles_y);

  std::vector<size_t> tile_counts(grid.tiles_x * grid.tiles_y, 0);

  // 第一遍（count only）：计算每个tile需要容纳多少三角形
  for (size_t tri_idx = 0; tri_idx < total_triangles; ++tri_idx) {
    ProcessTriangleForTileBinning(tri_idx, true, model, grid,
                                  tile_counts, tile_triangles);
  }

  // 预分配，避免动态扩容
  for (size_t tile_id = 0; tile_id < tile_triangles.size(); ++tile_id) {
    if (tile_counts[tile_id] > 0)
      tile_triangles[tile_id].reserve(tile_counts[tile_id]);
  }

  // 第二遍（fill）：按范围填充TriangleRef
  for (size_t tri_idx = 0; tri_idx < total_triangles; ++tri_idx) {
    ProcessTriangleForTileBinning(tri_idx, false, model, grid,
                                  tile_counts, tile_triangles);
  }

  size_t total_triangle_refs = 0;
  size_t non_empty_tiles = 0;
  for (const auto &tile : tile_triangles) {
    total_triangle_refs += tile.size();
    if (!tile.empty()) non_empty_tiles++;
  }
  SPDLOG_DEBUG("  (SoA) Total triangle references: {}", total_triangle_refs);
  SPDLOG_DEBUG("  (SoA) Non-empty tiles: {}", non_empty_tiles);
  SPDLOG_DEBUG("  (SoA) Average triangles per tile: {:.2f}",
              total_triangle_refs > 0
                  ? float(total_triangle_refs) / tile_triangles.size()
                  : 0.0f);
}

void TileBasedRenderer::RasterizeTile(
    size_t tile_id, const std::vector<TileTriangleRef> &triangles,
    const TileGridContext& grid, float *tile_depth_buffer,
    uint32_t *tile_color_buffer, std::unique_ptr<float[]> &global_depth_buffer,
    std::unique_ptr<uint32_t[]> &global_color_buffer,
    const Shader &shader, bool use_early_z,
    TileMaskStats* out_stats) {
  // 计算 tile 屏幕范围
  size_t tile_x = tile_id % grid.tiles_x;
  size_t tile_y = tile_id / grid.tiles_x;
  size_t screen_x_start = tile_x * grid.tile_size;
  size_t screen_y_start = tile_y * grid.tile_size;
  size_t screen_x_end = std::min(screen_x_start + grid.tile_size, width_);
  size_t screen_y_end = std::min(screen_y_start + grid.tile_size, height_);

  // 初始化 tile 局部缓冲
  size_t tile_width = screen_x_end - screen_x_start;
  size_t tile_height = screen_y_end - screen_y_start;
  std::fill_n(tile_depth_buffer, tile_width * tile_height, kDepthClear);
  std::fill_n(tile_color_buffer, tile_width * tile_height, kColorClear);

  // 掩码化扫描：按三角形直接写入 tile 局部缓冲，避免中间片段向量
  constexpr int kLane = 8;  // 横向处理的像素个数（便于编译器自动向量化）

  // 轻量统计：用于评估掩码收益（仅对少量tile打印DEBUG）
  uint64_t tested_pixels = 0;
  uint64_t covered_pixels = 0;
  uint64_t zpass_pixels = 0;
  uint64_t shaded_pixels = 0;

  auto cross2 = [](float ax, float ay, float bx, float by) {
    return ax * by - ay * bx;
  };

  for (const auto &tri : triangles) {
    const auto i0 = tri.i0, i1 = tri.i1, i2 = tri.i2;

    // 顶点屏幕坐标
    const Vector4f &p0 = grid.soa.pos_screen[i0];
    const Vector4f &p1 = grid.soa.pos_screen[i1];
    const Vector4f &p2 = grid.soa.pos_screen[i2];

    // 三角形屏幕空间 AABB，与 tile 矩形求交
    const float tri_minx = std::min({p0.x, p1.x, p2.x});
    const float tri_miny = std::min({p0.y, p1.y, p2.y});
    const float tri_maxx = std::max({p0.x, p1.x, p2.x});
    const float tri_maxy = std::max({p0.y, p1.y, p2.y});

    int sx = std::max<int>(static_cast<int>(screen_x_start),
                           static_cast<int>(std::floor(std::max(0.0f, tri_minx))));
    int sy = std::max<int>(static_cast<int>(screen_y_start),
                           static_cast<int>(std::floor(std::max(0.0f, tri_miny))));
    int ex = std::min<int>(static_cast<int>(screen_x_end - 1),
                           static_cast<int>(std::floor(std::min<float>(width_ - 1, tri_maxx))));
    int ey = std::min<int>(static_cast<int>(screen_y_end - 1),
                           static_cast<int>(std::floor(std::min<float>(height_ - 1, tri_maxy))));
    if (sx > ex || sy > ey) continue;

    // 边向量与有向面积
    const float e01x = p1.x - p0.x, e01y = p1.y - p0.y;
    const float e12x = p2.x - p1.x, e12y = p2.y - p1.y;
    const float e20x = p0.x - p2.x, e20y = p0.y - p2.y;
    const float area2 = cross2(e01x, e01y, p2.x - p0.x, p2.y - p0.y);
    if (std::abs(area2) < 1e-6f) continue;  // 退化三角形
    const bool positive = (area2 > 0.0f);

    // z 与 1/w 的平面插值准备
    const float z0 = p0.z, z1 = p1.z, z2 = p2.z;
    const float w0_inv = 1.0f / p0.w, w1_inv = 1.0f / p1.w, w2_inv = 1.0f / p2.w;

    // 行扫描
    for (int y = sy; y <= ey; ++y) { // 行优先遍历：有利于 cache 与向量化
      const float yf = static_cast<float>(y);
      for (int xb = sx; xb <= ex; xb += kLane) { // 每次处理kLane个像素
        const int lane = std::min(kLane, ex - xb + 1); // 当前需要处理的像素个数
        const float x0f = static_cast<float>(xb); // 本块起点的x坐标

        // 计算本块起点的三个边函数值与横向步长（dE/dx）
        float E01_base = cross2(e01x, e01y, x0f - p0.x, yf - p0.y);
        float E12_base = cross2(e12x, e12y, x0f - p1.x, yf - p1.y);
        float E20_base = cross2(e20x, e20y, x0f - p2.x, yf - p2.y);
        const float dE01dx = -e01y;
        const float dE12dx = -e12y;
        const float dE20dx = -e20y;

        // ============== 构造覆盖掩码 cover mask ==============
        unsigned mask_cover = 0u;
        int cover_count = 0;
        float E01[kLane], E12[kLane], E20[kLane];
        #pragma omp simd
        for (int j = 0; j < lane; ++j) {
          E01[j] = E01_base + dE01dx * static_cast<float>(j);
          E12[j] = E12_base + dE12dx * static_cast<float>(j);
          E20[j] = E20_base + dE20dx * static_cast<float>(j);
        }
        for (int j = 0; j < lane; ++j) { // 内点测试，如果三角形在像素内，则将该像素加入覆盖掩码
          bool inside = positive ? (E01[j] >= 0.0f && E12[j] >= 0.0f && E20[j] >= 0.0f)
                                 : (E01[j] <= 0.0f && E12[j] <= 0.0f && E20[j] <= 0.0f);
          if (inside) {
            mask_cover |= (1u << j);
            cover_count++;
          }
        }
        tested_pixels += static_cast<uint64_t>(lane);
        covered_pixels += static_cast<uint64_t>(cover_count);
        if (mask_cover == 0u) continue;

        // ============== 计算 z，进行early-z掩码 ==============
        unsigned mask_zpass = 0u;
        float zvals[kLane];
        // 缓存校正后的重心坐标，避免着色阶段重复计算
        float b0c_arr[kLane];
        float b1c_arr[kLane];
        float b2c_arr[kLane];
        int zpass_count = 0;
        for (int j = 0; j < lane; ++j) {
          if (((mask_cover >> j) & 1u) == 0u) { continue; } // 如果该像素不在覆盖掩码内，则跳过
          const float b0 = E12[j] / area2;
          const float b1 = E20[j] / area2;
          const float b2 = E01[j] / area2;
          const float w_inv = b0 * w0_inv + b1 * w1_inv + b2 * w2_inv; // 透视矫正
          const float b0c = (b0 * w0_inv) / w_inv;
          const float b1c = (b1 * w1_inv) / w_inv;
          const float b2c = (b2 * w2_inv) / w_inv;
          b0c_arr[j] = b0c; b1c_arr[j] = b1c; b2c_arr[j] = b2c;
          const float z = z0 * b0c + z1 * b1c + z2 * b2c;
          zvals[j] = z;

          const int sx_pix = xb + j;
          const int local_x = sx_pix - static_cast<int>(screen_x_start);
          const int local_y = y - static_cast<int>(screen_y_start);
          const size_t idx = static_cast<size_t>(local_x + local_y * static_cast<int>(tile_width));
          if (z < tile_depth_buffer[idx]) {
            mask_zpass |= (1u << j);
            zpass_count++;
          }
        }
        zpass_pixels += static_cast<uint64_t>(zpass_count);

        // ============== 构造最终掩码 ==============
        unsigned mask_final = use_early_z ? (mask_cover & mask_zpass) : mask_cover;
        if (mask_final == 0u && use_early_z) continue;

        // 对掩码内像素着色并写回（非 early-z 时，先着色，再按 z 测试写入）
        for (int j = 0; j < lane; ++j) {
          if (((mask_final >> j) & 1u) == 0u && use_early_z) continue;
          const int sx_pix = xb + j;
          const int local_x = sx_pix - static_cast<int>(screen_x_start);
          const int local_y = y - static_cast<int>(screen_y_start);
          const size_t idx = static_cast<size_t>(local_x + local_y * static_cast<int>(tile_width));

          // 计算插值属性
          const float b0c = b0c_arr[j];
          const float b1c = b1c_arr[j];
          const float b2c = b2c_arr[j];

          Fragment frag;
          frag.screen_coord = {sx_pix, y};
          frag.depth = zvals[j];
          frag.material = tri.material;

          // 法向量插值
          const Vector3f &n0 = grid.soa.normal[i0];
          const Vector3f &n1 = grid.soa.normal[i1];
          const Vector3f &n2 = grid.soa.normal[i2];
          frag.normal = n0 * b0c + n1 * b1c + n2 * b2c;

          // 纹理坐标插值
          const Vector2f &uv0 = grid.soa.uv[i0];
          const Vector2f &uv1 = grid.soa.uv[i1];
          const Vector2f &uv2 = grid.soa.uv[i2];
          frag.uv = uv0 * b0c + uv1 * b1c + uv2 * b2c;

          // 颜色插值
          const Color &c0 = grid.soa.color[i0];
          const Color &c1 = grid.soa.color[i1];
          const Color &c2 = grid.soa.color[i2];
          auto color_r = FloatToUint8_t(static_cast<float>(c0[Color::kColorIndexRed]) * b0c +
                                        static_cast<float>(c1[Color::kColorIndexRed]) * b1c +
                                        static_cast<float>(c2[Color::kColorIndexRed]) * b2c);
          auto color_g = FloatToUint8_t(static_cast<float>(c0[Color::kColorIndexGreen]) * b0c +
                                        static_cast<float>(c1[Color::kColorIndexGreen]) * b1c +
                                        static_cast<float>(c2[Color::kColorIndexGreen]) * b2c);
          auto color_b = FloatToUint8_t(static_cast<float>(c0[Color::kColorIndexBlue]) * b0c +
                                        static_cast<float>(c1[Color::kColorIndexBlue]) * b1c +
                                        static_cast<float>(c2[Color::kColorIndexBlue]) * b2c);
          frag.color = Color(color_r, color_g, color_b);

          if (use_early_z) { // 开启时，仅对mask中通过early-z的像素进行着色和写回
            auto out_color = shader.FragmentShader(frag);
            tile_depth_buffer[idx] = frag.depth;
            tile_color_buffer[idx] = uint32_t(out_color);
            shaded_pixels++;
          } else {
            // 关闭时，先着色，再按z测试写入
            auto out_color = shader.FragmentShader(frag);
            if (frag.depth < tile_depth_buffer[idx]) { // late-z
              tile_depth_buffer[idx] = frag.depth;
              tile_color_buffer[idx] = uint32_t(out_color);
              shaded_pixels++;
            }
          }
        }
      }
    }
  }

  if (out_stats) {
    out_stats->tested = tested_pixels;
    out_stats->covered = covered_pixels;
    out_stats->zpass = zpass_pixels;
    out_stats->shaded = shaded_pixels;
  }

  // 写回全局缓冲
  // TBR 下不同 tile 覆盖的屏幕区域互不重叠，且在 tile 内部已通过 Early‑Z
  // 得出每个像素的最终值。因此可以直接将 tile 行数据拷贝到全局缓冲
  for (size_t y = 0; y < tile_height; y++) {
    const size_t tile_row_off = y * tile_width;
    const size_t global_row_off =
        (screen_y_start + y) * width_ + screen_x_start;

    // 拷贝本行 color 到全局 color
    std::memcpy(global_color_buffer.get() + global_row_off,
                tile_color_buffer + tile_row_off,
                tile_width * sizeof(uint32_t));

    // 拷贝本行 depth 到全局 depth
    std::memcpy(global_depth_buffer.get() + global_row_off,
                tile_depth_buffer + tile_row_off, tile_width * sizeof(float));
  }
}

void TileBasedRenderer::ProcessTriangleForTileBinning(
    size_t tri_idx, bool count_only, const Model &model,
    const TileGridContext &grid, std::vector<size_t> &tile_counts,
    std::vector<std::vector<TileTriangleRef>> &tile_triangles) {
  const auto &f = model.GetFaces()[tri_idx];
  size_t i0 = f.GetIndex(0);
  size_t i1 = f.GetIndex(1);
  size_t i2 = f.GetIndex(2);

  // 视锥体裁剪 (裁剪空间)
  // 保守视锥体裁剪：只有当整个三角形都在视锥体外同一侧时才裁剪
  const Vector4f &c0 = grid.soa.pos_clip[i0];
  const Vector4f &c1 = grid.soa.pos_clip[i1];
  const Vector4f &c2 = grid.soa.pos_clip[i2];
  bool frustum_cull =
      (c0.x > c0.w && c1.x > c1.w && c2.x > c2.w) ||     // 右平面外
      (c0.x < -c0.w && c1.x < -c0.w && c2.x < -c0.w) ||  // 左平面外
      (c0.y > c0.w && c1.y > c1.w && c2.y > c2.w) ||     // 上平面外
      (c0.y < -c0.w && c1.y < -c0.w && c2.y < -c0.w) ||  // 下平面外
      (c0.z > c0.w && c1.z > c1.w && c2.z > c2.w) ||     // 远平面外
      (c0.z < -c0.w && c1.z < -c0.w && c2.z < -c0.w);    // 近平面外
  if (frustum_cull) {
    return;
  }

  const Vector4f &pos0 = grid.soa.pos_screen[i0];
  const Vector4f &pos1 = grid.soa.pos_screen[i1];
  const Vector4f &pos2 = grid.soa.pos_screen[i2];

  // 背面剔除（屏幕空间）
  // NDC空间中叉积为负表示顺时针，即背面。
  // 从NDC到屏幕空间中，会发生Y轴翻转，对应叉积应为正。
  Vector2f screen0(pos0.x, pos0.y);
  Vector2f screen1(pos1.x, pos1.y);
  Vector2f screen2(pos2.x, pos2.y);
  Vector2f edge1 = screen1 - screen0;
  Vector2f edge2 = screen2 - screen0;
  float cross_product = edge1.x * edge2.y - edge1.y * edge2.x;
  if (cross_product > 0.0f) return;

  float screen_x0 = pos0.x;
  float screen_y0 = pos0.y;
  float screen_x1 = pos1.x;
  float screen_y1 = pos1.y;
  float screen_x2 = pos2.x;
  float screen_y2 = pos2.y;

  // 计算屏幕bbox，用于后续tile划分
  float min_x = std::min({screen_x0, screen_x1, screen_x2});
  float max_x = std::max({screen_x0, screen_x1, screen_x2});
  float min_y = std::min({screen_y0, screen_y1, screen_y2});
  float max_y = std::max({screen_y0, screen_y1, screen_y2});

  int start_tile_x = std::max(0, static_cast<int>(min_x) /
                                     static_cast<int>(grid.tile_size));
  int end_tile_x =
      std::min(static_cast<int>(grid.tiles_x - 1),
               static_cast<int>(max_x) / static_cast<int>(grid.tile_size));
  int start_tile_y = std::max(0, static_cast<int>(min_y) /
                                     static_cast<int>(grid.tile_size));
  int end_tile_y =
      std::min(static_cast<int>(grid.tiles_y - 1),
               static_cast<int>(max_y) / static_cast<int>(grid.tile_size));
  if (start_tile_x > end_tile_x || start_tile_y > end_tile_y)
    return;  // 如果bbox不在任何tile内，直接返回

  if (count_only) {  // 第一遍计数，只统计tile内三角形数量
    for (int ty = start_tile_y; ty <= end_tile_y; ++ty) {
      for (int tx = start_tile_x; tx <= end_tile_x; ++tx) {
        size_t tile_id = ty * grid.tiles_x + tx;
        tile_counts[tile_id]++;
      }
    }
  } else {  // 第二遍填充，填充TriangleRef
    TileTriangleRef tri_ref{i0, i1, i2, &f.GetMaterial(), tri_idx};
    for (int ty = start_tile_y; ty <= end_tile_y; ++ty) {
      for (int tx = start_tile_x; tx <= end_tile_x; ++tx) {
        size_t tile_id = ty * grid.tiles_x + tx;
        tile_triangles[tile_id].push_back(tri_ref);
      }
    }
  }
}

}  // namespace simple_renderer
