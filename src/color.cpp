
/**
 * @file color.cpp
 * @brief 颜色实现
 * @author Zone.N (Zone.Niuzh@hotmail.com)
 * @version 1.0
 * @date 2022-11-02
 * @copyright MIT LICENSE
 * https://github.com/Simple-XX/SimpleRenderer
 * @par change log:
 * <table>
 * <tr><th>Date<th>Author<th>Description
 * <tr><td>2022-11-02<td>Zone.N<td>创建文件
 * </table>
 */

#include "limits"

#include "color.h"
#include "log.hpp"

color_t::color_t(void) {
    color_data[0] = 0;
    color_data[1] = 0;
    color_data[2] = 0;
    color_data[3] = std::numeric_limits<uint8_t>::max();
    return;
}

color_t::color_t(const uint32_t _data) {
    auto data_ptr = (uint8_t*)&_data;
    color_data[0] = data_ptr[0];
    color_data[1] = data_ptr[1];
    color_data[2] = data_ptr[2];
    color_data[3] = data_ptr[3];
    return;
}

color_t::color_t(const uint8_t _r, const uint8_t _g, const uint8_t _b,
                 const uint8_t _a) {
    color_data[0] = _r;
    color_data[1] = _g;
    color_data[2] = _b;
    color_data[3] = _a;
    return;
}

color_t::color_t(const float& _r, const float& _g, const float& _b,
                 const uint8_t _a) {
    color_data[0] = uint8_t(_r * std::numeric_limits<uint8_t>::max());
    color_data[1] = uint8_t(_g * std::numeric_limits<uint8_t>::max());
    color_data[2] = uint8_t(_b * std::numeric_limits<uint8_t>::max());
    color_data[3] = _a;
    return;
}

color_t::color_t(const color_t& _color) {
    color_data[0] = _color.color_data[0];
    color_data[1] = _color.color_data[1];
    color_data[2] = _color.color_data[2];
    color_data[3] = _color.color_data[3];
    return;
}

color_t::~color_t(void) {
    return;
}

color_t& color_t::operator=(const color_t& _color) {
    if (this == &_color) {
        throw std::runtime_error(log("this == &_color"));
    }
    color_data[0] = _color.color_data[0];
    color_data[1] = _color.color_data[1];
    color_data[2] = _color.color_data[2];
    color_data[3] = _color.color_data[3];
    return *this;
}

const color_t color_t::operator*(const float& _f) const {
    uint8_t r = color_data[0] * _f;
    uint8_t g = color_data[1] * _f;
    uint8_t b = color_data[2] * _f;
    uint8_t a = color_data[3] * _f;
    return color_t(r, g, b, a);
}

const color_t color_t::operator*(const vector4f_t& _vector) const {
    uint8_t r = color_data[0] * _vector.x;
    uint8_t g = color_data[1] * _vector.y;
    uint8_t b = color_data[2] * _vector.z;
    uint8_t a = color_data[3] * _vector.w;
    return color_t(r, g, b, a);
}

color_t& color_t::operator*=(const float& _f) {
    color_data[0] *= _f;
    color_data[1] *= _f;
    color_data[2] *= _f;
    color_data[3] *= _f;
    return *this;
}

color_t& color_t::operator*=(const vector4f_t& _vector) {
    color_data[0] *= _vector.x;
    color_data[1] *= _vector.y;
    color_data[2] *= _vector.z;
    color_data[3] *= _vector.w;
    return *this;
}

uint8_t& color_t::operator[](const uint8_t _idx) {
    if (_idx > DEPTH) {
        throw std::invalid_argument(log("_idx > DEPTH"));
    }
    return color_data[_idx];
}

uint8_t color_t::operator[](const uint8_t _idx) const {
    if (_idx > DEPTH) {
        throw std::invalid_argument(log("_idx > DEPTH"));
    }
    return color_data[_idx];
}

size_t color_t::bpp(void) {
    return BPP;
}

uint32_t color_t::to_uint32(void) const {
    return *(uint32_t*)color_data;
}

const uint8_t* color_t::to_arr(void) const {
    return color_data;
}

color_t color_t::WHITE = color_t((uint8_t)0xFF, 0xFF, 0xFF);
color_t color_t::BLACK = color_t((uint8_t)0x00, 0x00, 0x00);
color_t color_t::RED   = color_t((uint8_t)0xFF, 0x00, 0x00);
color_t color_t::GREEN = color_t((uint8_t)0x00, 0xFF, 0x00);
color_t color_t::BLUE  = color_t((uint8_t)0x00, 0x00, 0xFF);
