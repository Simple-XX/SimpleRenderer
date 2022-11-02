
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
    uint8_t* color_data_ptr = (uint8_t*)&color_data;
    color_data_ptr[0]       = _r;
    color_data_ptr[1]       = _g;
    color_data_ptr[2]       = _b;
    color_data_ptr[3]       = _a;
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

uint8_t& color_t::operator[](const uint8_t _idx) {
    if (_idx > DEPTH) {
        throw std::invalid_argument(log("_idx > DEPTH"));
    }
    uint8_t* color_data_ptr = (uint8_t*)&color_data;
    return color_data_ptr[_idx];
}

uint8_t color_t::operator[](const uint8_t _idx) const {
    if (_idx > DEPTH) {
        throw std::invalid_argument(log("_idx > DEPTH"));
    }
    uint8_t* color_data_ptr = (uint8_t*)&color_data;
    return color_data_ptr[_idx];
}

size_t color_t::size(void) const {
    return DEPTH;
}

const color_t color_t::to_argb(void) const {
    if (color_order == COLOR_ORDER_ARGB) {
        return color_t(color_data, COLOR_ORDER_ARGB);
    }
    else {
        uint8_t* color_data_ptr = (uint8_t*)&color_data;
        return color_t(color_data_ptr[3], color_data_ptr[0], color_data_ptr[1],
                       color_data_ptr[2], COLOR_ORDER_ARGB);
    }
}

const color_t color_t::to_rgba(void) const {
    if (color_order == COLOR_ORDER_RGBA) {
        return color_t(color_data, COLOR_ORDER_RGBA);
    }
    else {
        uint8_t* color_data_ptr = (uint8_t*)&color_data;
        return color_t(color_data_ptr[1], color_data_ptr[2], color_data_ptr[3],
                       color_data_ptr[0], COLOR_ORDER_RGBA);
    }
}

const uint8_t* color_t::to_arr(void) const {
    return (uint8_t*)&color_data;
}
