
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
#include "cstring"
#include "iostream"
#include "memory"

#include "framebuffer.h"
#include "log.hpp"

template <class _T>
framebuffer_t::color_buffer_t<_T>::color_buffer_t(
  const color_buffer_t<_T>& _color_buffer)
    : width(_color_buffer.width), height(_color_buffer.height) {
    try {
        color_arr = new _T[width * height];
    } catch (const std::bad_alloc& e) {
        std::cerr << log(e.what()) << std::endl;
    }
    // 复制数据
    std::copy(_color_buffer.color_arr,
              _color_buffer.color_arr + _color_buffer.length(), color_arr);
    return;
}

template <class _T>
framebuffer_t::color_buffer_t<_T>::color_buffer_t(const uint32_t _w,
                                                  const uint32_t _h,
                                                  const _T&      _color)
    : width(_w), height(_h) {
    try {
        color_arr = new _T[width * height];
    } catch (const std::bad_alloc& e) {
        std::cerr << log(e.what()) << std::endl;
    }
    std::fill_n(color_arr, width * height, _color);
    return;
}

template <class _T>
framebuffer_t::color_buffer_t<_T>::~color_buffer_t(void) {
    width  = 0;
    height = 0;
    if (color_arr != nullptr) {
        delete[] color_arr;
    }
    return;
}

template <class _T>
framebuffer_t::color_buffer_t<_T>& framebuffer_t::color_buffer_t<_T>::operator=(
  const color_buffer_t<_T>& _color_buffer) {
    if (width != _color_buffer.width || height != _color_buffer.height) {
        width  = _color_buffer.width;
        height = _color_buffer.height;
        if (color_arr != nullptr) {
            delete[] color_arr;
        }
        try {
            color_arr = new _T[width * height];
        } catch (const std::bad_alloc& e) {
            std::cerr << log(e.what()) << std::endl;
        }
    }
    std::copy(_color_buffer.color_arr,
              _color_buffer.color_arr + width * height * sizeof(_T), color_arr);

    return *this;
}

template <class _T>
_T& framebuffer_t::color_buffer_t<_T>::operator()(const uint32_t _x,
                                                  const uint32_t _y) {
    if (_x >= width) {
        throw std::invalid_argument(log("_x >= width"));
    }
    if (_y >= height) {
        throw std::invalid_argument(log("_y >= height"));
    }
    return color_arr[_y * width + _x];
}

template <class _T>
const _T
framebuffer_t::color_buffer_t<_T>::operator()(const uint32_t _x,
                                              const uint32_t _y) const {
    if (_x >= width) {
        throw std::invalid_argument(log("_x >= width"));
    }
    if (_y >= height) {
        throw std::invalid_argument(log("_y >= height"));
    }
    return color_arr[_y * width + _x];
}

template <class _T>
void framebuffer_t::color_buffer_t<_T>::clear(const _T& _color) {
    std::fill_n(color_arr, width * height, _color);
    return;
}

template <class _T>
size_t framebuffer_t::color_buffer_t<_T>::length(void) const {
    return width * height * sizeof(_T);
}

template <class _T>
framebuffer_t::depth_buffer_t<_T>::depth_buffer_t(
  const depth_buffer_t<_T>& _depth_buffer)
    : width(_depth_buffer.width), height(_depth_buffer.height) {
    try {
        depth_arr = new _T[width * height];
    } catch (const std::bad_alloc& e) {
        std::cerr << log(e.what()) << std::endl;
    }
    // 复制数据
    std::copy(_depth_buffer.depth_arr,
              _depth_buffer.depth_arr + _depth_buffer.length(), depth_arr);
    return;
}

