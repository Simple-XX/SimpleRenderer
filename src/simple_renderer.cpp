
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

#include <array>
#include <cstdint>
#include <limits>
#include <span>
#include <string_view>
#include <vector>

#include "config.h"
#include "default_shader.h"
#include "light.h"
#include "log_system.h"
#include "model.hpp"
#include "shader_base.h"

namespace simple_renderer {

SimpleRenderer::SimpleRenderer(size_t width, size_t height, uint32_t *buffer,
                               DrawPixelFunc draw_pixel_func)
    : height_(height),
      width_(width),
      buffer_(buffer),
      draw_pixel_func_(draw_pixel_func),
      log_system_(LogSystem(kLogFilePath, kLogFileMaxSize, kLogFileMaxCount)) {
  rasterizer_ = std::make_shared<Rasterizer>(width, height);
}

bool SimpleRenderer::render(const Model &model, const Shader &shader) {
  SPDLOG_INFO("render model: {}", model.modelPath());
  shader_ = std::make_shared<Shader>(shader);
  auto light = Light();
  DrawModel(model, light);
  return true;
}

void SimpleRenderer::DrawModel(const Model &model, const Light &light) {
  SPDLOG_INFO("draw {}", model.modelPath());
  std::vector<Vertex> processedVertex;
#pragma omp parallel for num_threads(kNProc) default(none) shared(shader) \
    firstprivate(model, light)
  for (const auto &v : model.vertices()) {
    auto vertex = shader_->VertexShader(v);
    processedVertex.push_back(vertex);
  }
  for (const auto &f : model.faces()) {
    auto v0 = processedVertex[f.index(0)];
    auto v1 = processedVertex[f.index(1)];
    auto v2 = processedVertex[f.index(2)];
    auto fragments = rasterizer_->rasterize(v0, v1, v2);
    for (const auto &fragment : fragments) {
      auto color = shader_->FragmentShader(fragment);
      draw_pixel_func_(fragment.screen_coord[0], fragment.screen_coord[1],
                       color, buffer_);
    }
  }
}

}  // namespace simple_renderer
