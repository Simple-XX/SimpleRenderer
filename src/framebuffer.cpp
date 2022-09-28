
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

template <class _T>
framebuffer_t::depth_buffer_t<_T>::depth_buffer_t(
    const depth_buffer_t<_T> &_depth_buffer)
    : width(_depth_buffer.width), height(_depth_buffer.height) {
    try {
        depth_arr = new _T[width * height];
    } catch (const std::bad_alloc &e) {
        std::cerr << log(e.what()) << std::endl;
    }
    // 复制数据
    std::copy(_depth_buffer.depth_arr,
              _depth_buffer.depth_arr + _depth_buffer.length(), depth_arr);
    return;
}

template <class _T>
framebuffer_t::depth_buffer_t<_T>::depth_buffer_t(const int32_t _w,
                                                  const int32_t _h,
                                                  const _T      _val)
    : width(_w), height(_h) {
    try {
        depth_arr = new _T[width * height];
    } catch (const std::bad_alloc &e) {
        std::cerr << log(e.what()) << std::endl;
    }
    std::fill_n(depth_arr, width * height, _val);
    return;
}

template <class _T>
framebuffer_t::depth_buffer_t<_T>::~depth_buffer_t(void) {
    width  = 0;
    height = 0;
    if (depth_arr != nullptr) {
        delete[] depth_arr;
    }
    return;
}

template <class _T>
framebuffer_t::depth_buffer_t<_T> &framebuffer_t::depth_buffer_t<_T>::operator=(
    const depth_buffer_t<_T> &_depth_buffer) {
    if (width != _depth_buffer.width || height != _depth_buffer.height) {
        width  = _depth_buffer.width;
        height = _depth_buffer.height;
        if (depth_arr != nullptr) {
            delete[] depth_arr;
        }
        try {
            depth_arr = new _T[width * height];
        } catch (const std::bad_alloc &e) {
            std::cerr << log(e.what()) << std::endl;
        }
    }
    std::copy(_depth_buffer.depth_arr,
              _depth_buffer.depth_arr + width * height * sizeof(_T), depth_arr);

    return *this;
}

template <class _T>
_T &framebuffer_t::depth_buffer_t<_T>::operator()(const int32_t _x,
                                                  const int32_t _y) {
    if (_x >= width) {
        throw std::invalid_argument(log("_x >= width"));
    }
    if (_y >= height) {
        throw std::invalid_argument(log("_y >= height"));
    }
    return depth_arr[_y * width + _x];
}

template <class _T>
const _T framebuffer_t::depth_buffer_t<_T>::operator()(const int32_t _x,
                                                       const int32_t _y) const {
    if (_x >= width) {
        throw std::invalid_argument(log("_x >= width"));
    }
    if (_y >= height) {
        throw std::invalid_argument(log("_y >= height"));
    }
    return depth_arr[_y * width + _x];
}

template <class _T>
void framebuffer_t::depth_buffer_t<_T>::clear(const _T &_val) {
    if (std::isnan(_val)) {
        throw(log("std::isnan(_depth)"));
    }
    std::fill_n(depth_arr, width * height, _val);
    return;
}
template <class _T>
size_t framebuffer_t::depth_buffer_t<_T>::length(void) const {
    return width * height * sizeof(_T);
}

framebuffer_t::framebuffer_t(const int32_t _width, const int32_t _height)
    : width(_width), height(_height),
      depth_buffer(depth_buffer_t<depth_t>(_width, _height)) {
    color_buffer = new color_t[width * height];
    std::fill_n(color_buffer, width * height, 0x00000000);
    return;
}

framebuffer_t::framebuffer_t(const framebuffer_t &_framebuffer)
    : depth_buffer(_framebuffer.depth_buffer) {
    if (width != _framebuffer.get_width()) {
        throw std::invalid_argument(log("width != _framebuffer.get_width()"));
    }
    if (height != _framebuffer.get_height()) {
        throw std::invalid_argument(log("height != _framebuffer.get_height()"));
    }
    if (color_buffer == nullptr) {
        throw std::invalid_argument(log("color_buffer == nullptr"));
    }
    if (_framebuffer.get_color_buffer() == nullptr) {
        throw std::invalid_argument(
            log("_framebuffer.get_color_buffer() == nullptr"));
    }
    memcpy(color_buffer, _framebuffer.color_buffer, width * height * BPP);

    return;
}

framebuffer_t::~framebuffer_t(void) {
    if (color_buffer != nullptr) {
        delete[] color_buffer;
    }
    return;
}

framebuffer_t &framebuffer_t::operator=(const framebuffer_t &_framebuffer) {
    if (this == &_framebuffer) {
        throw std::runtime_error(log("this == &_framebuffer"));
    }
    if (width != _framebuffer.get_width()) {
        throw std::invalid_argument(log("width != _framebuffer.get_width()"));
    }
    if (height != _framebuffer.get_height()) {
        throw std::invalid_argument(log("height != _framebuffer.get_height()"));
    }
    if (color_buffer == nullptr) {
        throw std::invalid_argument(log("color_buffer == nullptr"));
    }
    if (_framebuffer.get_color_buffer() == nullptr) {
        throw std::invalid_argument(
            log("_framebuffer.get_color_buffer() == nullptr"));
    }
    memcpy(color_buffer, _framebuffer.get_color_buffer(), width * height * BPP);
    depth_buffer = _framebuffer.depth_buffer;
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
            pixel(i, j, 0x00000000, std::numeric_limits<depth_t>::lowest());
        }
    }
    return;
}

void framebuffer_t::clear(const color_t &_color, const depth_t &_depth) {
    if (std::isnan(_depth)) {
        throw(log("std::isnan(_depth)"));
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
    if (_x < 0 || _y < 0 || _x >= width || _y >= height) {
        throw(std::invalid_argument(
            log("_x < 0 || _y < 0 || _x >= width || _y >= height")));
    }
    if (std::isnan(_depth)) {
        throw(log("std::isnan(_depth)"));
    }
    std::lock_guard<std::mutex> color_buffer_lock(color_buffer_mutex);
    std::lock_guard<std::mutex> depth_buffer_lock(depth_buffer_mutex);
    color_buffer[_y * width + _x] = _color;
    depth_buffer(_x, _y)          = _depth;
    return;
}

const framebuffer_t::color_t *framebuffer_t::get_color_buffer(void) const {
    return color_buffer;
}

framebuffer_t::depth_buffer_t<framebuffer_t::depth_t> &
framebuffer_t::get_depth_buffer(void) {
    return depth_buffer;
}

const framebuffer_t::depth_buffer_t<framebuffer_t::depth_t> &
framebuffer_t::get_depth_buffer(void) const {
    return depth_buffer;
}

framebuffer_t::color_t framebuffer_t::ARGB(const uint8_t _a, const uint8_t _r,
                                           const uint8_t _g, const uint8_t _b) {
    color_t color = 0x00000000;
    color =
        (_a & 0xFF) << 24 | (_r & 0xFF) << 16 | (_g & 0xFF) << 8 | (_b & 0xFF);
    return color;
}
