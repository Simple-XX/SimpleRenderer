
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

#include "simple_renderer.h"

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

SimpleRenderer::SimpleRenderer(size_t width, size_t height, uint32_t *buffer,
                               DrawPixelFunc draw_pixel_func)
    : height_(height),
      width_(width),
      buffer_(buffer),
      draw_pixel_func_(draw_pixel_func),
      log_system_(LogSystem(kLogFilePath, kLogFileMaxSize, kLogFileMaxCount)) {
  rasterizer_ = std::make_shared<Rasterizer>(width, height);
  // init depth buffer
  depth_buffer_ = std::shared_ptr<float[]>(new float[width * height],
                                           std::default_delete<float[]>());
  std::fill(depth_buffer_.get(), depth_buffer_.get() + width * height,
            std::numeric_limits<float>::infinity());
}

bool SimpleRenderer::render(const Model &model, const Shader &shader) {
  SPDLOG_INFO("render model: {}", model.modelPath());
  shader_ = std::make_shared<Shader>(shader);
  DrawModel(model);
  return true;
}

void SimpleRenderer::DrawModel(const Model &model) {
  SPDLOG_INFO("draw {}", model.modelPath());
  std::vector<Vertex> processedVertex;

  std::vector<std::vector<Vertex>> processed_vertices_per_thread(kNProc);
#pragma omp parallel num_threads(kNProc) default(none) \
    shared(shader_, processed_vertices_per_thread) firstprivate(model)
  {
    int thread_id = omp_get_thread_num();
    std::vector<Vertex> &local_vertices =
        processed_vertices_per_thread[thread_id];

#pragma omp for
    for (const auto &v : model.vertices()) {
      /* * * Vertex Shader * *  */
      auto vertex = shader_->VertexShader(v);
      local_vertices.push_back(vertex);
    }
  }

  for (const auto &local_vertices : processed_vertices_per_thread) {
    processedVertex.insert(processedVertex.end(), local_vertices.begin(),
                           local_vertices.end());
  }

  for (const auto &f : model.faces()) {
    auto v0 = processedVertex[f.index(0)];
    auto v1 = processedVertex[f.index(1)];
    auto v2 = processedVertex[f.index(2)];
    /* * * Rasterization * * */
    auto fragments = rasterizer_->rasterize(v0, v1, v2);
    // material
    shader_->SetUniform("material", f.material());
    for (const auto &fragment : fragments) {
      size_t x = fragment.screen_coord[0];
      size_t y = fragment.screen_coord[1];
      if (fragment.depth < depth_buffer_[x + y * width_]) {
        depth_buffer_[x + y * width_] = fragment.depth;

        /* * * Fragment Shader * * */
        auto color = shader_->FragmentShader(fragment);
        draw_pixel_func_(x, y, color, buffer_);
      }
    }
  }
}

}  // namespace simple_renderer
