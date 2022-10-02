
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
draw3d_t::get_barycentric_coord(const vector4f_t& _p0, const vector4f_t& _p1,
                                const vector4f_t& _p2, const vector4f_t& _p) {
    auto ab   = _p1 - _p0;
    auto ac   = _p2 - _p0;
    auto ap   = _p - _p0;

    auto deno = (ab.x * ac.y - ab.y * ac.x);
    if (deno == 0) {
        return std::pair<bool, vector4f_t>(false, vector4f_t());
    }

    auto s       = (ac.y * ap.x - ac.x * ap.y) / deno;
    auto t       = (ab.x * ap.y - ab.y * ap.x) / deno;
    auto weights = vector4f_t(1 - s - t, s, t);

    auto res     = ((weights.x <= 1) && (weights.x >= 0))
            && ((weights.y <= 1) && (weights.y >= 0))
            && ((weights.z <= 1) && (weights.z >= 0));

    return std::pair<bool, vector4f_t>(res, weights);
}

void draw3d_t::triangle(const model_t::vertex_t& _v0,
                        const model_t::vertex_t& _v1,
                        const model_t::vertex_t& _v2,
                        const model_t::normal_t& _normal) {
    auto min = _v0.coord.min(_v1.coord).min(_v2.coord);
    auto max = _v0.coord.max(_v1.coord).max(_v2.coord);
    for (auto x = int32_t(min.x); x <= max.x; x++) {
        for (auto y = int32_t(min.y); y <= max.y; y++) {
            auto [is_inside, barycentric_coord]
              = get_barycentric_coord(_v0.coord, _v1.coord, _v2.coord,
                                      vector4f_t(x, y, 0));
            // 如果点在三角形内再进行下一步
            static uint32_t aaa = 0;
            if (is_inside == true) {
                aaa++;
                // 计算该点的深度，通过重心坐标插值计算
                auto z = 0.;
                z      += _v0.coord.z * barycentric_coord.x;
                z      += _v1.coord.z * barycentric_coord.y;
                z      += _v2.coord.z * barycentric_coord.z;
                // 深度在已有颜色之上
                if (z >= (framebuffer->get_depth_buffer()(x, y))) {
                    // 光照方向为正，不绘制背面
                    /// @bug cube3 法线方向可能有问题
                    if (_normal * vector4f_t(0, 0, -1) > 0) {
                        // 计算颜色，颜色为三个点的颜色的重心坐标插值
                        model_t::color_t       color_v;
                        framebuffer_t::color_t color = 0xFFFFFFFF;
                        color_v = (_v0.color * barycentric_coord.x,
                                   _v1.color * barycentric_coord.y,
                                   _v2.color * barycentric_coord.z);
                        // color_v = color_v.normalize();
                        color_v = vector4f_t(1, 1, 1, 1);
                        color   = framebuffer_t::ARGB(255, 255 * color_v.x,
                                                      255 * color_v.y,
                                                      255 * color_v.z);
                        framebuffer->pixel(x, y, color, z);
                    }
                }
            }
        }
    }
    return;
}

void draw3d_t::triangle(const model_t::face_t& _face) {
    triangle(_face.v0, _face.v1, _face.v2, _face.normal);
    return;
}

