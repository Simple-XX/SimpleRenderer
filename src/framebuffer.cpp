
/**
 * @file framebuffer.cpp
 * @brief 缓冲区抽象
 * @author Zone.N (Zone.Niuzh@hotmail.com)
 * @version 1.0
 * @date 2022-09-03
 * @copyright MIT LICENSE
 * https://github.com/Simple-XX/SimpleRenderer
 * @par change log:
 * <table>
 * <tr><th>Date<th>Author<th>Description
 * <tr><td>2022-09-03<td>Zone.N<td>创建文件
 * </table>
 */

#include "memory"
#include "cassert"
#include "framebuffer.h"
#include "cstring"

framebuffer_t::framebuffer_t(const int32_t _width, const int32_t _height) {
    width        = _width;
    height       = _height;
    color_buffer = new color_t[width * height];
    depth_buffer = new depth_t[width * height];
    return;
}

framebuffer_t::framebuffer_t(const framebuffer_t &_framebuffer) {
    assert(width == _framebuffer.width);
    assert(height == _framebuffer.height);
    if (color_buffer == nullptr) {
        color_buffer = new color_t[width * height];
    }
    if (depth_buffer == nullptr) {
        depth_buffer = new depth_t[width * height];
    }
    if (_framebuffer.color_buffer != nullptr) {
        memcpy(color_buffer, _framebuffer.color_buffer, width * height * BPP);
    }
    if (_framebuffer.depth_buffer != nullptr) {
        memcpy(depth_buffer, _framebuffer.depth_buffer,
               width * height * BPP_DEPTH);
    }
    return;
}

framebuffer_t::~framebuffer_t(void) {
    if (color_buffer != nullptr) {
        delete[] color_buffer;
    }
    if (depth_buffer != nullptr) {
        delete[] depth_buffer;
    }
    return;
}

framebuffer_t &framebuffer_t::operator=(const framebuffer_t &_framebuffer) {
    if (&_framebuffer == this) {
        return *this;
    }
    assert(width == _framebuffer.get_width());
    assert(height == _framebuffer.get_height());
    if (color_buffer == nullptr) {
        color_buffer = new color_t[width * height];
    }
    if (depth_buffer == nullptr) {
        depth_buffer = new depth_t[width * height];
    }
    if (_framebuffer.color_buffer != nullptr) {
        memcpy(color_buffer, _framebuffer.color_buffer, width * height * BPP);
    }
    if (_framebuffer.depth_buffer != nullptr) {
        memcpy(depth_buffer, _framebuffer.depth_buffer,
               width * height * BPP_DEPTH);
    }
    return *this;
}

int32_t framebuffer_t::get_width(void) const {
    return width;
}

int32_t framebuffer_t::get_height(void) const {
    return height;
}

void framebuffer_t::clear(void) {
    for (auto i = 0; i < width; i++) {
        for (auto j = 0; j < height; j++) {
            pixel(i, j, 0x00000000, 0);
        }
    }
    return;
}

void framebuffer_t::clear(const color_t &_color, const depth_t &_depth) {
    for (auto i = 0; i < width; i++) {
        for (auto j = 0; j < height; j++) {
            pixel(i, j, _color, _depth);
        }
    }
    return;
}

void framebuffer_t::pixel(const int32_t _x, const int32_t _y,
                          const color_t &_color, const depth_t &_depth) {
    if (_x <= 0 || _y < 0 || _x >= width || _y >= height) {
        return;
    }
    std::lock_guard<std::mutex> color_buffer_lock(color_buffer_mutex);
    std::lock_guard<std::mutex> depth_buffer_lock(depth_buffer_mutex);
    color_buffer[_y * width + _x] = _color;
    depth_buffer[_y * width + _x] = _depth;
    return;
}

const framebuffer_t::color_t *framebuffer_t::get_color_buffer(void) const {
    return color_buffer;
}

framebuffer_t::color_t framebuffer_t::ARGB(const uint8_t _a, const uint8_t _r,
                                           const uint8_t _g, const uint8_t _b) {
    color_t color = 0x00000000;
    color =
        (_a & 0xFF) << 24 | (_r & 0xFF) << 16 | (_g & 0xFF) << 8 | (_b & 0xFF);
    return color;
}
