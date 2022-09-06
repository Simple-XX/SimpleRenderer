
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

void draw2d_t::triangle(const vector2i_t &_v0, const vector2i_t &_v1,
                        const vector2i_t             &_v2,
                        const framebuffer_t::color_t &_color) {
    line(_v0.x, _v0.y, _v1.x, _v1.y, _color);
    line(_v1.x, _v1.y, _v2.x, _v2.y, _color);
    line(_v2.x, _v2.y, _v0.x, _v0.y, _color);
    return;
}
