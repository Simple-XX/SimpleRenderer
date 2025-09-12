
/**
 * @file simple_renderer.cpp
 * @brief SimpleRenderer 实现
 * @author Zone.N (Zone.Niuzh@hotmail.com)
 * @version 1.0
 * @date 2023-10-23
 * @copyright MIT LICENSE
 * https://github.com/Simple-XX/SimpleRenderer
 * @par change log:
 * <table>
 * <tr><th>Date<th>Author<th>Description
 * <tr><td>2023-10-23<td>Zone.N<td>创建文件
 * </table>
 */

#include "renderer.h"

#include <omp.h>

#include <array>
#include <chrono>
#include <cstdint>
#include <limits>
#include <span>
#include <string_view>
#include <vector>

#include "config.h"
#include "light.h"
#include "log_system.h"
#include "model.hpp"

namespace simple_renderer {

// RenderingMode到字符串转换函数
std::string RenderingModeToString(RenderingMode mode) {
  switch(mode) {
    case RenderingMode::TRADITIONAL:
      return "TRADITIONAL";
    case RenderingMode::TILE_BASED:
      return "TILE_BASED";
    case RenderingMode::DEFERRED:
      return "DEFERRED";
  }
}
SimpleRenderer::SimpleRenderer(size_t width, size_t height)
    : height_(height),
      width_(width),
      log_system_(LogSystem(kLogFilePath, kLogFileMaxSize, kLogFileMaxCount)),
      current_mode_(RenderingMode::TILE_BASED),
      is_early_z_enabled_(true) {
  rasterizer_ = std::make_shared<Rasterizer>(width, height);
}

bool SimpleRenderer::DrawModel(const Model &model, const Shader &shader,
                               uint32_t *buffer) {
  SPDLOG_INFO("draw model: {}", model.GetModelPath());
  shader_ = std::make_shared<Shader>(shader);
  ExecuteDrawPipeline(model, buffer);
  return true;
}

void SimpleRenderer::SetRenderingMode(RenderingMode mode) {
  current_mode_ = mode;
  SPDLOG_INFO("rendering mode set to: {}", RenderingModeToString(mode));
}

RenderingMode SimpleRenderer::GetRenderingMode() const {
  return current_mode_;
}

/*
Optimizes performance by performing depth testing during rasterization, keeping
only the closest fragment per pixel, and avoiding storing all
fragments—resulting in faster rendering.

通过在光栅化过程中执行深度测试，仅保留每个像素的深度值最近的片段，避免存储所有片段，从而优化性能，实现更快的渲染。
*/
void SimpleRenderer::ExecuteDrawPipeline(const Model &model, uint32_t *buffer) {
  SPDLOG_INFO("execute draw pipeline for {} using {} mode", 
              model.GetModelPath(), RenderingModeToString(current_mode_));
  
  if (!shader_) {
    SPDLOG_ERROR("No shader set for DrawModel, cannot render");
    return;
  }
  
  /* * * Vertex Transformation * * */
  auto vertex_shader_start_time = std::chrono::high_resolution_clock::now();
  const auto &input_vertices = model.GetVertices();
  std::vector<Vertex> processedVertices;  // 非 TBR
  VertexSoA processedSoA;                 // TBR 专用

  if (current_mode_ == RenderingMode::TILE_BASED) {
    processedSoA.resize(input_vertices.size());
    // schedule(static)使并行过程保持连续分块，避免 false sharing
#pragma omp parallel for num_threads(kNProc) schedule(static) \
    shared(shader_, processedSoA, input_vertices)
    for (size_t i = 0; i < input_vertices.size(); ++i) { // 按索引并行处理
      const auto &v = input_vertices[i];
      // 顶点着色器：世界坐标 -> 裁剪坐标
      auto clipSpaceVertex = shader_->VertexShader(v);
      // 保存裁剪空间坐标用于后续视锥体裁剪
      processedSoA.pos_clip[i] = clipSpaceVertex.GetPosition();
      auto ndcVertex = PerspectiveDivision(clipSpaceVertex);
      auto screenSpaceVertex = ViewportTransformation(ndcVertex);

      // 填充为SoA数据结构，用于优化缓存局部性
      processedSoA.pos_screen[i] = screenSpaceVertex.GetPosition();
      processedSoA.normal[i]     = screenSpaceVertex.GetNormal();
      processedSoA.uv[i]         = screenSpaceVertex.GetTexCoords();
      processedSoA.color[i]      = screenSpaceVertex.GetColor();
    }
  } else { // Tradition或Deffer管线
    processedVertices.resize(input_vertices.size()); // 根据顶点总数量进行预分配
    // 并行过程保持连续分块，避免false sharing
#pragma omp parallel for num_threads(kNProc) schedule(static) \
    shared(shader_, processedVertices, input_vertices)
    for (size_t i = 0; i < input_vertices.size(); ++i) { // 按索引并行处理
      const auto &v = input_vertices[i];
      auto clipSpaceVertex = shader_->VertexShader(v);
      auto ndcVertex = PerspectiveDivision(clipSpaceVertex);
      auto screenSpaceVertex = ViewportTransformation(ndcVertex);
      processedVertices[i] = screenSpaceVertex;
    }
  }
  auto vertex_shader_end_time = std::chrono::high_resolution_clock::now();
  auto vertex_shader_duration = std::chrono::duration_cast<std::chrono::microseconds>(
      vertex_shader_end_time - vertex_shader_start_time);

  // 根据当前设置的模式选择不同的渲染管线
  double vertex_ms = vertex_shader_duration.count() / 1000.0;
  
  switch (current_mode_) {
    case RenderingMode::TRADITIONAL: {
      auto stats = ExecuteTraditionalPipeline(model, processedVertices, buffer);
      PrintTraditionalStats(vertex_ms, stats);
      break;
    }
    
    case RenderingMode::TILE_BASED: {
      auto stats = ExecuteTileBasedPipeline(model, processedSoA, buffer);
      PrintTileBasedStats(vertex_ms, stats);
      break;
    }
    
    case RenderingMode::DEFERRED: {
      auto stats = ExecuteDeferredPipeline(model, processedVertices, buffer);
      PrintDeferredStats(vertex_ms, stats);
      break;
    }
  }
}


/*
Organizes processing to simulate how OpenGL works with GPUs by collecting all
fragments per pixel before processing, closely mimicking the GPU pipeline but
leading to increased memory usage and slower performance.

组织处理方式模拟 OpenGL 在 GPU
上的工作原理，先收集每个像素的所有片段再并行处理屏幕上的每个像素，模仿 GPU
管线，但导致内存使用增加和渲染速度变慢。

现在作为延迟渲染管线的一部分，用于教学演示经典GPU管线概念。
*/
SimpleRenderer::DeferredRenderStats SimpleRenderer::ExecuteDeferredPipeline(
    const Model &model,
    const std::vector<Vertex> &processedVertices,
    uint32_t *buffer) {
    
  DeferredRenderStats stats;
  auto total_start_time = std::chrono::high_resolution_clock::now();
  SPDLOG_INFO("execute deferred pipeline for {}", model.GetModelPath());
  /*  *  *  *  *  *  *  */

  /* * * Buffer Allocation * * */
  auto buffer_alloc_start_time = std::chrono::high_resolution_clock::now();
  std::vector<std::vector<std::vector<Fragment>>> fragmentsBuffer_all_thread(
      kNProc, std::vector<std::vector<Fragment>>(width_ * height_));

  // 预先缓存所有Material数据，避免指针悬垂问题
  std::vector<Material> material_cache;
  material_cache.reserve(model.GetFaces().size());
  for (const auto &f : model.GetFaces()) {
    material_cache.emplace_back(f.GetMaterial()); // 值拷贝
  }
  auto buffer_alloc_end_time = std::chrono::high_resolution_clock::now();
  auto buffer_alloc_duration = std::chrono::duration_cast<std::chrono::microseconds>(
      buffer_alloc_end_time - buffer_alloc_start_time);
  SPDLOG_INFO("cached {} materials for deferred rendering", material_cache.size());

  /* * * Rasterization * * */
  auto rasterization_start_time = std::chrono::high_resolution_clock::now();
#pragma omp parallel num_threads(kNProc) default(none)                       \
  shared(processedVertices, fragmentsBuffer_all_thread, rasterizer_, width_, \
               height_, material_cache, model)
  {
    int thread_id = omp_get_thread_num();
    auto &fragmentsBuffer_per_thread = fragmentsBuffer_all_thread[thread_id];

#pragma omp for
    for (size_t face_idx = 0; face_idx < model.GetFaces().size(); ++face_idx) {
      const auto &f = model.GetFaces()[face_idx];
      auto v0 = processedVertices[f.GetIndex(0)];
      auto v1 = processedVertices[f.GetIndex(1)];
      auto v2 = processedVertices[f.GetIndex(2)];

      const Material *material = &material_cache[face_idx]; // 使用缓存的Material

      auto fragments = rasterizer_->Rasterize(v0, v1, v2);

      for (auto &fragment : fragments) {
        fragment.material = material;

        size_t x = fragment.screen_coord[0];
        size_t y = fragment.screen_coord[1];

        if (x >= width_ || y >= height_) {
          continue;
        }

        size_t index = x + y * width_;
        fragmentsBuffer_per_thread[index].push_back(fragment);
      }
    }
  }
  auto rasterization_end_time = std::chrono::high_resolution_clock::now();
  auto rasterization_duration = std::chrono::duration_cast<std::chrono::microseconds>(
      rasterization_end_time - rasterization_start_time);
  /*  *  *  *  *  *  *  */

  /* * * Fragment Collection * * */
  auto fragment_collection_start_time = std::chrono::high_resolution_clock::now();
  std::vector<std::vector<Fragment>> fragmentsBuffer(width_ * height_);
  for (const auto &fragmentsBuffer_per_thread : fragmentsBuffer_all_thread) {
    for (size_t i = 0; i < fragmentsBuffer_per_thread.size(); i++) {
      fragmentsBuffer[i].insert(fragmentsBuffer[i].end(),
                                fragmentsBuffer_per_thread[i].begin(),
                                fragmentsBuffer_per_thread[i].end());
    }
  }
  auto fragment_collection_end_time = std::chrono::high_resolution_clock::now();
  auto fragment_collection_duration = std::chrono::duration_cast<std::chrono::microseconds>(
      fragment_collection_end_time - fragment_collection_start_time);
  /*  *  *  *  *  *  *  */

  /* * * Fragment Merge & Deferred Shading * * */
  auto fragment_merge_start_time = std::chrono::high_resolution_clock::now();
  
  // Fragment Merge阶段：深度测试选择最近片段
  std::vector<const Fragment*> selected_fragments(width_ * height_, nullptr);
  #pragma omp parallel for
  for (size_t i = 0; i < fragmentsBuffer.size(); i++) {
    const auto &fragments = fragmentsBuffer[i];
    if (fragments.empty()) {
      continue;
    }

    const Fragment *renderFragment = nullptr;
    for (const auto &fragment : fragments) {
      if (!renderFragment || fragment.depth < renderFragment->depth) {
        renderFragment = &fragment;
      }
    }
    selected_fragments[i] = renderFragment;
  }
  auto fragment_merge_end_time = std::chrono::high_resolution_clock::now();
  auto fragment_merge_duration = std::chrono::duration_cast<std::chrono::microseconds>(
      fragment_merge_end_time - fragment_merge_start_time);
  
  // Deferred Shading阶段：执行片段着色器
  auto deferred_shading_start_time = std::chrono::high_resolution_clock::now();
#pragma omp parallel for
  for (size_t i = 0; i < selected_fragments.size(); i++) {
    const Fragment *renderFragment = selected_fragments[i];
    if (renderFragment) {
      // 添加Material指针有效性检查
      if (renderFragment->material == nullptr) {
        SPDLOG_ERROR("Fragment material is nullptr at pixel {}", i);
        continue;
      }
      auto color = shader_->FragmentShader(*renderFragment);
      buffer[i] = uint32_t(color);
    }
  }
  auto deferred_shading_end_time = std::chrono::high_resolution_clock::now();
  auto deferred_shading_duration = std::chrono::duration_cast<std::chrono::microseconds>(
      deferred_shading_end_time - deferred_shading_start_time);
  /*  *  *  *  *  *  *  */
  
  auto total_end_time = std::chrono::high_resolution_clock::now();
  auto total_duration = std::chrono::duration_cast<std::chrono::microseconds>(
      total_end_time - total_start_time);
  
  // 填充统计信息
  stats.buffer_alloc_ms = buffer_alloc_duration.count() / 1000.0;
  stats.rasterization_ms = rasterization_duration.count() / 1000.0;
  stats.fragment_collection_ms = fragment_collection_duration.count() / 1000.0;
  stats.fragment_merge_ms = fragment_merge_duration.count() / 1000.0;
  stats.deferred_shading_ms = deferred_shading_duration.count() / 1000.0;
  stats.total_ms = total_duration.count() / 1000.0;
  
  return stats;
}

Vertex SimpleRenderer::PerspectiveDivision(const Vertex &vertex) {
  Vector4f position = vertex.GetPosition();
  
  // 检查w分量，避免除零和负数问题
  if (position.w <= kMinWValue) {
    Vector4f farPosition(0.0f, 0.0f, 1.0f, 1.0f);
    return Vertex(farPosition, vertex.GetNormal(), vertex.GetTexCoords(), vertex.GetColor());
  }
  
  // 保存原始w分量用于透视矫正插值
  float original_w = position.w;
  
  // 执行透视除法：(x, y, z, w) -> (x/w, y/w, z/w, 1/w)
  Vector4f ndcPosition(
    position.x / position.w,  // x_ndc = x_clip / w_clip
    position.y / position.w,  // y_ndc = y_clip / w_clip  
    position.z / position.w,  // z_ndc = z_clip / w_clip
    1.0f / original_w         // 保存1/w用于透视矫正插值
  );
  
  // 只对Z坐标进行深度范围限制，X和Y允许超出以支持屏幕外三角形
  // 这些坐标在后续的视口变换和裁剪阶段会被正确处理
  ndcPosition.z = std::clamp(ndcPosition.z, -1.0f, 1.0f);
  
  // 创建新的顶点，保持其他属性和裁剪空间坐标不变
  return Vertex(ndcPosition, vertex.GetNormal(), vertex.GetTexCoords(), vertex.GetColor(), vertex.GetClipPosition());
}

Vertex SimpleRenderer::ViewportTransformation(const Vertex &vertex) {
  Vector4f ndcPosition = vertex.GetPosition();
  
  // 视口变换：将NDC坐标[-1,1]转换为屏幕坐标[0,width]x[0,height]
  float screen_x = (ndcPosition.x + 1.0f) * width_ / 2.0f;
  float screen_y = (1.0f - ndcPosition.y) * height_ / 2.0f;
  
  Vector4f screenPosition(
    screen_x,                    // x: 屏幕坐标
    screen_y,                    // y: 屏幕坐标
    ndcPosition.z,               // z: NDC坐标用于深度测试
    ndcPosition.w                // w: 保持1/w用于透视矫正插值
  );
  
  return Vertex(screenPosition, vertex.GetNormal(), vertex.GetTexCoords(), vertex.GetColor());
}



// SoA优化的Binning：两遍计数 + 预留 + 填充 TriangleRef
void SimpleRenderer::TriangleTileBinning(
    const Model &model,
    const VertexSoA &soa,
    std::vector<std::vector<TriangleRef>> &tile_triangles,
    size_t tiles_x, size_t tiles_y, size_t tile_size) {
  const size_t total_triangles = model.GetFaces().size();

  SPDLOG_INFO("Starting triangle-tile binning (SoA) for {} triangles", total_triangles);
  SPDLOG_INFO("Screen dimensions: {}x{}, Tile size: {}, Tiles: {}x{}",
              width_, height_, tile_size, tiles_x, tiles_y);

  std::vector<size_t> tile_counts(tiles_x * tiles_y, 0);

  auto process_triangle = [&](size_t tri_idx, bool count_only) {
    const auto &f = model.GetFaces()[tri_idx];
    size_t i0 = f.GetIndex(0);
    size_t i1 = f.GetIndex(1);
    size_t i2 = f.GetIndex(2);

    // 视锥体裁剪 (裁剪空间)
    // 保守视锥体裁剪：只有当整个三角形都在视锥体外同一侧时才裁剪
    const Vector4f &c0 = soa.pos_clip[i0];
    const Vector4f &c1 = soa.pos_clip[i1];
    const Vector4f &c2 = soa.pos_clip[i2];
    bool frustum_cull =
        (c0.x > c0.w && c1.x > c1.w && c2.x > c2.w) ||  // 右平面外
        (c0.x < -c0.w && c1.x < -c0.w && c2.x < -c0.w) || // 左平面外
        (c0.y > c0.w && c1.y > c1.w && c2.y > c2.w) ||  // 上平面外
        (c0.y < -c0.w && c1.y < -c0.w && c2.y < -c0.w) || // 下平面外
        (c0.z > c0.w && c1.z > c1.w && c2.z > c2.w) ||  // 远平面外
        (c0.z < -c0.w && c1.z < -c0.w && c2.z < -c0.w);  // 近平面外
    if (frustum_cull) {
      return;
    }

    const Vector4f &pos0 = soa.pos_screen[i0];
    const Vector4f &pos1 = soa.pos_screen[i1];
    const Vector4f &pos2 = soa.pos_screen[i2];

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

    int start_tile_x = std::max(0, static_cast<int>(min_x) / static_cast<int>(tile_size));
    int end_tile_x   = std::min(static_cast<int>(tiles_x - 1), static_cast<int>(max_x) / static_cast<int>(tile_size));
    int start_tile_y = std::max(0, static_cast<int>(min_y) / static_cast<int>(tile_size));
    int end_tile_y   = std::min(static_cast<int>(tiles_y - 1), static_cast<int>(max_y) / static_cast<int>(tile_size));
    if (start_tile_x > end_tile_x || start_tile_y > end_tile_y) return; // 如果bbox不在任何tile内，直接返回

    if (count_only) { // 第一遍计数，只统计tile内三角形数量
      for (int ty = start_tile_y; ty <= end_tile_y; ++ty) {
        for (int tx = start_tile_x; tx <= end_tile_x; ++tx) {
          size_t tile_id = ty * tiles_x + tx;
          tile_counts[tile_id]++;
        }
      }
    } else { // 第二遍填充，填充TriangleRef
      TriangleRef tri_ref{ i0, i1, i2, &f.GetMaterial(), tri_idx };
      for (int ty = start_tile_y; ty <= end_tile_y; ++ty) {
        for (int tx = start_tile_x; tx <= end_tile_x; ++tx) {
          size_t tile_id = ty * tiles_x + tx;
          tile_triangles[tile_id].push_back(tri_ref);
        }
      }
    }
  };

  // 第一遍（count only）：计算每个tile需要容纳多少三角形
  for (size_t tri_idx = 0; tri_idx < total_triangles; ++tri_idx) {
    process_triangle(tri_idx, true);
  }

  // 预分配，避免动态扩容
  for (size_t tile_id = 0; tile_id < tile_triangles.size(); ++tile_id) {
    if (tile_counts[tile_id] > 0) tile_triangles[tile_id].reserve(tile_counts[tile_id]);
  }

  // 第二遍（fill）：按范围填充TriangleRef
  for (size_t tri_idx = 0; tri_idx < total_triangles; ++tri_idx) {
    process_triangle(tri_idx, false);
  }

  size_t total_triangle_refs = 0;
  size_t non_empty_tiles = 0;
  for (const auto& tile : tile_triangles) {
    total_triangle_refs += tile.size();
    if (!tile.empty()) non_empty_tiles++;
  }
  SPDLOG_INFO("  (SoA) Total triangle references: {}", total_triangle_refs);
  SPDLOG_INFO("  (SoA) Non-empty tiles: {}", non_empty_tiles);
  SPDLOG_INFO("  (SoA) Average triangles per tile: {:.2f}",
              total_triangle_refs > 0 ? float(total_triangle_refs) / tile_triangles.size() : 0.0f);
}

// SoA 版：单个 tile 光栅化
void SimpleRenderer::RasterizeTile(
    size_t tile_id,
    const std::vector<TriangleRef> &triangles,
    size_t tiles_x, size_t tiles_y, size_t tile_size,
    float* tile_depth_buffer, uint32_t* tile_color_buffer,
    std::unique_ptr<float[]> &global_depth_buffer,
    std::unique_ptr<uint32_t[]> &global_color_buffer,
    const VertexSoA &soa,
    bool use_early_z,
    std::vector<Fragment>* scratch_fragments) {
  (void)tiles_y;
  // 计算 tile 屏幕范围
  size_t tile_x = tile_id % tiles_x;
  size_t tile_y = tile_id / tiles_x;
  size_t screen_x_start = tile_x * tile_size;
  size_t screen_y_start = tile_y * tile_size;
  size_t screen_x_end = std::min(screen_x_start + tile_size, width_);
  size_t screen_y_end = std::min(screen_y_start + tile_size, height_);

  // 初始化 tile 局部缓冲
  size_t tile_width = screen_x_end - screen_x_start;
  size_t tile_height = screen_y_end - screen_y_start;
  std::fill_n(tile_depth_buffer, tile_width * tile_height, 1.0f);
  std::fill_n(tile_color_buffer, tile_width * tile_height, 0); // 默认背景色为0/黑色

  for (const auto &tri : triangles) { // 用来应对scratch传入nullptr的情况
    // 始终走 SoA + 限制矩形的光栅化路径；如未提供 scratch，则使用函数内局部容器
    std::vector<Fragment> local_out;
    std::vector<Fragment> &out = scratch_fragments ? *scratch_fragments : local_out;

    out.clear();
    if (out.capacity() < tile_width * tile_height) {
      out.reserve(tile_width * tile_height);
    }

    rasterizer_->RasterizeTo(soa, tri.i0, tri.i1, tri.i2,
                             static_cast<int>(screen_x_start), static_cast<int>(screen_y_start),
                             static_cast<int>(screen_x_end),   static_cast<int>(screen_y_end),
                             out);

    for (auto &fragment : out) {
      fragment.material = tri.material;
      size_t sx = fragment.screen_coord[0];
      size_t sy = fragment.screen_coord[1];
      if (sx >= screen_x_start && sx < screen_x_end && sy >= screen_y_start && sy < screen_y_end) {
        size_t local_x = sx - screen_x_start;
        size_t local_y = sy - screen_y_start;
        size_t idx = local_x + local_y * tile_width;
        if (use_early_z) {
          if (fragment.depth < tile_depth_buffer[idx]) {
            auto color = shader_->FragmentShader(fragment);
            tile_depth_buffer[idx] = fragment.depth;
            tile_color_buffer[idx] = uint32_t(color);
          }
        } else {
          auto color = shader_->FragmentShader(fragment);
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
    const size_t tile_row_off   = y * tile_width;
    const size_t global_row_off = (screen_y_start + y) * width_ + screen_x_start;

    // 拷贝本行 color 到全局 color
    std::memcpy(global_color_buffer.get() + global_row_off,
                tile_color_buffer + tile_row_off,
                tile_width * sizeof(uint32_t));

    // 拷贝本行 depth 到全局 depth
    std::memcpy(global_depth_buffer.get() + global_row_off,
                tile_depth_buffer + tile_row_off,
                tile_width * sizeof(float));
  }
}

// 基础光栅化管线实现
SimpleRenderer::RenderStats SimpleRenderer::ExecuteTraditionalPipeline(
    const Model &model, 
    const std::vector<Vertex> &processedVertices,
    uint32_t *buffer) {
    
    RenderStats stats;
    auto total_start_time = std::chrono::high_resolution_clock::now();
    
    // 1. 为每个线程创建framebuffer
    auto buffer_alloc_start_time = std::chrono::high_resolution_clock::now();
    std::vector<std::unique_ptr<float[]>> depthBuffer_all_thread(kNProc);
    std::vector<std::unique_ptr<uint32_t[]>> colorBuffer_all_thread(kNProc);
    
    for (size_t thread_id = 0; thread_id < kNProc; thread_id++) {
        depthBuffer_all_thread[thread_id] = 
            std::make_unique<float[]>(width_ * height_);
        colorBuffer_all_thread[thread_id] = 
            std::make_unique<uint32_t[]>(width_ * height_);
        
        std::fill_n(depthBuffer_all_thread[thread_id].get(), width_ * height_,
                    std::numeric_limits<float>::infinity());
        std::fill_n(colorBuffer_all_thread[thread_id].get(), width_ * height_, 0);
    }
    auto buffer_alloc_end_time = std::chrono::high_resolution_clock::now();
    auto buffer_alloc_duration = std::chrono::duration_cast<std::chrono::microseconds>(
        buffer_alloc_end_time - buffer_alloc_start_time);
    
    // 2. 并行光栅化
    auto raster_start_time = std::chrono::high_resolution_clock::now();
#pragma omp parallel num_threads(kNProc) default(none) \
    shared(processedVertices, rasterizer_, shader_, width_, height_, \
           depthBuffer_all_thread, colorBuffer_all_thread, model)
    {
        int thread_id = omp_get_thread_num();
        auto &depthBuffer_per_thread = depthBuffer_all_thread[thread_id];
        auto &colorBuffer_per_thread = colorBuffer_all_thread[thread_id];
        
#pragma omp for
        for (const auto &f : model.GetFaces()) {
            auto v0 = processedVertices[f.GetIndex(0)];
            auto v1 = processedVertices[f.GetIndex(1)];
            auto v2 = processedVertices[f.GetIndex(2)];

            // 获取屏幕空间坐标
            Vector2f screen0(v0.GetPosition().x, v0.GetPosition().y);
            Vector2f screen1(v1.GetPosition().x, v1.GetPosition().y);  
            Vector2f screen2(v2.GetPosition().x, v2.GetPosition().y);
            
            // 计算屏幕空间叉积判断朝向
            Vector2f edge1 = screen1 - screen0;
            Vector2f edge2 = screen2 - screen0;
            float cross_product = edge1.x * edge2.y - edge1.y * edge2.x;
            
            // 背面剔除：NDC空间中叉积为负表示顺时针，即背面。
            // 从NDC到屏幕空间中，会发生Y轴翻转，对应叉积应为正。
            if (cross_product > 0.0f) {
                continue;
            }

            const Material *material = &f.GetMaterial();
            auto fragments = rasterizer_->Rasterize(v0, v1, v2);

            for (auto &fragment : fragments) {
                fragment.material = material;
                size_t x = fragment.screen_coord[0];
                size_t y = fragment.screen_coord[1];

                if (x >= width_ || y >= height_) {
                    continue;
                }

                size_t index = x + y * width_;
                if (fragment.depth < depthBuffer_per_thread[index]) {
                    depthBuffer_per_thread[index] = fragment.depth;
                    auto color = shader_->FragmentShader(fragment);
                    colorBuffer_per_thread[index] = uint32_t(color);
                }
            }
        }
    }
    auto raster_end_time = std::chrono::high_resolution_clock::now();
    auto raster_duration = std::chrono::duration_cast<std::chrono::microseconds>(
        raster_end_time - raster_start_time);
    
    // 3. 合并结果
    auto merge_start_time = std::chrono::high_resolution_clock::now();
    std::unique_ptr<float[]> depthBuffer = 
        std::make_unique<float[]>(width_ * height_);
    std::unique_ptr<uint32_t[]> colorBuffer = 
        std::make_unique<uint32_t[]>(width_ * height_);

    std::fill_n(depthBuffer.get(), width_ * height_,
                std::numeric_limits<float>::infinity());
    std::fill_n(colorBuffer.get(), width_ * height_, 0);

#pragma omp parallel for
    for (size_t i = 0; i < width_ * height_; i++) {
        float min_depth = std::numeric_limits<float>::infinity();
        uint32_t color = 0;

        for (size_t thread_id = 0; thread_id < kNProc; thread_id++) {
            float depth = depthBuffer_all_thread[thread_id][i];
            if (depth < min_depth) {
                min_depth = depth;
                color = colorBuffer_all_thread[thread_id][i];
            }
        }
        depthBuffer[i] = min_depth;
        colorBuffer[i] = color;
    }

    std::memcpy(buffer, colorBuffer.get(), width_ * height_ * sizeof(uint32_t));
    auto merge_end_time = std::chrono::high_resolution_clock::now();
    auto merge_duration = std::chrono::duration_cast<std::chrono::microseconds>(
        merge_end_time - merge_start_time);
    
    auto total_end_time = std::chrono::high_resolution_clock::now();
    auto total_duration = std::chrono::duration_cast<std::chrono::microseconds>(
        total_end_time - total_start_time);
    
    // 填充统计信息
    stats.buffer_alloc_ms = buffer_alloc_duration.count() / 1000.0;
    stats.rasterization_ms = raster_duration.count() / 1000.0;
    stats.merge_ms = merge_duration.count() / 1000.0;
    stats.total_ms = total_duration.count() / 1000.0;
    
    return stats;
}


// Tile-based光栅化管线实现（SoA 直连版本，避免 AoS->SoA 拷贝）
SimpleRenderer::TileRenderStats SimpleRenderer::ExecuteTileBasedPipeline(
    const Model &model,
    const VertexSoA &soa,
    uint32_t *buffer) {
    TileRenderStats stats;
    auto total_start_time = std::chrono::high_resolution_clock::now();

    // 1. Setup阶段
    auto setup_start_time = std::chrono::high_resolution_clock::now();
    const size_t TILE_SIZE = kDefaultTileSize; // Default tile size per tile
    const size_t tiles_x = (width_ + TILE_SIZE - 1) / TILE_SIZE;
    const size_t tiles_y = (height_ + TILE_SIZE - 1) / TILE_SIZE;
    const size_t total_tiles = tiles_x * tiles_y;

    // 为每个tile创建三角形列表（SoA 引用）
    std::vector<std::vector<TriangleRef>> tile_triangles(total_tiles);
    auto setup_end_time = std::chrono::high_resolution_clock::now();
    auto setup_duration = std::chrono::duration_cast<std::chrono::microseconds>(
        setup_end_time - setup_start_time);

    // 2. Triangle-Tile binning阶段（SoA）
    auto binning_start_time = std::chrono::high_resolution_clock::now();
    TriangleTileBinning(model, soa, tile_triangles, tiles_x, tiles_y, TILE_SIZE);
    auto binning_end_time = std::chrono::high_resolution_clock::now();
    auto binning_duration = std::chrono::duration_cast<std::chrono::microseconds>(
        binning_end_time - binning_start_time);

    // 3. 全局 framebuffer（单份）
    // 直接让每个 tile 写入这份全局缓冲区，避免末端 O(W*H*kNProc) 合并开销
    auto buffer_alloc_start_time = std::chrono::high_resolution_clock::now();
    std::unique_ptr<float[]> depthBuffer = std::make_unique<float[]>(width_ * height_);
    std::unique_ptr<uint32_t[]> colorBuffer = std::make_unique<uint32_t[]>(width_ * height_);
    // 深度初始化为最远值，颜色清零
    std::fill_n(depthBuffer.get(), width_ * height_, std::numeric_limits<float>::infinity());
    std::fill_n(colorBuffer.get(), width_ * height_, 0);
    auto buffer_alloc_end_time = std::chrono::high_resolution_clock::now();
    auto buffer_alloc_duration = std::chrono::duration_cast<std::chrono::microseconds>(
        buffer_alloc_end_time - buffer_alloc_start_time);

    // 4. 并行处理每个tile（SoA）
    auto rasterization_start_time = std::chrono::high_resolution_clock::now();
#pragma omp parallel num_threads(kNProc) default(none) \
    shared(tile_triangles, rasterizer_, shader_, width_, height_, \
           depthBuffer, colorBuffer, tiles_x, tiles_y, total_tiles, \
           is_early_z_enabled_, soa)
    {
        int thread_id = omp_get_thread_num();

        // 为当前线程创建 tile 局部缓冲区（避免在全局缓冲上直接逐像素竞争）
        std::unique_ptr<float[]> tile_depth_buffer = 
            std::make_unique<float[]>(TILE_SIZE * TILE_SIZE);
        std::unique_ptr<uint32_t[]> tile_color_buffer = 
            std::make_unique<uint32_t[]>(TILE_SIZE * TILE_SIZE);

        // 线程本地片段 scratch 容器（复用），容量按单 tile 上限预估
        std::vector<Fragment> scratch_fragments;
        scratch_fragments.reserve(TILE_SIZE * TILE_SIZE);

#pragma omp for
        for (size_t tile_id = 0; tile_id < total_tiles; tile_id++) {
            // 按照 tile 进行光栅化（SoA）
            // 直接写入单份全局 framebuffer；不同 tile 不重叠，无需加锁
            RasterizeTile(tile_id, tile_triangles[tile_id],
                          tiles_x, tiles_y, TILE_SIZE,
                          tile_depth_buffer.get(), tile_color_buffer.get(),
                          depthBuffer, colorBuffer,
                          soa, is_early_z_enabled_, &scratch_fragments);
        }
    }
    auto rasterization_end_time = std::chrono::high_resolution_clock::now();
    auto rasterization_duration = std::chrono::duration_cast<std::chrono::microseconds>(
        rasterization_end_time - rasterization_start_time);

    // 5. 直接将单份全局 colorBuffer 拷贝到输出
    auto present_start_time = std::chrono::high_resolution_clock::now();
    std::memcpy(buffer, colorBuffer.get(), width_ * height_ * sizeof(uint32_t));
    auto present_end_time = std::chrono::high_resolution_clock::now();
    auto present_duration = std::chrono::duration_cast<std::chrono::microseconds>(
        present_end_time - present_start_time);

    auto total_end_time = std::chrono::high_resolution_clock::now();
    auto total_duration = std::chrono::duration_cast<std::chrono::microseconds>(
        total_end_time - total_start_time);

    // 填充统计信息
    stats.setup_ms = setup_duration.count() / 1000.0;
    stats.binning_ms = binning_duration.count() / 1000.0;
    stats.buffer_alloc_ms = buffer_alloc_duration.count() / 1000.0;
    stats.rasterization_ms = rasterization_duration.count() / 1000.0;
    // 合并阶段已被消除，仅为拷贝开销
    stats.merge_ms = present_duration.count() / 1000.0;
    stats.total_ms = total_duration.count() / 1000.0;

    return stats;
}

void SimpleRenderer::PrintTraditionalStats(double vertex_ms, const RenderStats& stats) const {
  double total_ms = vertex_ms + stats.total_ms;
  
  SPDLOG_INFO("=== TRADITIONAL RENDERING PERFORMANCE ===");
  SPDLOG_INFO("Vertex Shader:    {:8.3f} ms ({:5.1f}%)", vertex_ms, vertex_ms/total_ms*100);
  SPDLOG_INFO("Buffer Alloc:     {:8.3f} ms ({:5.1f}%)", stats.buffer_alloc_ms, stats.buffer_alloc_ms/total_ms*100);
  SPDLOG_INFO("Rasterization:    {:8.3f} ms ({:5.1f}%)", stats.rasterization_ms, stats.rasterization_ms/total_ms*100);
  SPDLOG_INFO("Merge:            {:8.3f} ms ({:5.1f}%)", stats.merge_ms, stats.merge_ms/total_ms*100);
  SPDLOG_INFO("Total:            {:8.3f} ms", total_ms);
  SPDLOG_INFO("==========================================");
}

void SimpleRenderer::PrintTileBasedStats(double vertex_ms, const TileRenderStats& stats) const {
  double total_ms = vertex_ms + stats.total_ms;
  
  SPDLOG_INFO("=== TILE-BASED RENDERING PERFORMANCE ===");
  SPDLOG_INFO("Vertex Shader:    {:8.3f} ms ({:5.1f}%)", vertex_ms, vertex_ms/total_ms*100);
  SPDLOG_INFO("Setup:            {:8.3f} ms ({:5.1f}%)", stats.setup_ms, stats.setup_ms/total_ms*100);
  SPDLOG_INFO("Binning:          {:8.3f} ms ({:5.1f}%)", stats.binning_ms, stats.binning_ms/total_ms*100);
  SPDLOG_INFO("Buffer Alloc:     {:8.3f} ms ({:5.1f}%)", stats.buffer_alloc_ms, stats.buffer_alloc_ms/total_ms*100);
  SPDLOG_INFO("Rasterization:    {:8.3f} ms ({:5.1f}%)", stats.rasterization_ms, stats.rasterization_ms/total_ms*100);
  SPDLOG_INFO("Merge:            {:8.3f} ms ({:5.1f}%)", stats.merge_ms, stats.merge_ms/total_ms*100);
  SPDLOG_INFO("Total:            {:8.3f} ms", total_ms);
  SPDLOG_INFO("==========================================");
}

void SimpleRenderer::PrintDeferredStats(double vertex_ms, const DeferredRenderStats& stats) const {
  double total_ms = vertex_ms + stats.total_ms;
  
  SPDLOG_INFO("=== DEFERRED RENDERING PERFORMANCE ===");
  SPDLOG_INFO("Vertex Shader:        {:8.3f} ms ({:5.1f}%)", vertex_ms, vertex_ms/total_ms*100);
  SPDLOG_INFO("Buffer Alloc:         {:8.3f} ms ({:5.1f}%)", stats.buffer_alloc_ms, stats.buffer_alloc_ms/total_ms*100);
  SPDLOG_INFO("Rasterization:        {:8.3f} ms ({:5.1f}%)", stats.rasterization_ms, stats.rasterization_ms/total_ms*100);
  SPDLOG_INFO("Fragment Collection:  {:8.3f} ms ({:5.1f}%)", stats.fragment_collection_ms, stats.fragment_collection_ms/total_ms*100);
  SPDLOG_INFO("Fragment Merge:       {:8.3f} ms ({:5.1f}%)", stats.fragment_merge_ms, stats.fragment_merge_ms/total_ms*100);
  SPDLOG_INFO("Deferred Shading:     {:8.3f} ms ({:5.1f}%)", stats.deferred_shading_ms, stats.deferred_shading_ms/total_ms*100);
  SPDLOG_INFO("Total:                {:8.3f} ms", total_ms);
  SPDLOG_INFO("=========================================");
}

}  // namespace simple_renderer
