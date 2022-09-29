
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

const std::pair<bool, vector4f_t>
draw3d_t::get_barycentric_coord(const vector4f_t &_p0, const vector4f_t &_p1,
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

draw3d_t::draw3d_t(std::shared_ptr<framebuffer_t> _framebuffer)
    : framebuffer(_framebuffer) {
    width  = framebuffer->get_width();
    height = framebuffer->get_height();
    return;
}

draw3d_t::~draw3d_t(void) {
    return;
}

void draw3d_t::triangle(const vector4f_t &_v0, const vector4f_t &_v1,
                        const vector4f_t             &_v2,
                        const framebuffer_t::color_t &_color) {
    // 计算最小值
    auto min = _v0.min(_v1).min(_v2);
    // 计算最大值
    auto max = _v0.max(_v1).max(_v2);
    // 遍历区域中的每个点 p
    for (auto x = int32_t(min.x); x <= max.x; x++) {
        for (auto y = int32_t(min.y); y <= max.y; y++) {
            // 判断点 p 是否在三角形内
            auto [is_inside, barycentric_coord] =
                get_barycentric_coord(_v0, _v1, _v2, vector4f_t(x, y, 0));
            // 计算深度，深度为三个点的 z 值矢量和
            auto z = 0.;
            z += _v0.z * barycentric_coord.x;
            z += _v1.z * barycentric_coord.y;
            z += _v2.z * barycentric_coord.z;
            // 深度在已有像素之上
            if (z >= (framebuffer->get_depth_buffer()(x, y))) {
                // 在内部
                if (is_inside) {
                    // 计算面的法向量
                    auto v2v0   = _v2 - _v0;
                    auto v1v0   = _v1 - _v0;
                    auto normal = v2v0 ^ v1v0;
                    normal      = normal.normalize();
                    std::cout << "normal: " << normal << std::endl;
                    auto intensity = normal * vector4f_t(0, 0, -1);
                    std::cout << "intensity: " << intensity << std::endl;
                    if (intensity > 0) {
                        framebuffer->pixel(x, y, _color * intensity, z);
                    }
                }
            }
        }
    }
    return;
}

void draw3d_t::triangle(const model_t::vertex_t &_v0,
                        const model_t::vertex_t &_v1,
                        const model_t::vertex_t &_v2,
                        const model_t::normal_t &_normal) {
    auto min = _v0.coord.min(_v1.coord).min(_v2.coord);
    auto max = _v0.coord.max(_v1.coord).max(_v2.coord);
    for (auto x = int32_t(min.x); x <= max.x; x++) {
        for (auto y = int32_t(min.y); y <= max.y; y++) {
            auto [is_inside, barycentric_coord] = get_barycentric_coord(
                _v0.coord, _v1.coord, _v2.coord, vector4f_t(x, y, 0));
            // 如果点在三角形内再进行下一步
            if (is_inside == true) {
                // 计算该点的深度，通过重心坐标插值计算
                auto z = 0.;
                z += _v0.coord.z * barycentric_coord.x;
                z += _v1.coord.z * barycentric_coord.y;
                z += _v2.coord.z * barycentric_coord.z;
                // 深度在已有颜色之上
                if (z >= (framebuffer->get_depth_buffer()(x, y))) {
                    // 光照方向为正，不绘制背面
                    if (_normal * vector4f_t(0, 0, -1) > 0) {
                        // 计算颜色，颜色为三个点的颜色的重心坐标插值
                        model_t::color_t       color_v;
                        framebuffer_t::color_t color = 0xFFFFFFFF;
                        color_v = (_v0.color * barycentric_coord.x +
                                   _v1.color * barycentric_coord.y +
                                   _v2.color * barycentric_coord.z);
                        std::cout << "color_v: " << color_v << std::endl;
                        color_v = color_v.normalize();
                        std::cout << "color_v nor: " << color_v << std::endl;
                        color = framebuffer_t::ARGB(255, 255 * color_v.x,
                                                    255 * color_v.y,
                                                    255 * color_v.z);
                        printf("color from cal: [0x%X]\n", color);
                        framebuffer->pixel(x, y, color, z);
                    }
                }
            }
        }
    }
    return;
}

void draw3d_t::triangle(const model_t::face_t &_face) {
    triangle(_face.v0, _face.v1, _face.v2, _face.normal);
    return;
}

void draw3d_t::model(const model_t &_model, const matrix4f_t &_tran) {
    for (auto f : _model.get_face()) {
        triangle(f * _tran);
    }
    return;
}
