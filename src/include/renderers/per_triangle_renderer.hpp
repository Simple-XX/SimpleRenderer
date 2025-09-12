#ifndef SIMPLERENDER_SRC_INCLUDE_RENDERERS_PER_TRIANGLE_RENDERER_HPP_
#define SIMPLERENDER_SRC_INCLUDE_RENDERERS_PER_TRIANGLE_RENDERER_HPP_

#include "renderers/renderer_base.hpp"

namespace simple_renderer {

/**
 * @brief 逐三角形渲染器（Triangle‑Major）
 *
 * 特点：
 * - AoS 顶点路径；
 * - 每线程本地 framebuffer（depth/color）合并；
 * - 背面剔除在屏幕空间完成；
 * - 接近“传统”栈式前向渲染教学实现。
 */
class PerTriangleRenderer final : public RendererBase {
 public:
  using RendererBase::RendererBase;
  /**
   * @copydoc RendererBase::Render
   */
  bool Render(const Model& model, const Shader& shader, uint32_t* out_color) override;
};

}  // namespace simple_renderer

#endif  // SIMPLERENDER_SRC_INCLUDE_RENDERERS_PER_TRIANGLE_RENDERER_HPP_
