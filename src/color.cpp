
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

color_t::color_t(void) : color_data(0), color_order(COLOR_ORDER_RGBA) {
    return;
}

color_t::color_t(const uint32_t _data, const color_order_t& _color_order)
    : color_data(_data), color_order(_color_order) {
    if ((_color_order != COLOR_ORDER_RGBA)
        && (_color_order != COLOR_ORDER_ARGB)) {
        throw std::invalid_argument(log("invalid _color_order"));
    }
    return;
}

color_t::color_t(const uint8_t _r, const uint8_t _g, const uint8_t _b,
                 const uint8_t _a, const color_order_t& _color_order)
    : color_order(_color_order) {
    if ((_color_order != COLOR_ORDER_RGBA)
        && (_color_order != COLOR_ORDER_ARGB)) {
        throw std::invalid_argument(log("invalid _color_order"));
    }
    // std::cout<<"color_t uint8_t"<<std::endl;
    auto color_data_ptr = (uint8_t*)&color_data;
    color_data_ptr[0]   = _r;
    color_data_ptr[1]   = _g;
    color_data_ptr[2]   = _b;
    color_data_ptr[3]   = _a;
    return;
}

color_t::color_t(const float& _r, const float& _g, const float& _b,
                 const float& _a, const color_order_t& _color_order)
    : color_order(_color_order) {
    if ((_color_order != COLOR_ORDER_RGBA)
        && (_color_order != COLOR_ORDER_ARGB)) {
        throw std::invalid_argument(log("invalid _color_order"));
    }
    // std::cout<<"color_t float"<<std::endl;
    auto color_data_ptr = (uint8_t*)&color_data;
    color_data_ptr[0]   = _r * std::numeric_limits<uint8_t>::max();
    color_data_ptr[1]   = _g * std::numeric_limits<uint8_t>::max();
    color_data_ptr[2]   = _b * std::numeric_limits<uint8_t>::max();
    color_data_ptr[3]   = _a * std::numeric_limits<uint8_t>::max();
    return;
}

color_t::color_t(const color_t& _color)
    : color_data(_color.color_data), color_order(_color.color_order) {
    return;
}

color_t::~color_t(void) {
    return;
}

color_t& color_t::operator=(const color_t& _color) {
    if (this == &_color) {
        throw std::runtime_error(log("this == &_color"));
    }
    color_data  = _color.color_data;
    color_order = _color.color_order;
    return *this;
}

const color_t color_t::operator*(const float& _f) const {
    auto    color_data_ptr = (uint8_t*)&color_data;
    uint8_t new_0          = color_data_ptr[0] * _f;
    uint8_t new_1          = color_data_ptr[1] * _f;
    uint8_t new_2          = color_data_ptr[2] * _f;
    uint8_t new_3          = color_data_ptr[3] * _f;
    return color_t(new_0, new_1, new_2, new_3, color_order);
}

const color_t color_t::operator*(const vector4f_t& _vector) const {
    auto    color_data_ptr = (uint8_t*)&color_data;
    uint8_t new_0          = color_data_ptr[0] * _vector.x;
    uint8_t new_1          = color_data_ptr[1] * _vector.y;
    uint8_t new_2          = color_data_ptr[2] * _vector.z;
    uint8_t new_3          = color_data_ptr[3] * _vector.w;
    return color_t(new_0, new_1, new_2, new_3, color_order);
}

color_t& color_t::operator*=(const float& _f) {
    auto color_data_ptr = (uint8_t*)&color_data;
    color_data_ptr[0]   *= _f;
    color_data_ptr[1]   *= _f;
    color_data_ptr[2]   *= _f;
    color_data_ptr[3]   *= _f;
    return *this;
}

color_t& color_t::operator*=(const vector4f_t& _vector) {
    auto color_data_ptr = (uint8_t*)&color_data;
    color_data_ptr[0]   *= _vector.x;
    color_data_ptr[1]   *= _vector.y;
    color_data_ptr[2]   *= _vector.z;
    color_data_ptr[3]   *= _vector.w;
    return *this;
}

uint8_t& color_t::operator[](const uint8_t _idx) {
    if (_idx > DEPTH) {
        throw std::invalid_argument(log("_idx > DEPTH"));
    }
    auto color_data_ptr = (uint8_t*)&color_data;
    return color_data_ptr[_idx];
}

uint8_t color_t::operator[](const uint8_t _idx) const {
    if (_idx > DEPTH) {
        throw std::invalid_argument(log("_idx > DEPTH"));
    }
    auto color_data_ptr = (uint8_t*)&color_data;
    return color_data_ptr[_idx];
}

size_t color_t::size(void) const {
    return DEPTH;
}

uint32_t color_t::to_uint32(void) const {
    return color_data;
}

const color_t color_t::to_argb(void) const {
    if (color_order == COLOR_ORDER_ARGB) {
        return color_t(color_data, COLOR_ORDER_ARGB);
    }
    else {
        auto color_data_ptr = (uint8_t*)&color_data;
        return color_t(color_data_ptr[3], color_data_ptr[0], color_data_ptr[1],
                       color_data_ptr[2], COLOR_ORDER_ARGB);
    }
}

const color_t color_t::to_rgba(void) const {
    if (color_order == COLOR_ORDER_RGBA) {
        return color_t(color_data, COLOR_ORDER_RGBA);
    }
    else {
        auto color_data_ptr = (uint8_t*)&color_data;
        return color_t(color_data_ptr[1], color_data_ptr[2], color_data_ptr[3],
                       color_data_ptr[0], COLOR_ORDER_RGBA);
    }
}

const uint8_t* color_t::to_arr(void) const {
    return (uint8_t*)&color_data;
}

color_t color_t::WHITE = color_t((uint8_t)0xFF, 0xFF, 0xFF);
color_t color_t::BLACK = color_t((uint8_t)0x00, 0x00, 0x00);
color_t color_t::RED   = color_t((uint8_t)0xFF, 0x00, 0x00);
color_t color_t::GREEN = color_t((uint8_t)0x00, 0xFF, 0x00);
color_t color_t::BLUE  = color_t((uint8_t)0x00, 0x00, 0xFF);