template <class _T>
framebuffer_t::depth_buffer_t<_T>::depth_buffer_t(const uint32_t _w,
                                                  const uint32_t _h,
                                                  const _T&      _depth)
    : width(_w), height(_h) {
    try {
        depth_arr = new _T[width * height];
    } catch (const std::bad_alloc& e) {
        std::cerr << log(e.what()) << std::endl;
    }
    std::fill_n(depth_arr, width * height, _depth);
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
framebuffer_t::depth_buffer_t<_T>& framebuffer_t::depth_buffer_t<_T>::operator=(
  const depth_buffer_t<_T>& _depth_buffer) {
    if (width != _depth_buffer.width || height != _depth_buffer.height) {
        width  = _depth_buffer.width;
        height = _depth_buffer.height;
        if (depth_arr != nullptr) {
            delete[] depth_arr;
        }
        try {
            depth_arr = new _T[width * height];
        } catch (const std::bad_alloc& e) {
            std::cerr << log(e.what()) << std::endl;
        }
    }
    std::copy(_depth_buffer.depth_arr,
              _depth_buffer.depth_arr + width * height * sizeof(_T), depth_arr);

    return *this;
}

template <class _T>
_T& framebuffer_t::depth_buffer_t<_T>::operator()(const uint32_t _x,
                                                  const uint32_t _y) {
    if (_x >= width) {
        throw std::invalid_argument(log("_x >= width"));
    }
    if (_y >= height) {
        throw std::invalid_argument(log("_y >= height"));
    }
    return depth_arr[_y * width + _x];
}

template <class _T>
const _T
framebuffer_t::depth_buffer_t<_T>::operator()(const uint32_t _x,
                                              const uint32_t _y) const {
    if (_x >= width) {
        throw std::invalid_argument(log("_x >= width"));
    }
    if (_y >= height) {
        throw std::invalid_argument(log("_y >= height"));
    }
    return depth_arr[_y * width + _x];
}

template <class _T>
void framebuffer_t::depth_buffer_t<_T>::clear(const _T& _depth) {
    if (std::isnan(_depth)) {
        throw(log("std::isnan(_depth)"));
    }
    std::fill_n(depth_arr, width * height, _depth);
    return;
}

template <class _T>
size_t framebuffer_t::depth_buffer_t<_T>::length(void) const {
    return width * height * sizeof(_T);
}

framebuffer_t::framebuffer_t(const uint32_t _width, const uint32_t _height)
    : width(_width),
      height(_height),
      color_buffer(color_buffer_t<color_t>(_width, _height)),
      depth_buffer(depth_buffer_t<depth_t>(_width, _height)) {
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

void framebuffer_t::clear(void) {
    for (uint32_t i = 0; i < width; i++) {
        for (uint32_t j = 0; j < height; j++) {
            pixel(i, j, 0x00000000, std::numeric_limits<depth_t>::lowest());
        }
    }
    return;
}

void framebuffer_t::clear(const color_t& _color, const depth_t& _depth) {
    if (std::isnan(_depth)) {
        throw(log("std::isnan(_depth)"));
    }
    for (uint32_t i = 0; i < width; i++) {
        for (uint32_t j = 0; j < height; j++) {
            pixel(i, j, _color, _depth);
        }
    }
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
    std::lock_guard<std::mutex> color_buffer_lock(color_buffer_mutex);
    std::lock_guard<std::mutex> depth_buffer_lock(depth_buffer_mutex);
    color_buffer(_x, _y) = _color;
    depth_buffer(_x, _y) = _depth;
    return;
}

framebuffer_t::color_buffer_t<framebuffer_t::color_t>&
framebuffer_t::get_color_buffer(void) {
    return color_buffer;
}

const framebuffer_t::color_buffer_t<framebuffer_t::color_t>&
framebuffer_t::get_color_buffer(void) const {
    return color_buffer;
}

framebuffer_t::depth_buffer_t<framebuffer_t::depth_t>&
framebuffer_t::get_depth_buffer(void) {
    return depth_buffer;
}

const framebuffer_t::depth_buffer_t<framebuffer_t::depth_t>&
framebuffer_t::get_depth_buffer(void) const {
    return depth_buffer;
}

framebuffer_t::color_t framebuffer_t::ARGB(const uint8_t _a, const uint8_t _r,
                                           const uint8_t _g, const uint8_t _b) {
    color_t color = 0x00000000;
    color
      = (_a & 0xFF) << 24 | (_r & 0xFF) << 16 | (_g & 0xFF) << 8 | (_b & 0xFF);
    return color;
}
