
/**
 * @file render.h
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

#ifndef SIMPLERENDER_RENDER_H
#define SIMPLERENDER_RENDER_H

#include <cstdint>
#include <memory>

#include "config.h"
#include "default_shader.h"
#include "display.h"
#include "framebuffer.h"
#include "input.h"
#include "scene.h"
#include "shader.h"

/**
 * 渲染抽象
 */
class render_t {
public:
  /**
   * 构造函数
   * @param _scene 场景
   * @param _display 显示
   * @param _framebuffer 缓冲区
   * @param _input 输入
   */
  explicit render_t(const std::shared_ptr<scene_t> &_scene,
                    const std::shared_ptr<display_t> &_display,
                    const std::shared_ptr<input_t> &_input);

  /// @name 默认构造/析构函数
  /// @{
  render_t() = default;
  render_t(const render_t &_render) = default;
  render_t(render_t &&_render) = default;
  auto operator=(const render_t &_render) -> render_t & = default;
  auto operator=(render_t &&_render) -> render_t & = default;
  ~render_t() = default;
  /// @}

  /**
   * 渲染循环
   */
  void loop();

private:
  /// 场景
  std::shared_ptr<scene_t> scene;
  /// 显示
  std::shared_ptr<display_t> display;
  /// 输入
  std::shared_ptr<input_t> input;

  /// 缓冲
  std::vector<std::shared_ptr<framebuffer_t>> framebuffer;

  /// 着色器
  default_shader_t shader;

  /// 是否还在运行，为 false 时退出
  std::atomic_bool is_running = true;
};

#endif /* SIMPLERENDER_RENDER_H */