const matrix4f_t draw3d_t::model2world_tran(const model_t& _model) const {
    // 计算极值
    auto x_max = _model.get_face()[0].v0.coord.x;
    auto x_min = _model.get_face()[0].v0.coord.x;
    auto y_max = _model.get_face()[0].v0.coord.y;
    auto y_min = _model.get_face()[0].v0.coord.y;
    auto z_max = _model.get_face()[0].v0.coord.z;
    auto z_min = _model.get_face()[0].v0.coord.z;
    for (auto& i : _model.get_face()) {
        x_max = std::max(x_max, i.v0.coord.x);
        x_max = std::max(x_max, i.v1.coord.x);
        x_max = std::max(x_max, i.v2.coord.x);
        x_min = std::min(x_min, i.v0.coord.x);
        x_min = std::min(x_min, i.v1.coord.x);
        x_min = std::min(x_min, i.v2.coord.x);

        y_max = std::max(y_max, i.v0.coord.y);
        y_max = std::max(y_max, i.v1.coord.y);
        y_max = std::max(y_max, i.v2.coord.y);
        y_min = std::min(y_min, i.v0.coord.y);
        y_min = std::min(y_min, i.v1.coord.y);
        y_min = std::min(y_min, i.v2.coord.y);

        z_max = std::max(z_max, i.v0.coord.z);
        z_max = std::max(z_max, i.v1.coord.z);
        z_max = std::max(z_max, i.v2.coord.z);
        z_min = std::min(z_min, i.v0.coord.z);
        z_min = std::min(z_min, i.v1.coord.z);
        z_min = std::min(z_min, i.v2.coord.z);
    }

    // 各分量的长度
    auto       delta_x       = (std::abs(x_max) + std::abs(x_min));
    auto       delta_y       = (std::abs(y_max) + std::abs(y_min));
    auto       delta_z       = (std::abs(z_max) + std::abs(z_min));
    auto       delta_xy_max  = std::max(delta_x, delta_y);
    auto       delta_xyz_max = std::max(delta_xy_max, delta_z);

    // 缩放倍数
    auto       multi         = (height + width) / 4;
    // 归一化并乘倍数
    auto       scale         = multi / delta_xyz_max;

    // 构建变换矩阵
    matrix4f_t mat;
    // 缩放
    mat = mat.scale(scale, scale, scale);
    // 移动到左上角
    mat = mat.translate(std::abs(x_min) * scale, std::abs(y_min) * scale, 0);
    return matrix4f_t(mat);
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

void draw3d_t::line(const int32_t _x0, const int32_t _y0, const int32_t _x1,
                    const int32_t _y1, const framebuffer_t::color_t& _color) {
    auto p0_x  = _x0;
    auto p0_y  = _y0;
    auto p1_x  = _x1;
    auto p1_y  = _y1;

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
            framebuffer->pixel(y, x, _color);
        }
        else {
            framebuffer->pixel(x, y, _color);
        }
        de += std::abs(dy2);
        if (de >= dx2) {
            y  += yi;
            de -= dx2;
        }
    }

    return;
}

void draw3d_t::line(const vector4f_t& _p0, const vector4f_t& _p1,
                    const framebuffer_t::color_t& _color) {
    line(_p0.x, _p0.y, _p1.x, _p1.y, _color);
    return;
}

void draw3d_t::triangle2d(const vector4f_t& _v0, const vector4f_t& _v1,
                          const vector4f_t&             _v2,
                          const framebuffer_t::color_t& _color) {
    auto min = _v0.min(_v1).min(_v2);
    auto max = _v0.max(_v1).max(_v2);

    for (auto x = min.x; x <= max.x; x++) {
        for (auto y = min.y; y <= max.y; y++) {
            auto [is_inside, _]
              = get_barycentric_coord(_v0, _v1, _v2, vector4f_t(x, y));
            if (is_inside) {
                framebuffer->pixel(x, y, _color);
            }
        }
    }
    return;
}

void draw3d_t::triangle(const vector4f_t& _v0, const vector4f_t& _v1,
                        const vector4f_t&             _v2,
                        const framebuffer_t::color_t& _color) {
    // 计算最小值
    auto min = _v0.min(_v1).min(_v2);
    // 计算最大值
    auto max = _v0.max(_v1).max(_v2);
    // 遍历区域中的每个点 p
    for (auto x = int32_t(min.x); x <= max.x; x++) {
        for (auto y = int32_t(min.y); y <= max.y; y++) {
            // 判断点 p 是否在三角形内
            auto [is_inside, barycentric_coord]
              = get_barycentric_coord(_v0, _v1, _v2, vector4f_t(x, y, 0));
            // 计算深度，深度为三个点的 z 值矢量和
            auto z = 0.;
            z      += _v0.z * barycentric_coord.x;
            z      += _v1.z * barycentric_coord.y;
            z      += _v2.z * barycentric_coord.z;
            // 深度在已有像素之上
            if (z >= (framebuffer->get_depth_buffer()(x, y))) {
                // 在内部
                if (is_inside) {
                    // 计算面的法向量
                    auto v2v0      = _v2 - _v0;
                    auto v1v0      = _v1 - _v0;
                    auto normal    = v2v0 ^ v1v0;
                    normal         = normal.normalize();
                    // std::cout << "normal: " << normal <<
                    // std::endl;
                    auto intensity = normal * vector4f_t(0, 0, -1);
                    // std::cout << "intensity: " <<
                    // intensity << std::endl;
                    if (intensity > 0) {
                        framebuffer->pixel(x, y, _color * intensity, z);
                    }
                }
            }
        }
    }
    return;
}

void draw3d_t::model(const model_t& _model) {
    auto& tran = model2world_tran(_model);
    std::cout << "tran: " << tran << std::endl;
    for (auto f : _model.get_face()) {
        triangle(f * tran);
    }
    return;
}

void draw3d_t::model(const model_t& _model, const matrix4f_t& _tran) {
    for (auto f : _model.get_face()) {
        triangle(f * _tran);
    }
    return;
}
