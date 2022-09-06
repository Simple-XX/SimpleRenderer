
#include "draw2d.h"

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

draw2d_t::draw2d_t(framebuffer_t &_framebuffer) : framebuffer(_framebuffer) {
    width  = framebuffer.get_width();
    height = framebuffer.get_height();
    return;
}

draw2d_t::~draw2d_t(void) {
    return;
}

void draw2d_t::line(const uint32_t _x0, const uint32_t _y0, const uint32_t _x1,
                    const uint32_t _y1, const framebuffer_t::color_t &_color) {
    assert(_x0 < width);
    assert(_x1 < width);
    assert(_y0 < height);
    assert(_y0 < height);
    auto p0_x = _x0;
    auto p0_y = _y0;
    auto p1_x = _x1;
    auto p1_y = _y1;

    // 如果只是一个点的话
    if (p0_x == p1_x && p0_y == p1_y) {
        framebuffer.pixel(p0_x, p0_y, _color);
    }
    else if (p0_x == p1_x) {
        auto inc = (p0_y <= p1_y) ? 1 : -1;
        for (auto y = p0_y; y != p1_y; y += inc) {
            framebuffer.pixel(p0_x, y, _color);
        }
        framebuffer.pixel(p1_x, p1_y, _color);
    }
    else if (p0_y == p1_y) {
        auto inc = (p0_x <= p1_x) ? 1 : -1;
        for (auto x = p0_x; x != p1_x; x += inc) {
            framebuffer.pixel(x, p0_y, _color);
        }
        framebuffer.pixel(p1_x, p1_y, _color);
    }
    else {
        auto dx  = (p0_x < p1_x) ? p1_x - p0_x : p0_x - p1_x;
        auto dy  = (p0_y < p1_y) ? p1_y - p0_y : p0_y - p1_y;
        auto rem = 0;
        if (dx >= dy) {
            if (p1_x < p0_x) {
                std::swap(p0_x, p1_x);
                std::swap(p0_y, p1_y);
            }
            for (auto x = p0_x, y = p0_y; x <= p1_x; x++) {
                framebuffer.pixel(x, y, _color);
                rem += dy;
                if (rem >= dx) {
                    rem -= dx;
                    y += (p1_y >= p0_y) ? 1 : -1; // 模糊接近这个直线
                    framebuffer.pixel(x, y, _color);
                }
            }
            framebuffer.pixel(p1_x, p1_y, _color);
        }
        else {
            if (p1_y < p0_y) {
                std::swap(p0_x, p1_x);
                std::swap(p0_y, p1_y);
            }
            for (auto x = p0_x, y = p0_y; y <= p1_y; y++) {
                framebuffer.pixel(x, y, _color);
                rem += dx;
                if (rem >= dy) {
                    rem -= dy;
                    x += (p1_x >= p0_x) ? 1 : -1;
                    framebuffer.pixel(x, y, _color);
                }
            }
            framebuffer.pixel(p1_x, p1_y, _color);
        }
    }

    return;
}

void draw2d_t::triangle(const vector2i_t &_v0, const vector2i_t &_v1,
                        const vector2i_t             &_v2,
                        const framebuffer_t::color_t &_color) {
    //    line(_v0.x, _v0.y, _v1.x, _v1.y, _color);
    //    line(_v1.x, _v1.y, _v2.x, _v2.y, _color);
    line(_v2.x, _v2.y, _v0.x, _v0.y, _color);
    return;
}
