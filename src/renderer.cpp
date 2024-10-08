
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

SimpleRenderer::SimpleRenderer(size_t width, size_t height)
    : height_(height),
      width_(width),
      log_system_(LogSystem(kLogFilePath, kLogFileMaxSize, kLogFileMaxCount)) {
  rasterizer_ = std::make_shared<Rasterizer>(width, height);
}

bool SimpleRenderer::Render(const Model &model, const Shader &shader,
                            uint32_t *buffer) {
  SPDLOG_INFO("render model: {}", model.GetModelPath());
  shader_ = std::make_shared<Shader>(shader);
  DrawModel(model, buffer);
  return true;
}

/*
Optimizes performance by performing depth testing during rasterization, keeping
only the closest fragment per pixel, and avoiding storing all
fragments—resulting in faster rendering.

通过在光栅化过程中执行深度测试，仅保留每个像素的深度值最近的片段，避免存储所有片段，从而优化性能，实现更快的渲染。
*/
void SimpleRenderer::DrawModel(const Model &model, uint32_t *buffer) {
  SPDLOG_INFO("draw {}", model.GetModelPath());

  /* * * Vertex Shader * * */
  std::vector<Vertex> processedVertices;
  std::vector<std::vector<Vertex>> processed_vertices_all_thread(kNProc);
#pragma omp parallel num_threads(kNProc) default(none) \
    shared(shader_, processed_vertices_all_thread) firstprivate(model)
  {
    int thread_id = omp_get_thread_num();
    std::vector<Vertex> &processedVertices_per_thread =
        processed_vertices_all_thread[thread_id];

#pragma omp for
    for (const auto &v : model.GetVertices()) {
      auto vertex = shader_->VertexShader(v);
      processedVertices_per_thread.push_back(vertex);
    }
  }

  for (const auto &processedVertices_per_thread :
       processed_vertices_all_thread) {
    processedVertices.insert(processedVertices.end(),
                             processedVertices_per_thread.begin(),
                             processedVertices_per_thread.end());
  }
  /*  *  *  *  *  *  *  */

  /* * * Rasterization * * */
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

#pragma omp parallel num_threads(kNProc) default(none) \ 
  shared(processedVertices, rasterizer_, shader_, width_, height_, \
             depthBuffer_all_thread, colorBuffer_all_thread)       \
    firstprivate(model)
  {
    int thread_id = omp_get_thread_num();
    auto &depthBuffer_per_thread = depthBuffer_all_thread[thread_id];
    auto &colorBuffer_per_thread = colorBuffer_all_thread[thread_id];
#pragma omp for
    for (const auto &f : model.GetFaces()) {
      auto v0 = processedVertices[f.GetIndex(0)];
      auto v1 = processedVertices[f.GetIndex(1)];
      auto v2 = processedVertices[f.GetIndex(2)];

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

          /* * * Fragment Shader * * */
          auto color = shader_->FragmentShader(fragment);
          colorBuffer_per_thread[index] = uint32_t(color);
        }
      }
    }
  }

  // Merge
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
}

/*
Organizes processing to simulate how OpenGL works with GPUs by collecting all
fragments per pixel before processing, closely mimicking the GPU pipeline but
leading to increased memory usage and slower performance.

组织处理方式模拟 OpenGL 在 GPU
上的工作原理，先收集每个像素的所有片段再并行处理屏幕上的每个像素，模仿 GPU
管线，但导致内存使用增加和渲染速度变慢
*/
void SimpleRenderer::DrawModelSlower(const Model &model, uint32_t *buffer) {
  SPDLOG_INFO("draw {}", model.GetModelPath());

  /* * * Vertex Shader * * */
  std::vector<Vertex> processedVertex;
  std::vector<std::vector<Vertex>> processed_vertices_per_thread(kNProc);
#pragma omp parallel num_threads(kNProc) default(none) \
    shared(shader_, processed_vertices_per_thread) firstprivate(model)
  {
    int thread_id = omp_get_thread_num();
    std::vector<Vertex> &local_vertices =
        processed_vertices_per_thread[thread_id];

#pragma omp for
    for (const auto &v : model.GetVertices()) {
      /* * * Vertex Shader * *  */
      auto vertex = shader_->VertexShader(v);
      local_vertices.push_back(vertex);
    }
  }

  for (const auto &local_vertices : processed_vertices_per_thread) {
    processedVertex.insert(processedVertex.end(), local_vertices.begin(),
                           local_vertices.end());
  }
  /*  *  *  *  *  *  *  */

  /* * * Rasterization * * */
  std::vector<std::vector<std::vector<Fragment>>> fragmentsBuffer_all_thread(
      kNProc, std::vector<std::vector<Fragment>>(width_ * height_));

#pragma omp parallel num_threads(kNProc) default(none)                       \
    shared(processedVertex, fragmentsBuffer_all_thread, rasterizer_, width_, \
               height_) firstprivate(model)
  {
    int thread_id = omp_get_thread_num();
    auto &fragmentsBuffer_per_thread = fragmentsBuffer_all_thread[thread_id];

#pragma omp for
    for (const auto &f : model.GetFaces()) {
      auto v0 = processedVertex[f.GetIndex(0)];
      auto v1 = processedVertex[f.GetIndex(1)];
      auto v2 = processedVertex[f.GetIndex(2)];

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
        fragmentsBuffer_per_thread[index].push_back(fragment);
      }
    }
  }

  // Merge fragments
  std::vector<std::vector<Fragment>> fragmentsBuffer(width_ * height_);
  for (const auto &fragmentsBuffer_per_thread : fragmentsBuffer_all_thread) {
    for (size_t i = 0; i < fragmentsBuffer_per_thread.size(); i++) {
      fragmentsBuffer[i].insert(fragmentsBuffer[i].end(),
                                fragmentsBuffer_per_thread[i].begin(),
                                fragmentsBuffer_per_thread[i].end());
    }
  }
/*  *  *  *  *  *  *  */

/* * * Fragment Shader * * */
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

    if (renderFragment) {
      auto color = shader_->FragmentShader(*renderFragment);
      buffer[i] = uint32_t(color);
    }
  }
  /*  *  *  *  *  *  *  */
}

}  // namespace simple_renderer
