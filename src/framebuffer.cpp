
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

#include "iostream"
#include "memory"
#include "cassert"
#include "cstring"
#include "cmath"
#include "framebuffer.h"
#include "log.hpp"

framebuffer_t::framebuffer_t(const int32_t _width, const int32_t _height) {
    width  = _width;
    height = _height;
    try {
        color_buffer = new color_t[width * height];
        depth_buffer = new depth_t[width * height];
    } catch (const std::bad_alloc &e) {
        std::cerr << log(e.what()) << std::endl;
        if (color_buffer != nullptr) {
            delete[] color_buffer;
        }
        if (depth_buffer != nullptr) {
            delete[] depth_buffer;
        }
        return;
    }

    return;
}

framebuffer_t::framebuffer_t(const framebuffer_t &_framebuffer) {
    try {
        if (width != _framebuffer.get_width()) {
            throw std::invalid_argument(
                log("width != _framebuffer.get_width()"));
        }
        if (height != _framebuffer.get_height()) {
            throw std::invalid_argument(
                log("height != _framebuffer.get_height()"));
        }
        if (color_buffer == nullptr) {
            throw std::invalid_argument(log("color_buffer == nullptr"));
        }
        if (depth_buffer == nullptr) {
            throw std::invalid_argument(log("depth_buffer == nullptr"));
        }
        if (_framebuffer.get_color_buffer() == nullptr) {
            throw std::invalid_argument(
                log("_framebuffer.get_color_buffer() == nullptr"));
        }
        if (_framebuffer.get_depth_buffer() == nullptr) {
            throw std::invalid_argument(
                log("_framebuffer.get_depth_buffer() == nullptr"));
        }
    } catch (const std::invalid_argument &e) {
        std::cerr << e.what() << std::endl;
        return;
    }
    memcpy(color_buffer, _framebuffer.color_buffer, width * height * BPP);
    memcpy(depth_buffer, _framebuffer.depth_buffer, width * height * BPP_DEPTH);
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
    try {
        if (this == &_framebuffer) {
            throw std::runtime_error(log("this == &_framebuffer"));
        }
        if (width != _framebuffer.get_width()) {
            throw std::invalid_argument(
                log("width != _framebuffer.get_width()"));
        }
        if (height != _framebuffer.get_height()) {
            throw std::invalid_argument(
                log("height != _framebuffer.get_height()"));
        }
        if (color_buffer == nullptr) {
            throw std::invalid_argument(log("color_buffer == nullptr"));
        }
        if (depth_buffer == nullptr) {
            throw std::invalid_argument(log("depth_buffer == nullptr"));
        }
        if (_framebuffer.get_color_buffer() == nullptr) {
            throw std::invalid_argument(
                log("_framebuffer.get_color_buffer() == nullptr"));
        }
        if (_framebuffer.get_depth_buffer() == nullptr) {
            throw std::invalid_argument(
                log("_framebuffer.get_depth_buffer() == nullptr"));
        }
    } catch (const std::invalid_argument &e) {
        std::cerr << e.what() << std::endl;
        return *this;
    }
    memcpy(color_buffer, _framebuffer.get_color_buffer(), width * height * BPP);
    memcpy(depth_buffer, _framebuffer.get_depth_buffer(),
           width * height * BPP_DEPTH);
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
    try {
        if (std::isnan(_depth)) {
            throw(log("std::isnan(_depth)"));
        }
    } catch (const std::invalid_argument &e) {
        std::cerr << e.what() << std::endl;
        return;
    }
    for (auto i = 0; i < width; i++) {
        for (auto j = 0; j < height; j++) {
            pixel(i, j, _color, _depth);
        }
    }
    return;
}

void framebuffer_t::pixel(const int32_t _x, const int32_t _y,
                          const color_t &_color, const depth_t &_depth) {
    try {
        if (_x < 0 || _y < 0 || _x >= width || _y >= height) {
            throw(std::invalid_argument(
                log("_x < 0 || _y < 0 || _x >= width || _y >= height")));
        }

        if (std::isnan(_depth)) {
            throw(log("std::isnan(_depth)"));
        }
    } catch (const std::invalid_argument &e) {
        std::cerr << e.what() << std::endl;
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

const framebuffer_t::depth_t *framebuffer_t::get_depth_buffer(void) const {
    return depth_buffer;
}

framebuffer_t::color_t framebuffer_t::ARGB(const uint8_t _a, const uint8_t _r,
                                           const uint8_t _g, const uint8_t _b) {
    color_t color = 0x00000000;
    color =
        (_a & 0xFF) << 24 | (_r & 0xFF) << 16 | (_g & 0xFF) << 8 | (_b & 0xFF);
    return color;
}
