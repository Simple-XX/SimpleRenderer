
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
    right  = (front ^ up).normalize();

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
                             const float _aspect)
    : pos(_pos), target(_target), aspect(_aspect) {
    up    = DEFAULT_UP;
    front = DEFAULT_FRONT;
    right = (front ^ up).normalize();

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
    right  = DEFAULT_RIGHT;

    aspect = DEFAULT_ASPECT;
    speed  = DEFAULT_SPEED;
    return;
}

void camera_base_t::update_pos(const to_t& _to, const uint32_t _delta_time) {
    switch (_to) {
        case ADD_X: {
            pos += front * (speed * _delta_time);
            break;
        }
        case SUB_X: {
            pos -= front * (speed * _delta_time);
            break;
        }
        case ADD_Y: {
            pos += right * (speed * _delta_time);
            break;
        }
        case SUB_Y: {
            pos -= right * (speed * _delta_time);
            break;
        }
        case ADD_Z: {
            pos += up * (speed * _delta_time);
            break;
        }
        case SUB_Z: {
            pos -= up * (speed * _delta_time);
            break;
        }
    }

    return;
}

void camera_base_t::update_target(const to_t& _to, const uint32_t _delta_time) {
    switch (_to) {
        case ADD_X: {
            pos += front * (speed * _delta_time);
            break;
        }
        case SUB_X: {
            pos -= front * (speed * _delta_time);
            break;
        }
        case ADD_Y: {
            pos += right * (speed * _delta_time);
            break;
        }
        case SUB_Y: {
            pos -= right * (speed * _delta_time);
            break;
        }
        case ADD_Z: {
            pos += up * (speed * _delta_time);
            break;
        }
        case SUB_Z: {
            pos -= up * (speed * _delta_time);
            break;
        }
    }

    return;
}

void camera_base_t::update_up(const to_t& _to, const uint32_t _delta_time) {
    switch (_to) {
        case ADD_X: {
            up += front * (speed * _delta_time);
            break;
        }
        case SUB_X: {
            up -= front * (speed * _delta_time);
            break;
        }
        case ADD_Y: {
            up += right * (speed * _delta_time);
            break;
        }
        case SUB_Y: {
            up -= right * (speed * _delta_time);
            break;
        }
        case ADD_Z: {
            up += up * (speed * _delta_time);
            break;
        }
        case SUB_Z: {
            up -= up * (speed * _delta_time);
            break;
        }
    }

    return;
}

const matrix4f_t camera_base_t::look_at(void) {
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

surround_camera_t::surround_camera_t(const vector4f_t& _pos,
                                     const vector4f_t& _target,
                                     const float       _aspect)
    : camera_base_t(_pos, _target, _aspect) {
    return;
}

surround_camera_t::~surround_camera_t(void) {
    return;
}
