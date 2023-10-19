
/**
 * @file input.cpp
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

#include "input.h"

void input_t::key_a(scene_t &_scene, uint32_t _delta_time) const {
  _scene.get_current_camera().move(camera_base_t::LEFT, _delta_time);
}

void input_t::key_d(scene_t &_scene, uint32_t _delta_time) const {
  _scene.get_current_camera().move(camera_base_t::RIGHT, _delta_time);
}

void input_t::key_w(scene_t &_scene, uint32_t _delta_time) const {
  _scene.get_current_camera().move(camera_base_t::FORWARD, _delta_time);
}

void input_t::key_s(scene_t &_scene, uint32_t _delta_time) const {
  _scene.get_current_camera().move(camera_base_t::BACKWARD, _delta_time);
}

void input_t::key_z(scene_t &_scene, uint32_t _delta_time) const {
  _scene.get_current_camera().move(camera_base_t::DOWN, _delta_time);
}

void input_t::key_space(scene_t &_scene, uint32_t _delta_time) const {
  _scene.get_current_camera().move(camera_base_t::UP, _delta_time);
}

void input_t::key_r(scene_t &_scene,
                    const uint32_t _delta_time [[maybe_unused]]) const {
  _scene.get_current_camera().set_default();
}

void input_t::key_q(scene_t &_scene, uint32_t _delta_time) const {
  _scene.get_current_camera().update_up(camera_base_t::UP, _delta_time);
}

void input_t::key_e(scene_t &_scene, uint32_t _delta_time) const {
  _scene.get_current_camera().update_up(camera_base_t::DOWN, _delta_time);
}

void input_t::key_left_ctrl(scene_t &_scene, uint32_t _delta_time) const {
  (void)_scene;
  (void)_delta_time;
}

void input_t::key_left_shift(scene_t &_scene, uint32_t _delta_time) const {
  (void)_scene;
  (void)_delta_time;
}

void input_t::mouse_motion(scene_t &_scene, int32_t _x, int32_t _y,
                           uint32_t _delta_time) const {
  (void)_scene;
  (void)_x;
  (void)_y;
  (void)_delta_time;
  // _scene.get_current_camera().update_target(_x, _y);
}

auto input_t::handle(scene_t &_scene, uint32_t _delta_time) const -> bool {
  auto res = true;
  switch (curr_event.type) {
  // 键盘事件
  case SDL_KEYDOWN: {
    switch (curr_event.key.keysym.sym) {
    case SDLK_ESCAPE: {
      // 如果是 esc 键则返回 false，程序退出
      res = false;
      break;
    }
    case SDLK_a: {
      key_a(_scene, _delta_time);
      break;
    }
    case SDLK_d: {
      key_d(_scene, _delta_time);
      break;
    }
    case SDLK_SPACE: {
      key_space(_scene, _delta_time);
      break;
    }
    case SDLK_z: {
      key_z(_scene, _delta_time);
      break;
    }
    case SDLK_r: {
      key_r(_scene, _delta_time);
      break;
    }
    case SDLK_LCTRL: {
      key_left_ctrl(_scene, _delta_time);
      break;
    }
    case SDLK_w: {
      key_w(_scene, _delta_time);
      break;
    }
    case SDLK_s: {
      key_s(_scene, _delta_time);
      break;
    }
    case SDLK_q: {
      key_q(_scene, _delta_time);
      break;
    }
    case SDLK_e: {
      key_e(_scene, _delta_time);
      break;
    }
    case SDLK_LSHIFT: {
      /// @bug 按一次触发两次
      SPDLOG_LOGGER_INFO(SRLOG, "key {} down!",
                         SDL_GetKeyName(curr_event.key.keysym.sym));
      key_left_shift(_scene, _delta_time);
      break;
    }
    default: {
      // 输出按键名
      SPDLOG_LOGGER_INFO(SRLOG, "key {} down!",
                         SDL_GetKeyName(curr_event.key.keysym.sym));
      break;
    }
    }
    break;
  }
  // 鼠标移动
  case SDL_MOUSEMOTION: {
    mouse_motion(_scene, curr_event.motion.xrel, curr_event.motion.yrel,
                 _delta_time);
    break;
  }
    /// @todo
    // 鼠标点击
    // case SDL_MOUSEBUTTONDOWN: {
    // break;
    // }
    // 鼠标滚轮
    // case SDL_MOUSEWHEEL: {
    //     break;
    // }
  }
  return res;
}

auto input_t::process() -> bool {
  SDL_Event event = SDL_Event();
  if (SDL_PollEvent(&event) != 0) {
    event_q.push(event);
    if (curr_event.type == SDL_QUIT) {
      return false;
    }
  }
  return true;
}

auto input_t::process(scene_t &_scene, uint32_t _delta_time) -> bool {
  for (size_t i = 0; i < event_q.size(); i++) {
    curr_event = event_q.front();
    event_q.pop();
    auto handle_ret = handle(_scene, _delta_time);
    if (handle_ret == false) {
      return false;
    }
  }
  return true;
}
