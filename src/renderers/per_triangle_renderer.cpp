#include "renderers/per_triangle_renderer.hpp"

#include <omp.h>

#include <algorithm>
#include <chrono>
#include <cstring>
#include <limits>
#include <memory>

#include "config.h"
#include "log_system.h"

namespace simple_renderer {

bool PerTriangleRenderer::Render(const Model &model, const Shader &shader_in,
                                 uint32_t *buffer) {
  auto total_start_time = std::chrono::high_resolution_clock::now();

  // 复制 shader 以便在多线程中共享
  auto shader = std::make_shared<Shader>(shader_in);

  // 顶点变换（AoS）
  auto vertex_start = std::chrono::high_resolution_clock::now();
  const auto &input_vertices = model.GetVertices();
  std::vector<Vertex> processedVertices(input_vertices.size());

#pragma omp parallel for num_threads(kNProc) schedule(static) \
    shared(shader, processedVertices, input_vertices)
  for (size_t i = 0; i < input_vertices.size(); ++i) {
    const auto &v = input_vertices[i];
    auto clipSpaceVertex = shader->VertexShader(v);
    auto ndcVertex = PerspectiveDivision(clipSpaceVertex);
    auto screenSpaceVertex = ViewportTransformation(ndcVertex);
    processedVertices[i] = screenSpaceVertex;
  }
  auto vertex_end = std::chrono::high_resolution_clock::now();
  auto vertex_ms = std::chrono::duration_cast<std::chrono::microseconds>(
                       vertex_end - vertex_start)
                       .count() /
                   1000.0;

  // 1. 为每个线程创建framebuffer
  auto buffer_alloc_start = std::chrono::high_resolution_clock::now();
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
  auto buffer_alloc_end = std::chrono::high_resolution_clock::now();
  auto buffer_alloc_ms = std::chrono::duration_cast<std::chrono::microseconds>(
                             buffer_alloc_end - buffer_alloc_start)
                             .count() /
                         1000.0;

  // 2. 并行光栅化
  auto raster_start = std::chrono::high_resolution_clock::now();
#pragma omp parallel num_threads(kNProc) default(none)              \
    shared(processedVertices, shader, rasterizer_, width_, height_, \
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

      // 背面剔除（屏幕空间叉积）
      Vector2f screen0(v0.GetPosition().x, v0.GetPosition().y);
      Vector2f screen1(v1.GetPosition().x, v1.GetPosition().y);
      Vector2f screen2(v2.GetPosition().x, v2.GetPosition().y);

      // 计算屏幕空间叉积判断朝向
      Vector2f edge1 = screen1 - screen0;
      Vector2f edge2 = screen2 - screen0;

      // 背面剔除：NDC空间中叉积为负表示顺时针，即背面。
      // 从NDC到屏幕空间中，会发生Y轴翻转，对应叉积应为正。
      float cross_product = edge1.x * edge2.y - edge1.y * edge2.x;
      if (cross_product > 0.0f) {
        continue;  // 背面
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
          auto color = shader->FragmentShader(fragment);
          colorBuffer_per_thread[index] = uint32_t(color);
        }
      }
    }
  }
  auto raster_end = std::chrono::high_resolution_clock::now();
  auto raster_ms = std::chrono::duration_cast<std::chrono::microseconds>(
                       raster_end - raster_start)
                       .count() /
                   1000.0;

  // 3. 合并结果
  auto merge_start = std::chrono::high_resolution_clock::now();
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
  auto merge_end = std::chrono::high_resolution_clock::now();
  auto merge_ms = std::chrono::duration_cast<std::chrono::microseconds>(
                      merge_end - merge_start)
                      .count() /
                  1000.0;

  auto total_end_time = std::chrono::high_resolution_clock::now();
  auto total_ms = std::chrono::duration_cast<std::chrono::microseconds>(
                      total_end_time - total_start_time)
                      .count() /
                  1000.0;

  SPDLOG_DEBUG("=== PER-TRIANGLE RENDERING PERFORMANCE ===");
  double sum_ms = vertex_ms + (total_ms - vertex_ms);
  SPDLOG_DEBUG("Vertex Shader:    {:8.3f} ms ({:5.1f}%)", vertex_ms,
              vertex_ms / sum_ms * 100);
  SPDLOG_DEBUG("Buffer Alloc:     {:8.3f} ms", buffer_alloc_ms);
  SPDLOG_DEBUG("Rasterization:    {:8.3f} ms", raster_ms);
  SPDLOG_DEBUG("Merge:            {:8.3f} ms", merge_ms);
  SPDLOG_DEBUG("Total:            {:8.3f} ms",
              vertex_ms + (buffer_alloc_ms + raster_ms + merge_ms));
  SPDLOG_DEBUG("==========================================");

  return true;
}

}  // namespace simple_renderer
