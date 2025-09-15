#include "renderers/tile_based_renderer.hpp"

#include <omp.h>

#include <algorithm>
#include <chrono>
#include <cstring>
#include <limits>

#include "config.h"
#include "log_system.h"

namespace simple_renderer {

bool TileBasedRenderer::Render(const Model &model, const Shader &shader_in,
                               uint32_t *buffer) {
  auto total_start_time = std::chrono::high_resolution_clock::now();
  auto shader = std::make_shared<Shader>(shader_in);

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
#pragma omp parallel num_threads(kNProc) default(none)                        \
    shared(tile_triangles, shader, depthBuffer, colorBuffer, total_tiles,     \
               grid_ctx, early_z_)
  {
    // 为每个 tile 分配局部深度和颜色缓冲
    std::unique_ptr<float[]> tile_depth_buffer =
        std::make_unique<float[]>(grid_ctx.tile_size * grid_ctx.tile_size);
    std::unique_ptr<uint32_t[]> tile_color_buffer =
        std::make_unique<uint32_t[]>(grid_ctx.tile_size * grid_ctx.tile_size);

    // 为每个 tile 分配可复用片段临时容器，容量按单 tile 上限预估
    std::vector<Fragment> scratch_fragments;
    scratch_fragments.reserve(grid_ctx.tile_size * grid_ctx.tile_size);

#pragma omp for schedule(static)
    for (size_t tile_id = 0; tile_id < total_tiles; ++tile_id) {
      // 按照 tile 进行光栅化（SoA）
      // 直接写入单份全局 framebuffer；不同 tile 不重叠，无需加锁
      RasterizeTile(tile_id, tile_triangles[tile_id], grid_ctx,
                    tile_depth_buffer.get(), tile_color_buffer.get(),
                    depthBuffer, colorBuffer, *shader, early_z_,
                    &scratch_fragments);
    }
  }
  auto raster_end = std::chrono::high_resolution_clock::now();
  auto raster_ms = std::chrono::duration_cast<std::chrono::microseconds>(
                       raster_end - raster_start)
                       .count() /
                   1000.0;

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

  SPDLOG_INFO("=== TILE-BASED RENDERING PERFORMANCE ===");
  double sum_ms = vertex_ms + (total_ms - vertex_ms);
  SPDLOG_INFO("Vertex Shader:    {:8.3f} ms ({:5.1f}%)", vertex_ms,
              vertex_ms / sum_ms * 100);
  SPDLOG_INFO("Setup:            {:8.3f} ms", setup_ms);
  SPDLOG_INFO("Binning:          {:8.3f} ms", binning_ms);
  SPDLOG_INFO("Buffer Alloc:     {:8.3f} ms", buffer_alloc_ms);
  SPDLOG_INFO("Rasterization:    {:8.3f} ms", raster_ms);
  SPDLOG_INFO("Copy:             {:8.3f} ms", present_ms);
  SPDLOG_INFO("Total:            {:8.3f} ms",
              vertex_ms + (setup_ms + binning_ms + buffer_alloc_ms + raster_ms +
                           present_ms));
  SPDLOG_INFO("==========================================");

  return true;
}

void TileBasedRenderer::TriangleTileBinning(
    const Model& model,
    const TileGridContext& grid,
    std::vector<std::vector<TileTriangleRef>> &tile_triangles) {
  const size_t total_triangles = model.GetFaces().size();

  SPDLOG_INFO("Starting triangle-tile binning (SoA) for {} triangles",
              total_triangles);
  SPDLOG_INFO("Screen dimensions: {}x{}, Tile size: {}, Tiles: {}x{}", width_,
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
  SPDLOG_INFO("  (SoA) Total triangle references: {}", total_triangle_refs);
  SPDLOG_INFO("  (SoA) Non-empty tiles: {}", non_empty_tiles);
  SPDLOG_INFO("  (SoA) Average triangles per tile: {:.2f}",
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
    std::vector<Fragment> *scratch_fragments) {
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

  // 只有当调用方没有提供 scratch 时，才启用本地容器并且只构造一次
  const bool use_internal_scratch = (scratch_fragments == nullptr);
  std::vector<Fragment> internal_out;
  if (use_internal_scratch) internal_out.reserve(tile_width * tile_height);

  for (const auto &tri : triangles) {  // 用来应对scratch传入nullptr的情况
    // 始终走 SoA + 限制矩形的光栅化路径；如未提供 scratch，则使用函数内局部容器
    std::vector<Fragment> &out =
        use_internal_scratch ? internal_out : *scratch_fragments;
    out.clear();
    if (out.capacity() < tile_width * tile_height)
      out.reserve(tile_width * tile_height);

    rasterizer_->RasterizeTo(
        grid.soa, tri.i0, tri.i1, tri.i2, static_cast<int>(screen_x_start),
        static_cast<int>(screen_y_start), static_cast<int>(screen_x_end),
        static_cast<int>(screen_y_end), out);

    for (auto &fragment : out) {
      fragment.material = tri.material;
      size_t sx = fragment.screen_coord[0];
      size_t sy = fragment.screen_coord[1];
      if (sx >= screen_x_start && sx < screen_x_end && sy >= screen_y_start &&
          sy < screen_y_end) {
        size_t local_x = sx - screen_x_start;
        size_t local_y = sy - screen_y_start;
        size_t idx = local_x + local_y * tile_width;
        if (use_early_z) {
          if (fragment.depth < tile_depth_buffer[idx]) {
            auto color = shader.FragmentShader(fragment);
            tile_depth_buffer[idx] = fragment.depth;
            tile_color_buffer[idx] = uint32_t(color);
          }
        } else {
          auto color = shader.FragmentShader(fragment);
          if (fragment.depth < tile_depth_buffer[idx]) {
            tile_depth_buffer[idx] = fragment.depth;
            tile_color_buffer[idx] = uint32_t(color);
          }
        }
      }
    }
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
