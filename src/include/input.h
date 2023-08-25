
/**
 * @file input.h
 * @brief 输入处理
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

#ifndef SIMPLERENDER_INPUT_H
#define SIMPLERENDER_INPUT_H

#include <SDL_events.h>
#include <memory>

#include <SDL.h>

#include "scene.h"

/**
 * 输入处理
 */
class input_t {
public:
  /// @name 默认构造/析构函数
  /// @{
  input_t() = default;
  input_t(const input_t &_input) = default;
  input_t(input_t &&_input) = default;
  auto operator=(const input_t &_input) -> input_t & = default;
  auto operator=(input_t &&_input) -> input_t & = default;
  virtual ~input_t() = default;
  /// @}

  /**
   * 处理输入, 如果程序退出返回 false
   * @param _scene 要应用到的场景
   * @param _delta_time 时间变化
   * @return 为 false 时退出
   */
  auto process(scene_t &_scene, uint32_t _delta_time) -> bool;

private:
  /// SDL 事件
  SDL_Event event = SDL_Event();

  /**
   * a 键
   * @param _scene 要应用到的场景
   * @param _delta_time 时间变化
   */
  virtual void key_a(scene_t &_scene, uint32_t _delta_time) const;

  /**
   * d 键
   * @param _scene 要应用到的场景
   * @param _delta_time 时间变化
   */
  virtual void key_d(scene_t &_scene, uint32_t _delta_time) const;

  /**
   * w 键
   * @param _scene 要应用到的场景
   * @param _delta_time 时间变化
   */
  virtual void key_w(scene_t &_scene, uint32_t _delta_time) const;

  /**
   * s 键
   * @param _scene 要应用到的场景
   * @param _delta_time 时间变化
   */
  virtual void key_s(scene_t &_scene, uint32_t _delta_time) const;

  /**
   * z 键
   * @param _scene 要应用到的场景
   * @param _delta_time 时间变化
   */
  virtual void key_z(scene_t &_scene, uint32_t _delta_time) const;

  /**
   * r 键
   * @param _scene 要应用到的场景
   * @param _delta_time 时间变化
   */
  virtual void key_r(scene_t &_scene, uint32_t _delta_time) const;

  /**
   * q 键
   * @param _scene 要应用到的场景
   * @param _delta_time 时间变化
   */
  virtual void key_q(scene_t &_scene, uint32_t _delta_time) const;

  /**
   * e 键
   * @param _scene 要应用到的场景
   * @param _delta_time 时间变化
   */
  virtual void key_e(scene_t &_scene, uint32_t _delta_time) const;

  /**
   * space 键
   * @param _scene 要应用到的场景
   * @param _delta_time 时间变化
   */
  virtual void key_space(scene_t &_scene, uint32_t _delta_time) const;

  /**
   * left_ctrl 键
   * @param _scene 要应用到的场景
   * @param _delta_time 时间变化
   */
  virtual void key_left_ctrl(scene_t &_scene, uint32_t _delta_time) const;

  /**
   * left_shift 键
   * @param _scene 要应用到的场景
   * @param _delta_time 时间变化
   */
  virtual void key_left_shift(scene_t &_scene, uint32_t _delta_time) const;

  /**
   * 鼠标移动
   * @param _scene 要应用到的场景
   * @param _x 横坐标变化
   * @param _y 纵坐标变化
   * @param _delta_time 时间变化
   */
  virtual void mouse_motion(scene_t &_scene, int32_t _x, int32_t _y,
                            uint32_t _delta_time) const;

  /**
   * 处理输入
   * @param _scene 要应用到的场景
   * @param _delta_time 时间变化
   * @return 为 false 时退出
   */
  virtual auto handle(scene_t &_scene, uint32_t _delta_time) const -> bool;
};

#endif /* SIMPLERENDER_INPUT_H */
