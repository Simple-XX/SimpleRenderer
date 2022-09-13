
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

template <class _T>
bool draw2d_t::is_inside(const vector2_t<_T> &_p0, const vector2_t<_T> &_p1,
                         const vector2_t<_T> &_p2, const vector2_t<_T> &_p) {
    // 边向量
    auto edge_p2p0 = (_p2 - _p0);
    auto edge_p1p0 = (_p1 - _p0);
    // 到点 _p 的向量
    auto edge_pp0 = (_p - _p0);

    float p0p0 = edge_p2p0 * edge_p2p0;
    float p0p1 = edge_p2p0 * edge_p1p0;
    float p0p2 = edge_p2p0 * edge_pp0;
    float p1p1 = edge_p1p0 * edge_p1p0;
    float p1p2 = edge_p1p0 * edge_pp0;

    auto deno = 1. / ((p0p0 * p1p1) - (p0p1 * p0p1));

    auto u = ((p1p1 * p0p2) - (p0p1 * p1p2)) * deno;
    if (u < 0. || u > 1.) {
        return false;
    }

    auto v = ((p0p0 * p1p2) - (p0p1 * p0p2)) * deno;
    if (v < 0. || v > 1.) {
        return false;
    }

    // 重心坐标的分量和为 1
    if (u + v >= 1.) {
        return false;
    }

    return true;
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
    assert(_x0 >= 0);
    assert(_x1 >= 0);
    assert(_y0 >= 0);
    assert(_y1 >= 0);
    assert(_x0 < width);
    assert(_x1 < width);
    assert(_y0 < height);
    assert(_y0 < height);

    auto p0_x = _x0;
    auto p0_y = _y0;
    auto p1_x = _x1;
    auto p1_y = _y1;

    auto steep = false;
    if (std::abs((signed)p0_x - (signed)p1_x) <
        std::abs((signed)p0_y - (signed)p1_y)) {
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
    auto dy2 = std::abs((signed)p1_y - (signed)p0_y) << 1;
    auto dx2 = std::abs((signed)p1_x - (signed)p0_x) << 1;
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
        de += std::abs((signed)dy2);
        if (de >= dx2) {
            y += yi;
            de -= dx2;
        }
    }

    return;
}

void draw2d_t::line(const vector2i_t &_p0, const vector2i_t &_p1,
                    const framebuffer_t::color_t &_color) {
    line(_p0.x, _p0.y, _p1.x, _p1.y, _color);
    return;
}

void draw2d_t::triangle(const vector2i_t &_v0, const vector2i_t &_v1,
                        const vector2i_t             &_v2,
                        const framebuffer_t::color_t &_color) {
    vector2i_t min = _v0.min(_v1).min(_v2);
    vector2i_t max = _v0.max(_v1).max(_v2);
    for (auto x = min.x; x <= max.x; x++) {
        for (auto y = min.y; y <= max.y; y++) {
            if (is_inside(_v0, _v1, _v2, vector2i_t(x, y)) == true) {
                framebuffer.pixel(x, y, _color);
            }
        }
    }
    return;
    return;
}
