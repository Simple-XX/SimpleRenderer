
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

#include "cassert"
#include "cmath"
#include "iostream"

#include "framebuffer.h"
#include "log.hpp"

framebuffer_t::framebuffer_t(const uint32_t _width, const uint32_t _height)
    : width(_width),
      height(_height),
      color_buffer(color_buffer_t(_width, _height)),
      depth_buffer(depth_buffer_t(_width, _height)) {
    return;
}

framebuffer_t::framebuffer_t(const framebuffer_t& _framebuffer)
    : color_buffer(_framebuffer.color_buffer),
      depth_buffer(_framebuffer.depth_buffer) {
    if (width != _framebuffer.get_width()) {
        throw std::invalid_argument(log("width != _framebuffer.get_width()"));
    }
    if (height != _framebuffer.get_height()) {
        throw std::invalid_argument(log("height != _framebuffer.get_height()"));
    }
    return;
}

framebuffer_t::~framebuffer_t(void) {
    return;
}

framebuffer_t& framebuffer_t::operator=(const framebuffer_t& _framebuffer) {
    if (this == &_framebuffer) {
        throw std::runtime_error(log("this == &_framebuffer"));
    }
    if (width != _framebuffer.get_width()) {
        throw std::invalid_argument(log("width != _framebuffer.get_width()"));
    }
    if (height != _framebuffer.get_height()) {
        throw std::invalid_argument(log("height != _framebuffer.get_height()"));
    }
    color_buffer = _framebuffer.color_buffer;
    depth_buffer = _framebuffer.depth_buffer;
    return *this;
}

uint32_t framebuffer_t::get_width(void) const {
    return width;
}

uint32_t framebuffer_t::get_height(void) const {
    return height;
}

void framebuffer_t::clear(const color_t& _color, const depth_t& _depth) {
    if (std::isnan(_depth)) {
        throw(log("std::isnan(_depth)"));
    }
    (void)_color;
    (void)_depth;
    color_buffer.clear();
    depth_buffer.clear();
    return;
}

void framebuffer_t::pixel(const uint32_t _x, const uint32_t _y,
                          const color_t& _color, const depth_t& _depth) {
    if (_x >= width) {
        throw std::invalid_argument(log("_x >= width"));
    }
    if (_y >= height) {
        throw std::invalid_argument(log("_y >= height"));
    }
    if (std::isnan(_depth)) {
        throw(log("std::isnan(_depth)"));
    }
    /// @todo 性能瓶颈
    // std::lock_guard<std::mutex> color_buffer_lock(color_buffer_mutex);
    // std::lock_guard<std::mutex> depth_buffer_lock(depth_buffer_mutex);
    color_buffer(_x, _y) = _color;
    depth_buffer(_x, _y) = _depth;
    return;
}

color_buffer_t& framebuffer_t::get_color_buffer(void) {
    return color_buffer;
}

const color_buffer_t& framebuffer_t::get_color_buffer(void) const {
    return color_buffer;
}

depth_buffer_t& framebuffer_t::get_depth_buffer(void) {
    return depth_buffer;
}

framebuffer_t::depth_t&
framebuffer_t::get_depth_buffer(const uint32_t _x, const uint32_t _y) {
    return depth_buffer(_x, _y);
}

const depth_buffer_t& framebuffer_t::get_depth_buffer(void) const {
    return depth_buffer;
}

framebuffer_t::depth_t
framebuffer_t::get_depth_buffer(const uint32_t _x, const uint32_t _y) const {
    return depth_buffer(_x, _y);
}
