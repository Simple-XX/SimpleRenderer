
/**
 * @file render.hpp
 * @brief 渲染抽象
 * @author Zone.N (Zone.Niuzh@hotmail.com)
 * @version 1.0
 * @date 2022-12-15
 * @copyright MIT LICENSE
 * https://github.com/Simple-XX/SimpleRenderer
 * @par change log:
 * <table>
 * <tr><th>Date<th>Author<th>Description
 * <tr><td>2022-12-15<td>Zone.N<td>创建文件
 * </table>
 */

#ifndef SIMPLERENDER_RENDER_HPP
#define SIMPLERENDER_RENDER_HPP

#include <cstdint>
#include <future>
#include <memory>

#include "config.h"
#include "default_shader.h"
#include "framebuffer.h"
#include "light.h"
#include "model.hpp"
#include "scene.h"
#include "shader.h"
#include "status.h"

namespace SimpleRenderer {

/**
 * 渲染抽象
 */
template <uint64_t _W, uint64_t _H> class render_t {
public:
  typedef std::function<void(uint64_t _x, uint64_t _y, const color_t &_color,
                             std::array<color_t, _W * _H> &_buffer)>
      pixel_func_t;

  enum draw_type_t {
    /// 绘制线框
    LINE,
    /// 填充颜色
    DRAW,
  };

  /**
   * 构造函数
   * @param _scene 场景
   * @param _framebuffer 缓冲区
   */
  explicit render_t(const scene_t &_scene,
                    std::array<color_t, _W * _H> &_output_buffer,
                    pixel_func_t _pixel_func) {
    /// @todo 几何层
    /// @todo 光栅化
    /// @todo 像素处理
  }

  /// @name 默认构造/析构函数
  /// @{
  render_t() = delete;
  render_t(const render_t &_render) = default;
  render_t(render_t &&_render) = default;
  auto operator=(const render_t &_render) -> render_t & = default;
  auto operator=(render_t &&_render) -> render_t & = default;
  ~render_t() = default;
  /// @}

private:
  /// 渲染目标帧率
  uint64_t max_fps;
  /// 绘制方式
  draw_type_t draw_type;
  /// 输出缓冲区地址
  std::array<color_t, _W * _H> output_buffer;
  /// 绘制方式函数指针
  pixel_func_t pixel_func;

  /// MVP 矩阵
  matrix4f_t model_mat;
  matrix4f_t view_mat;
  matrix4f_t proj_mat;
  /// 着色器
  std::shared_ptr<shader_base_t> shader = std::make_shared<default_shader_t>();
};

} // namespace SimpleRenderer

#endif /* SIMPLERENDER_RENDER_HPP */
