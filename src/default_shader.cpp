
/**
 * @file default_shader.cpp
 * @brief 默认着色器实现
 * @author Zone.N (Zone.Niuzh@hotmail.com)
 * @version 1.0
 * @date 2022-11-01
 * @copyright MIT LICENSE
 * https://github.com/Simple-XX/SimpleRenderer
 * @par change log:
 * <table>
 * <tr><th>Date<th>Author<th>Description
 * <tr><td>2022-11-01<td>Zone.N<td>创建文件
 * </table>
 */

#include "default_shader.h"

color_t
default_shader_t::interpolate_color(const color_t&    _color0,
                                    const color_t&    _color1,
                                    const color_t&    _color2,
                                    const vector4f_t& _barycentric_coord) {
    return color_t(
      static_cast<uint8_t>(static_cast<float>(_color0[color_t::COLOR_IDX_R])
                             * _barycentric_coord.x
                           + static_cast<float>(_color1[color_t::COLOR_IDX_R])
                               * _barycentric_coord.y
                           + static_cast<float>(_color2[color_t::COLOR_IDX_R])
                               * _barycentric_coord.z),
      static_cast<uint8_t>(static_cast<float>(_color0[color_t::COLOR_IDX_G])
                             * _barycentric_coord.x
                           + static_cast<float>(_color1[color_t::COLOR_IDX_G])
                               * _barycentric_coord.y
                           + static_cast<float>(_color2[color_t::COLOR_IDX_G])
                               * _barycentric_coord.z),
      static_cast<uint8_t>(static_cast<float>(_color0[color_t::COLOR_IDX_B])
                             * _barycentric_coord.x
                           + static_cast<float>(_color1[color_t::COLOR_IDX_B])
                               * _barycentric_coord.y
                           + static_cast<float>(_color2[color_t::COLOR_IDX_B])
                               * _barycentric_coord.z));
}

default_shader_t::default_shader_t(void) {
    return;
}

default_shader_t::~default_shader_t(void) {
    return;
}

/// @todo 巨大性能开销
shader_vertex_out_t
default_shader_t::vertex(const shader_vertex_in_t& _shader_vertex_in) const {
    /// @todo 处理变换
    auto face(_shader_vertex_in.face);
    // 变换坐标
    /// @todo 这里的问题在于在 m 矩阵中做了 t 操作，然后再应用 v 矩阵时，会在 t
    /// 的基础上 r
    auto aaa = matrix4f_t().translate(-960, -540, 0);
    auto bbb = matrix4f_t().translate(960, 540, 0);
    auto mvp = shader_data.project_matrix * bbb * shader_data.view_matrix * aaa
             * shader_data.model_matrix;
    // auto mvp = shader_data.project_matrix * shader_data.view_matrix
    //          * shader_data.model_matrix;

    face.v0.coord = mvp * face.v0.coord;
    face.v1.coord = mvp * face.v1.coord;
    face.v2.coord = mvp * face.v2.coord;
    /// @todo 通过矩阵变换法线
    auto v2v0     = face.v2.coord - face.v0.coord;
    auto v1v0     = face.v1.coord - face.v0.coord;
    face.normal   = (v2v0 ^ v1v0).normalize();
    /// @todo 变换贴图
    return shader_vertex_out_t(face);
}

shader_fragment_out_t default_shader_t::fragment(
  const shader_fragment_in_t& _shader_fragment_in) const {
    auto intensity = (_shader_fragment_in.normal * _shader_fragment_in.light);
    auto is_need_draw = true;
    // 光照方向为正，不绘制背面
    if (intensity <= 0) {
        is_need_draw = false;
        return shader_fragment_out_t(is_need_draw, color_t());
    }
    auto color = interpolate_color(_shader_fragment_in.color0,
                                   _shader_fragment_in.color1,
                                   _shader_fragment_in.color2,
                                   _shader_fragment_in.barycentric_coord)
               * intensity;
    return shader_fragment_out_t(is_need_draw, color);
}
