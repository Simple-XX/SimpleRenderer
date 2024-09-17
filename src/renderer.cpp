
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
  // init depth buffer
  depth_buffer_ = std::shared_ptr<float[]>(new float[width * height],
                                           std::default_delete<float[]>());
}

bool SimpleRenderer::Render(const Model &model, const Shader &shader,
                            uint32_t *buffer) {
  SPDLOG_INFO("render model: {}", model.GetModelPath());
  ClearDepthBuffer();
  shader_ = std::make_shared<Shader>(shader);
  DrawModel(model, buffer);
  return true;
}

void SimpleRenderer::ClearDepthBuffer() {
  std::fill(depth_buffer_.get(), depth_buffer_.get() + width_ * height_,
            std::numeric_limits<float>::infinity());
}

void SimpleRenderer::DrawModel(const Model &model, uint32_t *buffer) {
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
}  // namespace simple_renderer

}  // namespace simple_renderer
