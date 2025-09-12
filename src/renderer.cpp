#include "renderer.h"

#include <string>

#include "config.h"
#include "renderers/per_triangle_renderer.hpp"
#include "renderers/tile_based_renderer.hpp"
#include "renderers/deferred_renderer.hpp"

namespace simple_renderer {

std::string RenderingModeToString(RenderingMode mode) {
  switch(mode) {
    case RenderingMode::PER_TRIANGLE: return "PER_TRIANGLE";
    case RenderingMode::TILE_BASED:  return "TILE_BASED";
    case RenderingMode::DEFERRED:    return "DEFERRED";
  }
  return "PER_TRIANGLE";
}

SimpleRenderer::SimpleRenderer(size_t width, size_t height)
    : height_(height),
      width_(width),
      log_system_(LogSystem(kLogFilePath, kLogFileMaxSize, kLogFileMaxCount)),
      current_mode_(RenderingMode::TILE_BASED) {
  tbr_early_z_ = true;
  tbr_tile_size_ = 64;
  EnsureRenderer();
}

bool SimpleRenderer::DrawModel(const Model &model, const Shader &shader, uint32_t *buffer) {
  EnsureRenderer(); // 确保渲染器实例存在
  SPDLOG_INFO("draw model: {}", model.GetModelPath());
  return renderer_->Render(model, shader, buffer);
}

void SimpleRenderer::SetRenderingMode(RenderingMode mode) {
  current_mode_ = mode;
  SPDLOG_INFO("rendering mode set to: {}", RenderingModeToString(mode));
  renderer_.reset();
  EnsureRenderer();
}

RenderingMode SimpleRenderer::GetRenderingMode() const { return current_mode_; }

void SimpleRenderer::SetEarlyZEnabled(bool enabled) {
  tbr_early_z_ = enabled;
  if (current_mode_ == RenderingMode::TILE_BASED) {
    renderer_.reset();
    EnsureRenderer();
  }
}

void SimpleRenderer::SetTileSize(size_t tile_size) {
  tbr_tile_size_ = tile_size;
  if (current_mode_ == RenderingMode::TILE_BASED) {
    renderer_.reset();
    EnsureRenderer();
  }
}

void SimpleRenderer::EnsureRenderer() {
  if (renderer_) return;
  switch (current_mode_) { // 延迟初始化，根据模式创建相应实例
    case RenderingMode::PER_TRIANGLE: {
      auto r = std::make_unique<PerTriangleRenderer>(width_, height_);
      renderer_ = std::move(r);
      break;
    }
    case RenderingMode::TILE_BASED: {
      auto r = std::make_unique<TileBasedRenderer>(width_, height_, tbr_early_z_, tbr_tile_size_);
      renderer_ = std::move(r);
      break;
    }
    case RenderingMode::DEFERRED: {
      auto r = std::make_unique<DeferredRenderer>(width_, height_);
      renderer_ = std::move(r);
      break;
    }
  }
}

}  // namespace simple_renderer
