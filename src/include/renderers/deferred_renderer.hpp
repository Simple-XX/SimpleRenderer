#ifndef SIMPLERENDER_SRC_INCLUDE_RENDERERS_DEFERRED_RENDERER_HPP_
#define SIMPLERENDER_SRC_INCLUDE_RENDERERS_DEFERRED_RENDERER_HPP_

#include "renderers/renderer_base.hpp"

namespace simple_renderer {

/**
 * @brief 延迟渲染器（Deferred）
 * 
 * 组织处理方式模拟 OpenGL 在 GPU上的工作原理，模仿 GPU管线。
 * 但相比于另外两个前向渲染实现，导致内存使用增加和渲染速度变慢。
 * 
 * 特点：
 * - AoS 顶点路径；
 * - 首先按像素收集所有片段并选择最近深度；
 * - 再对选择的片段执行片段着色（模拟经典 GPU 管线的一种教学实现）。
 * - 
 */
class DeferredRenderer final : public RendererBase {
 public:
  using RendererBase::RendererBase;
  /**
   * @copydoc RendererBase::Render
   */
  bool Render(const Model& model, const Shader& shader, uint32_t* out_color) override;
};

}  // namespace simple_renderer

#endif  // SIMPLERENDER_SRC_INCLUDE_RENDERERS_DEFERRED_RENDERER_HPP_
