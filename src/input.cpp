
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

/// @todo 补全

void input_t::key_a(const uint32_t _delta_time) {
    camera.update_pos(camera_base_t::SUB_X, _delta_time);
    return;
}

void input_t::key_d(const uint32_t _delta_time) {
    camera.update_pos(camera_base_t::ADD_X, _delta_time);
    return;
}

void input_t::key_w(const uint32_t _delta_time) {
    camera.update_pos(camera_base_t::ADD_Z, _delta_time);
    return;
}

void input_t::key_s(const uint32_t _delta_time) {
    camera.update_pos(camera_base_t::SUB_Z, _delta_time);
    return;
}

void input_t::key_z(const uint32_t _delta_time) {
    camera.update_pos(camera_base_t::SUB_Y, _delta_time);
    return;
}

void input_t::key_r(const uint32_t) {
    camera.set_default();
    return;
}

void input_t::key_q(const uint32_t _delta_time) {
    camera.update_up(camera_base_t::ADD_Y, _delta_time);
    return;
}

void input_t::key_e(const uint32_t _delta_time) {
    camera.update_up(camera_base_t::SUB_Y, _delta_time);

    return;
}

void input_t::key_space(const uint32_t _delta_time) {
    camera.update_pos(camera_base_t::ADD_Y, _delta_time);
    return;
}

void input_t::key_left_ctrl(const uint32_t _delta_time) {
    return;
}

void input_t::key_left_shift(const uint32_t _delta_time) {
    config.draw_wireframe = !config.draw_wireframe;
    return;
}

void input_t::mouse_motion(const int32_t _x, const int32_t _y,
                           const uint32_t _delta_time) {
    // camera.update_target(_x, _y);
    return;
}

bool input_t::handle(SDL_Event* _event, const uint32_t _delta_time) {
    auto res = true;
    switch (_event->type) {
        // 键盘事件
        case SDL_KEYDOWN: {
            switch (_event->key.keysym.sym) {
                case SDLK_ESCAPE: {
                    // 如果是 esc 键则返回 false，程序退出
                    res = false;
                    break;
                }
                case SDLK_a: {
                    key_a(_delta_time);
                    break;
                }
                case SDLK_d: {
                    key_d(_delta_time);
                    break;
                }
                case SDLK_SPACE: {
                    key_space(_delta_time);
                    break;
                }
                case SDLK_z: {
                    key_z(_delta_time);
                    break;
                }
                case SDLK_r: {
                    key_r(_delta_time);
                    break;
                }
                case SDLK_LCTRL: {
                    key_left_ctrl(_delta_time);
                    break;
                }
                case SDLK_w: {
                    key_w(_delta_time);
                    break;
                }
                case SDLK_s: {
                    key_s(_delta_time);
                    break;
                }
                case SDLK_q: {
                    key_q(_delta_time);
                    break;
                }
                case SDLK_e: {
                    key_e(_delta_time);
                    break;
                }
                case SDLK_LSHIFT: {
                    key_left_shift(_delta_time);
                    break;
                }
                default: {
                    // 输出按键名
                    printf("key %s down！\n",
                           SDL_GetKeyName(_event->key.keysym.sym));
                }
            }
            break;
        }
        // 鼠标移动
        case SDL_MOUSEMOTION: {
            mouse_motion(_event->motion.xrel, _event->motion.yrel, _delta_time);
            break;
        }
        // 鼠标点击
        case SDL_MOUSEBUTTONDOWN: {
            /// @todo
            break;
        }
        // 鼠标滚轮
        case SDL_MOUSEWHEEL: {
            /// @todo
            break;
        }
    }
    return res;
}

input_t::input_t(config_t& _config, surround_camera_t& _camera)
    : config(_config), camera(_camera) {
    return;
}

input_t::~input_t(void) {
    return;
}

bool input_t::process(const uint32_t _delta_time) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            return false;
        }
        return handle(&event, _delta_time);
    }
    return true;
}
