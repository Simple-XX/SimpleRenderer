
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
#include <future>
#include <memory>

#include "config.h"
#include "default_shader.h"
#include "display.h"
#include "framebuffer.h"
#include "scene.h"
#include "shader.h"
#include "status.h"

namespace SimpleRenderer {

/**
 * 渲染抽象
 */
class render_t {
public:
  /**
   * 构造函数
   * @param _state 运行状态
   * @param _scene 场景
   * @param _input 输入
   * @param _framebuffer 缓冲区
   */
  explicit render_t(
      const std::shared_ptr<state_t> &_state,
      const std::shared_ptr<scene_t> &_scene,
      const std::vector<std::shared_ptr<framebuffer_t>> &_framebuffers);

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
   * 运行
   */
  auto run() -> std::future<state_t::status_t>;

private:
  /// 状态
  std::shared_ptr<state_t> state;
  /// 场景
  std::shared_ptr<scene_t> scene;
  /// 显示
  std::shared_ptr<display_t> display;
  /// 着色器
  std::shared_ptr<shader_base_t> shader = std::make_shared<default_shader_t>();
  /// 缓冲
  std::vector<std::shared_ptr<framebuffer_t>> framebuffers;

  /**
   * 渲染循环
   */
  auto loop() -> state_t::status_t;
};

} // namespace SimpleRenderer

#endif /* SIMPLERENDER_RENDER_H */
