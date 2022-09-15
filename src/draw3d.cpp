
/**
 * @file draw3d.cpp
 * @brief 三维绘制
 * @author Zone.N (Zone.Niuzh@hotmail.com)
 * @version 1.0
 * @date 2022-09-14
 * @copyright MIT LICENSE
 * https://github.com/Simple-XX/SimpleRenderer
 * @par change log:
 * <table>
 * <tr><th>Date<th>Author<th>Description
 * <tr><td>2022-09-14<td>Zone.N<td>创建文件
 * </table>
 */

#include "cmath"
#include "draw3d.h"

template <class _T>
const std::pair<bool, vector3f_t> draw3d_t::get_barycentric_coord(
    const vector3_t<_T> &_p0, const vector3_t<_T> &_p1,
    const vector3_t<_T> &_p2, const vector3_t<_T> &_p) {
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
    auto v = ((p0p0 * p1p2) - (p0p1 * p0p2)) * deno;
    auto w = 1 - u - v;

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

    return std::pair<bool, vector3f_t>(res, vector3f_t(u, v, w));
}

draw3d_t::draw3d_t(framebuffer_t &_framebuffer) : framebuffer(_framebuffer) {
    width  = framebuffer.get_width();
    height = framebuffer.get_height();
    return;
}

draw3d_t::~draw3d_t(void) {
    return;
}

void draw3d_t::triangle(const vector3i_t &_v0, const vector3i_t &_v1,
                        const vector3i_t             &_v2,
                        const framebuffer_t::color_t &_color) {
    auto min = _v0.min(_v1).min(_v2);
    auto max = _v0.max(_v1).max(_v2);
    for (auto x = min.x; x <= max.x; x++) {
        for (auto y = min.y; y <= max.y; y++) {
            auto [is_inside, barycentric_coord] =
                get_barycentric_coord(_v0, _v1, _v2, vector3i_t(x, y, 0));
            auto z = 0.;
            z += _v0.z * barycentric_coord.x;
            z += _v1.z * barycentric_coord.y;
            z += _v2.z * barycentric_coord.z;
            if (framebuffer.get_depth_buffer()[y * width + x] < z) {
                if (is_inside) {
                    framebuffer.pixel(x, y, _color, z);
                }
            }
        }
    }
    return;
}
