
/**
 * @file camera.cpp
 * @brief 相机抽象
 * @author Zone.N (Zone.Niuzh@hotmail.com)
 * @version 1.0
 * @date 2022-09-19
 * @copyright MIT LICENSE
 * https://github.com/Simple-XX/SimpleRenderer
 * @par change log:
 * <table>
 * <tr><th>Date<th>Author<th>Description
 * <tr><td>2022-09-19<td>Zone.N<td>创建文件
 * </table>
 */

#include "camera.h"

camera_base_t::camera_base_t(const vector3f_t &_pos, const vector3f_t &_target,
                             float _aspect)
    : pos(_pos), target(_target), aspect(_aspect) {}

void camera_base_t::set_default() {
  pos = DEFAULT_POS;
  target = DEFAULT_TARGET;

  up = DEFAULT_UP;
  front = DEFAULT_FRONT;
  right = DEFAULT_RIGHT;

  aspect = DEFAULT_ASPECT;
  speed = DEFAULT_SPEED;
}

void camera_base_t::move(const move_to_t &_to, uint32_t _delta_time) {
  switch (_to) {
  case RIGHT: {
    pos += right * (speed * static_cast<decltype(speed)>(_delta_time));
    break;
  }
  case LEFT: {
    pos -= right * (speed * static_cast<decltype(speed)>(_delta_time));
    break;
  }
  case UP: {
    pos += up * (speed * static_cast<decltype(speed)>(_delta_time));
    break;
  }
  case DOWN: {
    pos -= up * (speed * static_cast<decltype(speed)>(_delta_time));
    break;
  }
  case FORWARD: {
    pos += front * (speed * static_cast<decltype(speed)>(_delta_time));
    break;
  }
  case BACKWARD: {
    pos -= front * (speed * static_cast<decltype(speed)>(_delta_time));
    break;
  }
  }
}

void camera_base_t::update_target(const move_to_t &_to,
                                  const uint32_t _delta_time) {
  switch (_to) {
  case RIGHT: {
    pos += front * (speed * static_cast<decltype(speed)>(_delta_time));
    break;
  }
  case LEFT: {
    pos -= front * (speed * static_cast<decltype(speed)>(_delta_time));
    break;
  }
  case UP: {
    pos += right * (speed * static_cast<decltype(speed)>(_delta_time));
    break;
  }
  case DOWN: {
    pos -= right * (speed * static_cast<decltype(speed)>(_delta_time));
    break;
  }
  case FORWARD: {
    pos += up * (speed * static_cast<decltype(speed)>(_delta_time));
    break;
  }
  case BACKWARD: {
    pos -= up * (speed * static_cast<decltype(speed)>(_delta_time));
    break;
  }
  }

  return;
}

void camera_base_t::update_up(const move_to_t &_to,
                              const uint32_t _delta_time) {
  switch (_to) {
  case RIGHT: {
    up += right * (speed * static_cast<decltype(speed)>(_delta_time));
    break;
  }
  case LEFT: {
    up -= right * (speed * static_cast<decltype(speed)>(_delta_time));
    break;
  }
  case UP: {
    up += up * (speed * static_cast<decltype(speed)>(_delta_time));
    break;
  }
  case DOWN: {
    up -= up * (speed * static_cast<decltype(speed)>(_delta_time));
    break;
  }
  case FORWARD: {
    up += front * (speed * static_cast<decltype(speed)>(_delta_time));
    break;
  }
  case BACKWARD: {
    up -= front * (speed * static_cast<decltype(speed)>(_delta_time));
    break;
  }
  }

  return;
}

const vector3f_t camera_base_t::DEFAULT_POS = vector3f_t(0.0, 0.0, 0.0);
const vector3f_t camera_base_t::DEFAULT_TARGET = vector3f_t(0, 0, 0);
const vector3f_t camera_base_t::DEFAULT_UP = vector3f_t(0, -1, 0);
const vector3f_t camera_base_t::DEFAULT_FRONT = vector3f_t(0, 0, -1);
const vector3f_t camera_base_t::DEFAULT_RIGHT = vector3f_t(-1, 0, 0);

/// @todo 有问题
matrix4f_t camera_base_t::look_at() const {
  //  auto z = (pos - target).normalized();
  //  auto x = (up.cross(z)).normalized();
  //  auto y = (z.cross(x)).normalized();
  //
  //  auto mat = matrix4f_t();
  //  mat << x.x(), x.y(), x.z(), -x * pos, y.x(), y.y(), y.z(), -y * pos,
  //  z.x(),
  //      z.y(), z.z(), -z * pos, 0, 0, 0, 1;
  //  return matrix4f_t(mat);
  return matrix4f_t();
}
