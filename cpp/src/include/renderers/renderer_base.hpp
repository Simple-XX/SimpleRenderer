// Renderer base and options
#ifndef SIMPLERENDER_SRC_INCLUDE_RENDERERS_RENDERER_BASE_HPP_
#define SIMPLERENDER_SRC_INCLUDE_RENDERERS_RENDERER_BASE_HPP_

#include <cstdint>
#include <memory>

#include "rasterizer.hpp"
#include "vertex.hpp"
#include "model.hpp"
#include "shader.hpp"

namespace simple_renderer {


/**
 * @brief 渲染器抽象基类
 *
 * 约定：
 * - Render 负责完成完整的渲染过程（顶点变换 + 光栅化 + 着色 + 写入输出缓冲）。
 * - 子类选择不同的“组织单元”：（按照并行组织单元）逐三角形、按 tile、或延迟管线。
 * - 公共的透视除法与视口变换在此提供，子类按需复用。
 */
class RendererBase {
 public:
  RendererBase(size_t width, size_t height)
      : width_(width), height_(height), rasterizer_(std::make_shared<Rasterizer>(width, height)) {}
  virtual ~RendererBase() = default;

  RendererBase(const RendererBase&) = delete;
  RendererBase& operator=(const RendererBase&) = delete;

  /**
   * @brief 执行一次渲染
   * @param model 模型数据
   * @param shader 着色器（包含材质/光照/矩阵等 uniform）
   * @param out_color 输出颜色缓冲（大小为 width*height）
   * @return 是否渲染成功
   */
  virtual bool Render(const Model& model, const Shader& shader, uint32_t* out_color) = 0;

 protected:
  /**
   * @brief 透视除法：裁剪空间 -> NDC
   * @param vertex 裁剪空间顶点
   * @return NDC 顶点（保留 1/w 以供透视校正）
   */
  Vertex PerspectiveDivision(const Vertex& vertex);
  /**
   * @brief 视口变换：NDC -> 屏幕坐标
   * @param vertex NDC 顶点
   * @return 屏幕空间顶点
   */
  Vertex ViewportTransformation(const Vertex& vertex);

 protected:
  size_t width_;
  size_t height_;
  std::shared_ptr<Rasterizer> rasterizer_;

  static constexpr float kMinWValue = 1e-6f;
};

}  // namespace simple_renderer

#endif  // SIMPLERENDER_SRC_INCLUDE_RENDERERS_RENDERER_BASE_HPP_
