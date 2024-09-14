
/**
 * @file simple_renderer.h
 * @brief simple_renderer 接口头文件
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

#ifndef SIMPLERENDER_SRC_INCLUDE_RENDERER_H_
#define SIMPLERENDER_SRC_INCLUDE_RENDERER_H_

#include <cstdint>
#include <functional>
#include <span>

#include "buffer.hpp"
#include "light.h"
#include "log_system.h"
#include "math.hpp"
#include "model.hpp"
#include "rasterizer.hpp"
#include "shader.hpp"

namespace simple_renderer {

class SimpleRenderer {
 public:
  typedef std::function<void(size_t, size_t, uint32_t, uint32_t *)>
      DrawPixelFunc;

  /**
   * 构造函数
   * @param width
   * @param height
   * @param buffer 要进行绘制的内存区域，大小为 width*height*sizeof(uint32_t)
   * @param
   */
  SimpleRenderer(size_t width, size_t height, uint32_t *buffer,
                 DrawPixelFunc draw_pixel_func);

  /// @name 默认构造/析构函数
  /// @{
  SimpleRenderer(const SimpleRenderer &_simplerenderer) = default;
  SimpleRenderer(SimpleRenderer &&_simplerenderer) = default;
  auto operator=(const SimpleRenderer &_simplerenderer) -> SimpleRenderer & =
                                                               default;
  auto operator=(SimpleRenderer &&_simplerenderer) -> SimpleRenderer & =
                                                          default;
  virtual ~SimpleRenderer() = default;
  /// @}

  bool render(const Model &model, const Shader &shader);

 private:
  const size_t height_;
  const size_t width_;
  uint32_t *buffer_;
  std::shared_ptr<float[]> depth_buffer_;
  DrawPixelFunc draw_pixel_func_;
  LogSystem log_system_;

  std::shared_ptr<Shader> shader_;
  std::shared_ptr<Rasterizer> rasterizer_;

  /**
   * 绘制模型
   * @param model 模型
   */
  void DrawModel(const Model &model);
};
}  // namespace simple_renderer

#endif /* SIMPLERENDER_SRC_INCLUDE_SIMPLE_RENDER_H_ */
