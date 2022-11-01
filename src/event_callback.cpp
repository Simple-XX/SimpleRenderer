
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

event_callback_t::event_callback_t(config_t& _config, camera_t& _camera)
    : config(_config), camera(_camera) {
    return;
}

event_callback_t::~event_callback_t(void) {
    return;
}

void event_callback_t::key_a(void) {
    camera.get_pos().x -= camera.speed;
    return;
}

void event_callback_t::key_d(void) {
    camera.get_pos().x += camera.speed;
    return;
}

void event_callback_t::key_w(void) {
    camera.get_pos().z += camera.speed;
    return;
}

void event_callback_t::key_s(void) {
    camera.get_pos().z -= camera.speed;
    return;
}

void event_callback_t::key_z(void) {
    camera.get_pos().y += camera.speed;
    return;
}

void event_callback_t::key_space(void) {
    camera.get_pos().y -= camera.speed;
    return;
}

void event_callback_t::key_left_ctrl(void) {
    camera.get_pos().y -= camera.speed;
    return;
}

void event_callback_t::key_left_shift(void) {
    config.draw_wireframe = !config.draw_wireframe;
    return;
}

void event_callback_t::key_mouse_motion(void) {
    // camera.get_target().x += sdl_event.motion.xrel;
    // camera.get_target().y += sdl_event.motion.yrel;
    return;
}
