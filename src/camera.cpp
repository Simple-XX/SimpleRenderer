
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

camera_base_t::camera_base_t(void) {
    pos    = DEFAULT_POS;
    target = DEFAULT_TARGET;

    up     = DEFAULT_UP;
    front  = DEFAULT_FRONT;
    /// @todo 这里的右方向需要确认正确性
    right  = DEFAULT_RIGHT;

    aspect = DEFAULT_ASPECT;
    speed  = DEFAULT_SPEED;

    return;
}

camera_base_t::camera_base_t(const camera_base_t& _camera) {
    pos    = _camera.pos;
    target = _camera.target;

    up     = _camera.up;
    front  = _camera.front;
    right  = _camera.right;

    aspect = _camera.aspect;
    speed  = _camera.speed;

    return;
}

camera_base_t::camera_base_t(const vector4f_t& _pos, const vector4f_t& _target,
                             float _aspect)
    : pos(_pos), target(_target), aspect(_aspect) {
    up    = DEFAULT_UP;
    front = DEFAULT_FRONT;
    /// @todo 这里的右方向需要确认正确性
    right = DEFAULT_RIGHT;

    speed = DEFAULT_SPEED;
    return;
}

camera_base_t::~camera_base_t(void) {
    return;
}

camera_base_t& camera_base_t::operator=(const camera_base_t& _camera) {
    pos    = _camera.pos;
    target = _camera.target;

    up     = _camera.up;
    front  = _camera.front;
    right  = _camera.right;

    aspect = _camera.aspect;
    speed  = _camera.speed;

    return *this;
}

void camera_base_t::set_default(void) {
    pos    = DEFAULT_POS;
    target = DEFAULT_TARGET;

    up     = DEFAULT_UP;
    front  = DEFAULT_FRONT;
    /// @todo 这里的右方向需要确认正确性
    right  = DEFAULT_RIGHT;

    aspect = DEFAULT_ASPECT;
    speed  = DEFAULT_SPEED;
    return;
}

void camera_base_t::move(const move_to_t& _to, uint32_t _delta_time) {
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

    return;
}

void camera_base_t::update_target(const move_to_t& _to,
                                  const uint32_t   _delta_time) {
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

void camera_base_t::update_up(const move_to_t& _to,
                              const uint32_t   _delta_time) {
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

/// @todo 有问题
matrix4f_t camera_base_t::look_at(void) const {
    auto  z         = (pos - target).normalize();
    auto  x         = (up ^ z).normalize();
    auto  y         = (z ^ x).normalize();

    float arr[4][4] = {
        {x.x, x.y, x.z, -x * pos},
        {y.x, y.y, y.z, -y * pos},
        {z.x, z.y, z.z, -z * pos},
        {  0,   0,   0,        1}
    };

    return matrix4f_t(arr);
}

surround_camera_t::surround_camera_t(void) : camera_base_t() {
    return;
}

surround_camera_t::surround_camera_t(const surround_camera_t& _camera)
    : camera_base_t(_camera) {
    return;
}

surround_camera_t::surround_camera_t(const vector4f_t& _pos,
                                     const vector4f_t& _target, float _aspect)
    : camera_base_t(_pos, _target, _aspect) {
    return;
}

surround_camera_t::~surround_camera_t(void) {
    return;
}
