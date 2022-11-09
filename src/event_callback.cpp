
/**
 * @file event_callback.cpp
 * @brief 事件处理
 * @author Zone.N (Zone.Niuzh@hotmail.com)
 * @version 1.0
 * @date 2022-11-01
 * @copyright MIT LICENSE
 * https://github.com/Simple-XX/SimpleRenderer
 * @par change log:
 * <table>
 * <tr><th>Date<th>Author<th>Description
 * <tr><td>2022-11-01<td>Zone.N<td>创建文件
 * </table>
 */

#include "event_callback.h"

event_callback_t::event_callback_t(config_t&          _config,
                                   surround_camera_t& _camera)
    : config(_config), camera(_camera) {
    return;
}

event_callback_t::~event_callback_t(void) {
    return;
}

void event_callback_t::key_a(void) {
    camera.update_pos_x(false);
    return;
}

void event_callback_t::key_d(void) {
    camera.update_pos_x(true);
    return;
}

void event_callback_t::key_w(void) {
    camera.update_pos_z(true);
    return;
}

void event_callback_t::key_s(void) {
    camera.update_pos_z(false);
    return;
}

void event_callback_t::key_z(void) {
    camera.update_pos_y(true);
    return;
}

void event_callback_t::key_r(void) {
    camera.set_default();
    return;
}

void event_callback_t::key_q(void) {
    /// @todo 完善
    // camera.update_up_x();
    return;
}

void event_callback_t::key_e(void) {
    /// @todo 完善
    // camera.update_up_y();
    return;
}

void event_callback_t::key_space(void) {
    camera.update_pos_y(false);
    return;
}

void event_callback_t::key_left_ctrl(void) {
    return;
}

void event_callback_t::key_left_shift(void) {
    config.draw_wireframe = !config.draw_wireframe;
    return;
}

void event_callback_t::mouse_motion(const int32_t _x, const int32_t _y) {
    camera.update_target(_x, _y);
    return;
}
