
/**
 * @file draw2d.cpp
 * @brief 二维绘制
 * @author Zone.N (Zone.Niuzh@hotmail.com)
 * @version 1.0
 * @date 2022-09-06
 * @copyright MIT LICENSE
 * https://github.com/Simple-XX/SimpleRenderer
 * @par change log:
 * <table>
 * <tr><th>Date<th>Author<th>Description
 * <tr><td>2022-09-06<td>Zone.N<td>创建文件
 * </table>
 */

#include "cmath"
#include "draw2d.h"

const std::pair<bool, vector4f_t>
draw2d_t::get_barycentric_coord(const vector4f_t &_p0, const vector4f_t &_p1,
                                const vector4f_t &_p2, const vector4f_t &_p) {
    // 边向量
    auto edge_p1p0 = (_p1 - _p0);
    auto edge_p2p0 = (_p2 - _p0);
    // 到点 _p 的向量
    auto edge_pp0 = (_p - _p0);

    float p1p0p1p0 = edge_p1p0 * edge_p1p0;
    float p1p0p2p0 = edge_p1p0 * edge_p2p0;
    float p2p0p2p0 = edge_p2p0 * edge_p2p0;
    float pp0p1p0  = edge_pp0 * edge_p1p0;
    float pp0p2p0  = edge_pp0 * edge_p2p0;

    auto deno = 1. / (p1p0p1p0 * p2p0p2p0 - p1p0p2p0 * p1p0p2p0);

    if (std::isinf(deno)) {
        return std::pair<bool, vector4f_t>(false, vector4f_t());
    }

    auto v = (p2p0p2p0 * pp0p1p0 - p1p0p2p0 * pp0p2p0) * deno;
    auto w = (p1p0p1p0 * pp0p2p0 - p1p0p2p0 * pp0p1p0) * deno;
    auto u = 1 - v - w;

    auto res = true;

    if (u < 0. || u > 1.) {
        res = false;
    }

    if (v < 0. || v > 1.) {
        res = false;
    }

    if (w < 0 || w > 1.) {
        res = false;
    }

    return std::pair<bool, vector4f_t>(res, vector4f_t(u, v, w));
}

draw2d_t::draw2d_t(framebuffer_t &_framebuffer) : framebuffer(_framebuffer) {
    width  = framebuffer.get_width();
    height = framebuffer.get_height();
    return;
}

draw2d_t::~draw2d_t(void) {
    return;
}

void draw2d_t::line(const int32_t _x0, const int32_t _y0, const int32_t _x1,
                    const int32_t _y1, const framebuffer_t::color_t &_color) {
    auto p0_x = _x0;
    auto p0_y = _y0;
    auto p1_x = _x1;
    auto p1_y = _y1;

    auto steep = false;
    if (std::abs(p0_x - p1_x) < std::abs(p0_y - p1_y)) {
        std::swap(p0_x, p0_y);
        std::swap(p1_x, p1_y);
        steep = true;
    }
    // 终点 x 坐标在起点 左边
    if (p0_x > p1_x) {
        std::swap(p0_x, p1_x);
        std::swap(p0_y, p1_y);
    }

    auto de  = 0;
    auto dy2 = std::abs(p1_y - p0_y) << 1;
    auto dx2 = std::abs(p1_x - p0_x) << 1;
    auto y   = p0_y;
    auto yi  = 1;
    if (p1_y <= p0_y) {
        yi = -1;
    }
    for (auto x = p0_x; x <= p1_x; x++) {
        if (steep == true) {
            framebuffer.pixel(y, x, _color);
        }
        else {
            framebuffer.pixel(x, y, _color);
        }
        de += std::abs(dy2);
        if (de >= dx2) {
            y += yi;
            de -= dx2;
        }
    }

    return;
}

void draw2d_t::line(const vector4f_t &_p0, const vector4f_t &_p1,
                    const framebuffer_t::color_t &_color) {
    line(_p0.x, _p0.y, _p1.x, _p1.y, _color);
    return;
}

void draw2d_t::triangle(const vector4f_t &_v0, const vector4f_t &_v1,
                        const vector4f_t             &_v2,
                        const framebuffer_t::color_t &_color) {
    auto min = _v0.min(_v1).min(_v2);
    auto max = _v0.max(_v1).max(_v2);

    for (auto x = min.x; x <= max.x; x++) {
        for (auto y = min.y; y <= max.y; y++) {
            auto [is_inside, _] =
                get_barycentric_coord(_v0, _v1, _v2, vector4f_t(x, y));
            if (is_inside) {
                framebuffer.pixel(x, y, _color);
            }
        }
    }
    return;
}
