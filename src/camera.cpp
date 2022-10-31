
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
    pos    = pos_default;
    target = target_default;
    up     = up_default;
    aspect = aspect_default;
    speed  = speed_default;
    return;
}

camera_t::camera_t(const vector4f_t& _pos, const vector4f_t& _target,
                   const float _aspect)
    : pos(_pos), target(_target), aspect(_aspect) {
    speed = speed_default;
    return;
}

camera_t& camera_t::operator=(const camera_t& _camera) {
    pos    = _camera.pos;
    target = _camera.target;
    up     = _camera.up;
    aspect = _camera.aspect;
    speed  = _camera.speed;
    return *this;
}

bool camera_t::operator==(const camera_t& _camera) const {
    return pos == _camera.pos && target == _camera.target
        && aspect == _camera.aspect;
}

bool camera_t::operator!=(const camera_t& _camera) const {
    return pos != _camera.pos || target != _camera.target
        || aspect != _camera.aspect;
}

const vector4f_t camera_t::operator*(const vector4f_t& _v) const {
    (void)_v;
    return vector4f_t();
}

const matrix4f_t camera_t::operator*(const matrix4f_t& _m) const {
    (void)_m;
    return matrix4f_t();
}

vector4f_t& camera_t::get_pos(void) {
    return pos;
}

vector4f_t& camera_t::get_target(void) {
    return target;
}

vector4f_t& camera_t::get_up(void) {
    return up;
}

float camera_t::get_aspect(void) {
    return aspect;
}

// 视图变换
// 将相机移动到原点，方向指向 -z，即屏幕里，up 为 -y，即屏幕向上

// Eye 相机位置
// Center 相机注视的点
// up 世界坐标向上方向

matrix4f_t
LookAt(const vector4f_t& Eye, const vector4f_t& Center, const vector4f_t& Up) {
    auto  z          = (Center - Eye).normalize();
    // auto  z          = (Eye - Center).normalize();
    auto  x          = (Up ^ z).normalize();
    auto  y          = (z ^ x).normalize();

    // float Matrix[4][4] = {
    //     {     x.x,      y.x,      z.x, 0},
    //     {     x.y,      y.y,      z.y, 0},
    //     {     x.z,      y.z,      z.z, 0},
    //     {-x * Eye, -y * Eye, -z * Eye, 1}
    // };
    // return matrix4f_t(Matrix);

    float Minv[4][4] = {
        {x.x, x.y, x.z, 0},
        {y.x, y.y, y.z, 0},
        {z.x, z.y, z.z, 0},
        {  0,   0,   0, 1}
    };
    float Tr[4][4] = {
        {1, 0, 0, -Eye.x},
        {0, 1, 0, -Eye.y},
        {0, 0, 1, -Eye.z},
        {0, 0, 0,      1}
    };

    float tmp[4][4] = {
        {x.x, x.y, x.z,   x.x * -Eye.x + x.y * -Eye.y + x.z * -Eye.z},
        {y.x, y.y, y.z, (y.x * -Eye.x + -Eye.y + y.z * -Eye.z) * y.y},
        {z.x, z.y, z.z, (z.x * -Eye.x + z.y * -Eye.y + -Eye.z) * z.z},
        {  0,   0,   0,                                            1},
    };

    // return matrix4f_t(tmp);
    return matrix4f_t(Minv) * matrix4f_t(Tr);

    float m2[4][4] = {
        {x.x, x.y, x.z, -x * Eye},
        {y.x, y.y, y.z, -y * Eye},
        {z.x, z.y, z.z, -z * Eye},
        {  0,   0,   0,        1}
    };
    return matrix4f_t(m2);
}

const matrix4f_t camera_t::look_at(void) {
    // float viewMatrix[4][4] = { { 0 } };
    //
    // auto  cameraPos        = vector4f_t(pos);
    // auto  cameraTarget     = vector4f_t(target);
    // auto  cameraDirection  = (cameraPos - cameraTarget).normalize();
    //
    // auto  cameraWorldUP    = vector4f_t(vector4f_t(0, -1, 0));
    // auto  cameraRight      = (cameraWorldUP ^ cameraDirection).normalize();
    // auto  cameraUp         = cameraDirection ^ cameraRight;
    //
    // viewMatrix[0][0]       = cameraRight.x;
    // viewMatrix[0][1]       = cameraRight.y;
    // viewMatrix[0][2]       = cameraRight.z;
    // viewMatrix[0][3]       = -cameraRight.x * cameraPos.x
    //                  - cameraRight.y * cameraPos.y
    //                  - cameraRight.z * cameraPos.z;
    //
    // viewMatrix[1][0] = cameraUp.x;
    // viewMatrix[1][1] = cameraUp.y;
    // viewMatrix[1][2] = cameraUp.z;
    // viewMatrix[1][3] = -cameraUp.x * cameraPos.x - cameraUp.y * cameraPos.y
    //                  - cameraUp.z * cameraPos.z;
    //
    // viewMatrix[2][0] = cameraDirection.x;
    // viewMatrix[2][1] = cameraDirection.y;
    // viewMatrix[2][2] = cameraDirection.z;
    // viewMatrix[2][3] = -cameraDirection.x * cameraPos.x
    //                  - cameraDirection.y * cameraPos.y
    //                  - cameraDirection.z * cameraPos.z;
    //
    // viewMatrix[3][0] = 0;
    // viewMatrix[3][1] = 0;
    // viewMatrix[3][2] = 0;
    // viewMatrix[3][3] = 1;
    //
    // return matrix4f_t(viewMatrix);

    // auto   z_axis           = (pos - target).normalize();
    // auto   x_axis           = (up ^ z_axis).normalize();
    // auto   y_axis           = (z_axis ^ x_axis).normalize();

    // auto  z_axis           = (pos - target).normalize();
    // auto  x_axis           = (up ^ z_axis).normalize();
    // auto  y_axis           = (z_axis ^ x_axis);
    //
    // float viewMatrix[4][4] = {
    //     {x_axis.x, x_axis.y, x_axis.z, -x_axis * pos},
    //     {y_axis.x, y_axis.y, y_axis.z, -y_axis * pos},
    //     {z_axis.x, z_axis.y, z_axis.z, -z_axis * pos},
    //     {       0,        0,        0,             1}
    // };
    // return matrix4f_t(viewMatrix);

    return LookAt(pos, target, up);
}

void camera_t::set_default(void) {
    pos    = pos_default;
    target = target_default;
    up     = up_default;
    aspect = aspect_default;
    speed  = speed_default;
    return;
}

camera_t camera;
