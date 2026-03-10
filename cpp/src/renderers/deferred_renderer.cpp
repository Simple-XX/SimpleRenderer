#include "renderers/deferred_renderer.hpp"

#include <omp.h>
#include <algorithm>
#include <chrono>
#include <cassert>
#include <iterator>

#include "config.h"
#include "log_system.h"

namespace simple_renderer {

bool DeferredRenderer::Render(const Model& model, const Shader& shader_in, uint32_t* buffer) {
  auto total_start_time = std::chrono::high_resolution_clock::now();
  auto shader = std::make_shared<Shader>(shader_in);
  shader->PrepareUniformCaches();

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
  auto vertex_ms = std::chrono::duration_cast<std::chrono::microseconds>(vertex_end - vertex_start).count() / 1000.0;

  // Buffer allocation
  auto buffer_alloc_start = std::chrono::high_resolution_clock::now();
  std::vector<std::vector<std::vector<Fragment>>> fragmentsBuffer_all_thread(
      kNProc, std::vector<std::vector<Fragment>>(width_ * height_));

  std::vector<Material> material_cache;
  material_cache.reserve(model.GetFaces().size());
  for (const auto &f : model.GetFaces()) {
    material_cache.emplace_back(f.GetMaterial());
  }
  auto buffer_alloc_end = std::chrono::high_resolution_clock::now();
  auto buffer_alloc_ms = std::chrono::duration_cast<std::chrono::microseconds>(buffer_alloc_end - buffer_alloc_start).count() / 1000.0;

  // Rasterization: collect fragments per pixel per thread
  auto raster_start = std::chrono::high_resolution_clock::now();
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

        if (x >= width_ || y >= height_) continue;
        size_t index = x + y * width_;
        fragmentsBuffer_per_thread[index].push_back(fragment);
      }
    }
  }
  auto raster_end = std::chrono::high_resolution_clock::now();
  auto raster_ms = std::chrono::duration_cast<std::chrono::microseconds>(raster_end - raster_start).count() / 1000.0;

  /* * * Fragment Collection * * */
  auto collect_start = std::chrono::high_resolution_clock::now();

  const size_t pixel_count = static_cast<size_t>(width_) * static_cast<size_t>(height_);

#ifndef NDEBUG
  for (const auto &tb : fragmentsBuffer_all_thread) {
    // 断言避免越界，确保固定维度
    assert(tb.size() == pixel_count && "thread buffer size mismatch");
  }
#endif

  // Pass 1: 统计每个像素桶的总片元数
  std::vector<size_t> bucket_total(pixel_count, 0);
  for (const auto &tb : fragmentsBuffer_all_thread) {
    for (size_t i = 0; i < pixel_count; ++i) {
      bucket_total[i] += tb[i].size();
    }
  }

  // Pass 2: 统一预分配
  std::vector<std::vector<Fragment>> fragmentsBuffer(pixel_count);
  for (size_t i = 0; i < pixel_count; ++i) {
    if (bucket_total[i] > 0) fragmentsBuffer[i].reserve(bucket_total[i]);
  }

  // Pass 3: 按桶并行合并（每个桶内部保持按线程序的插入顺序）
#pragma omp parallel for num_threads(kNProc) schedule(static)
  for (long long i = 0; i < static_cast<long long>(pixel_count); ++i) {
    auto &dst = fragmentsBuffer[static_cast<size_t>(i)];
    for (size_t t = 0; t < fragmentsBuffer_all_thread.size(); ++t) {
      auto &src = fragmentsBuffer_all_thread[t][static_cast<size_t>(i)];
      dst.insert(dst.end(),
                 std::make_move_iterator(src.begin()),
                 std::make_move_iterator(src.end()));
      src.clear();
    }
  }
  auto collect_end = std::chrono::high_resolution_clock::now();
  auto collect_ms = std::chrono::duration_cast<std::chrono::microseconds>(collect_end - collect_start).count() / 1000.0;

  /* * * Fragment Merge & Deferred Shading * * */
  auto merge_start = std::chrono::high_resolution_clock::now();

  // Fragment Merge阶段：深度测试选择最近片段
  std::vector<const Fragment*> selected_fragments(width_ * height_, nullptr);
#pragma omp parallel for
  for (size_t i = 0; i < fragmentsBuffer.size(); i++) {
    const auto &fragments = fragmentsBuffer[i];
    if (fragments.empty()) continue;
    const Fragment *renderFragment = nullptr;
    for (const auto &fragment : fragments) {
      if (!renderFragment || fragment.depth < renderFragment->depth) {
        renderFragment = &fragment;
      }
    }
    selected_fragments[i] = renderFragment;
  }
  auto merge_end = std::chrono::high_resolution_clock::now();
  auto merge_ms = std::chrono::duration_cast<std::chrono::microseconds>(merge_end - merge_start).count() / 1000.0;

  // Deferred Shading阶段：对选择的片段执行片段着色
  auto shade_start = std::chrono::high_resolution_clock::now();
#pragma omp parallel for
  for (size_t i = 0; i < selected_fragments.size(); i++) {
    const Fragment *renderFragment = selected_fragments[i];
    if (renderFragment) {
      // 添加Material指针有效性检查
      if (renderFragment->material == nullptr) {
        SPDLOG_ERROR("Fragment material is nullptr at pixel {}", i);
        continue;
      }
      auto color = shader->FragmentShader(*renderFragment);
      buffer[i] = uint32_t(color);
    }
  }
  auto shade_end = std::chrono::high_resolution_clock::now();
  auto shade_ms = std::chrono::duration_cast<std::chrono::microseconds>(shade_end - shade_start).count() / 1000.0;

  auto total_end_time = std::chrono::high_resolution_clock::now();
  double total_ms = std::chrono::duration_cast<std::chrono::microseconds>(total_end_time - total_start_time).count() / 1000.0;

  SPDLOG_DEBUG("=== DEFERRED RENDERING PERFORMANCE ===");
  double sum_ms = vertex_ms + (total_ms - vertex_ms);
  SPDLOG_DEBUG("Vertex Shader:        {:8.3f} ms ({:5.1f}%)", vertex_ms, vertex_ms/sum_ms*100);
  SPDLOG_DEBUG("Buffer Alloc:         {:8.3f} ms", buffer_alloc_ms);
  SPDLOG_DEBUG("Rasterization:        {:8.3f} ms", raster_ms);
  SPDLOG_DEBUG("Fragment Collection:  {:8.3f} ms", collect_ms);
  SPDLOG_DEBUG("Fragment Merge:       {:8.3f} ms", merge_ms);
  SPDLOG_DEBUG("Deferred Shading:     {:8.3f} ms", shade_ms);
  SPDLOG_DEBUG("Total:                {:8.3f} ms", vertex_ms + (buffer_alloc_ms + raster_ms + collect_ms + merge_ms + shade_ms));
  SPDLOG_DEBUG("=========================================");

  return true;
}

}  // namespace simple_renderer
