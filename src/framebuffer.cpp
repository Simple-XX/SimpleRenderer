
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

framebuffer_t::framebuffer_t(void) {
    width        = 0;
    height       = 0;
    color_buffer = nullptr;
    depth_buffer = nullptr;
    return;
}

framebuffer_t::framebuffer_t(uint32_t _width, uint32_t _height) {
    width        = _width;
    height       = _height;
    color_buffer = new color_t[width * height];
    depth_buffer = new depth_buffer_t[width * height];
    return;
}

framebuffer_t::framebuffer_t(const framebuffer_t &_framebuffer) {
    assert(width == _framebuffer.width);
    assert(height == _framebuffer.height);
    if (color_buffer == nullptr) {
        color_buffer = new color_t[width * height];
    }
    if (depth_buffer == nullptr) {
        depth_buffer = new depth_buffer_t[width * height];
    }
    if (_framebuffer.color_buffer != nullptr) {
        memcpy(color_buffer, _framebuffer.color_buffer, width * height * BPP);
    }
    if (_framebuffer.depth_buffer != nullptr) {
        memcpy(depth_buffer, _framebuffer.depth_buffer,
               width * height * sizeof(depth_buffer_t));
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
    assert(&_framebuffer == this);
    assert(width == _framebuffer.width);
    assert(height == _framebuffer.height);
    if (color_buffer == nullptr) {
        color_buffer = new color_t[width * height];
    }
    if (depth_buffer == nullptr) {
        depth_buffer = new depth_buffer_t[width * height];
    }
    if (_framebuffer.color_buffer != nullptr) {
        memcpy(color_buffer, _framebuffer.color_buffer, width * height * BPP);
    }
    if (_framebuffer.depth_buffer != nullptr) {
        memcpy(depth_buffer, _framebuffer.depth_buffer,
               width * height * sizeof(depth_buffer_t));
    }
    return *this;
}

void framebuffer_t::clear(void) {
    for (uint32_t i = 0; i < height; i++) {
        for (uint32_t j = 0; j < width; j++) {
            color_buffer[i * width + j] = 0;
        }
    }
    return;
}

void framebuffer_t::clear(const color_t &_color) {
    for (uint32_t i = 0; i < height; i++) {
        for (uint32_t j = 0; j < width; j++) {
            color_buffer[i * width + j] = _color;
        }
    }
    return;
}

void framebuffer_t::pixel(int _x, int _y, const color_t &_color) {
    color_buffer[_y * width + _x] = _color;
    return;
}

framebuffer_t::color_t framebuffer_t::ARGB(const uint8_t _r, const uint8_t _g,
                                           const uint8_t _b, const uint8_t _a) {
    color_t color;
    /// @see https://en.wikipedia.org/wiki/RGBA_color_model
    color = _a << 24 | _r << 16 | _g << 8 | _b;
    return color;
}
