
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

camera_t::camera_t(void) {
    pos    = vector3f_t();
    target = vector3f_t();
    aspect = 1;
    return;
}

camera_t::camera_t(const vector3f_t &_pos, const vector3f_t &_target,
                   const float _aspect)
    : pos(_pos), target(_target), aspect(_aspect) {
    return;
}

bool camera_t::operator==(const camera_t &_camera) const {
    return pos == _camera.pos && target == _camera.target &&
           aspect == _camera.aspect;
}

bool camera_t::operator!=(const camera_t &_camera) const {
    return pos != _camera.pos || target != _camera.target ||
           aspect != _camera.aspect;
}

const vector3f_t camera_t::operator*(const vector3f_t &_v) const {
    return vector3f_t();
}

const matrix4f_t camera_t::operator*(const matrix4f_t &_m) const {
    return matrix4f_t();
}

vector3f_t &camera_t::get_pos(void) {
    return pos;
}

vector3f_t &camera_t::get_target(void) {
    return target;
}

float camera_t::get_aspect(void) const {
    return aspect;
}
