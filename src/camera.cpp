
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
    pos    = default_pos;
    target = default_target;
    up     = default_up;
    aspect = default_aspect;
    speed  = default_speed;
    return;
}

camera_base_t::camera_base_t(const vector4f_t& _pos, const vector4f_t& _target,
                             const float _aspect)
    : pos(_pos), target(_target), aspect(_aspect) {
    speed = default_speed;
    return;
}

camera_base_t::~camera_base_t(void) {
    return;
}

camera_base_t& camera_base_t::operator=(const camera_base_t& _camera) {
    pos    = _camera.pos;
    target = _camera.target;
    up     = _camera.up;
    aspect = _camera.aspect;
    speed  = _camera.speed;
    return *this;
}

// 视图变换
// 将相机移动到原点，方向指向 -z，即屏幕里，up 为 -y，即屏幕向上
const matrix4f_t camera_base_t::look_at(void) {
    auto  z          = (target - pos).normalize();
    auto  x          = (up ^ z).normalize();
    auto  y          = (z ^ x).normalize();

    float Minv[4][4] = {
        {x.x, x.y,  x.z, 0},
        {y.x, y.y,  y.z, 0},
        {z.x, z.y, -z.z, 0},
        {  0,   0,    0, 1}
    };
    auto  minv     = matrix4_t(Minv);
    float Tr[4][4] = {
        {1, 0, 0, -pos.x},
        {0, 1, 0, -pos.y},
        {0, 0, 1, -pos.z},
        {0, 0, 0,      1}
    };

    return minv * matrix4f_t(Tr);
}

void camera_base_t::set_default(void) {
    pos    = default_pos;
    target = default_target;
    up     = default_up;
    aspect = default_aspect;
    speed  = default_speed;
    return;
}

void camera_base_t::update_pos_x(const bool _is_positive) {
    auto delta = _is_positive ? speed : -speed;
    pos.x      += delta;
    return;
}

void camera_base_t::update_pos_y(const bool _is_positive) {
    auto delta = _is_positive ? speed : -speed;
    pos.y      += delta;
    return;
}

void camera_base_t::update_pos_z(const bool _is_positive) {
    auto delta = _is_positive ? speed : -speed;
    pos.z      += delta;
    return;
}

void camera_base_t::update_up_x(const bool _is_positive) {
    auto delta = _is_positive ? speed : -speed;
    up.x       += delta;
    return;
}

void camera_base_t::update_up_y(const bool _is_positive) {
    auto delta = _is_positive ? speed : -speed;
    up.y       += delta;
    return;
}

void camera_base_t::update_target(const int32_t _x, const int32_t _y) {
    target.x += _x * speed;
    target.y += _y * speed;
    return;
}

surround_camera_t::surround_camera_t(void) : camera_base_t() {
    pos    = default_pos;
    target = default_target;
    up     = default_up;
    aspect = default_aspect;
    speed  = default_speed;
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

surround_camera_t&
surround_camera_t::operator=(const surround_camera_t& _camera) {
    pos    = _camera.pos;
    target = _camera.target;
    up     = _camera.up;
    aspect = _camera.aspect;
    speed  = _camera.speed;
    return *this;
}

void surround_camera_t::set_default(void) {
    pos    = default_pos;
    target = default_target;
    up     = default_up;
    aspect = default_aspect;
    speed  = default_speed;
    return;
}

void surround_camera_t::update_pos_x(const bool _is_positive) {
    auto delta = _is_positive ? speed : -speed;
    pos.x      += delta;
    return;
}

void surround_camera_t::update_pos_y(const bool _is_positive) {
    auto delta = _is_positive ? speed : -speed;
    pos.y      += delta;
    return;
}

void surround_camera_t::update_pos_z(const bool _is_positive) {
    auto delta = _is_positive ? speed : -speed;
    pos.z      += delta;
    return;
}

void surround_camera_t::update_up_x(const bool _is_positive) {
    auto delta = _is_positive ? speed : -speed;
    up.x       += delta;
    return;
}

void surround_camera_t::update_up_y(const bool _is_positive) {
    auto delta = _is_positive ? speed : -speed;
    up.y       += delta;
    return;
}

void surround_camera_t::update_target(const int32_t _x, const int32_t _y) {
    target.x += _x * speed;
    target.y += _y * speed;
    return;
}
