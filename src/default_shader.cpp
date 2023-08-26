
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
#include <Eigen/src/Geometry/Scaling.h>
#include <Eigen/src/Geometry/Transform.h>

auto default_shader_t::interpolate_color(const color_t &_color0,
                                         const color_t &_color1,
                                         const color_t &_color2,
                                         const vector3f_t &_barycentric_coord)
    -> color_t {
  return color_t(
      static_cast<uint8_t>(static_cast<float>(_color0[color_t::COLOR_IDX_R]) *
                               _barycentric_coord.x() +
                           static_cast<float>(_color1[color_t::COLOR_IDX_R]) *
                               _barycentric_coord.y() +
                           static_cast<float>(_color2[color_t::COLOR_IDX_R]) *
                               _barycentric_coord.z()),
      static_cast<uint8_t>(static_cast<float>(_color0[color_t::COLOR_IDX_G]) *
                               _barycentric_coord.x() +
                           static_cast<float>(_color1[color_t::COLOR_IDX_G]) *
                               _barycentric_coord.y() +
                           static_cast<float>(_color2[color_t::COLOR_IDX_G]) *
                               _barycentric_coord.z()),
      static_cast<uint8_t>(static_cast<float>(_color0[color_t::COLOR_IDX_B]) *
                               _barycentric_coord.x() +
                           static_cast<float>(_color1[color_t::COLOR_IDX_B]) *
                               _barycentric_coord.y() +
                           static_cast<float>(_color2[color_t::COLOR_IDX_B]) *
                               _barycentric_coord.z()));
}

/// @todo 巨大性能开销
auto default_shader_t::vertex(const shader_vertex_in_t &_shader_vertex_in) const
    -> shader_vertex_out_t {
  /// @todo 处理变换
  auto face(_shader_vertex_in.face);
  // 变换坐标
  /// @todo 这里的问题在于在 m 矩阵中做了 t 操作，然后再应用 v 矩阵时，会在 t
  /// 的基础上 r
  auto mvp = shader_data.project_matrix * shader_data.view_matrix *
             shader_data.model_matrix;

  //  std::cout << "mvp: " << std::endl << shader_data.model_matrix <<
  //  std::endl;
  // auto mvp = shader_data.project_matrix * shader_data.view_matrix
  //          * shader_data.model_matrix;

  auto a0 = vector4f_t();
  a0.head(3) = face.v0.coord;
  a0 = mvp * a0;
  face.v0.coord = a0.head(3);

  auto a1 = vector4f_t();
  a1.head(3) = face.v1.coord;
  a1 = mvp * a1;
  face.v1.coord = a1.head(3);

  auto a2 = vector4f_t();
  a2.head(3) = face.v2.coord;
  a2 = mvp * a2;
  face.v2.coord = a2.head(3);

  //  face.v0.coord = mvp * face.v0.coord;
  //  face.v1.coord = mvp * face.v1.coord;
  //  face.v2.coord = mvp * face.v2.coord;
  /// @todo 通过矩阵变换法线
  auto v2v0 = face.v2.coord - face.v0.coord;
  auto v1v0 = face.v1.coord - face.v0.coord;
  face.normal = (v2v0.cross(v1v0)).normalized();
  /// @todo 变换贴图
  return shader_vertex_out_t(face);
}

auto default_shader_t::fragment(const shader_fragment_in_t &_shader_fragment_in)
    const -> shader_fragment_out_t {
  auto intensity = (_shader_fragment_in.normal.dot(_shader_fragment_in.light));
  auto is_need_draw = true;
  // 光照方向为正，不绘制背面
  if (intensity <= 0) {
    is_need_draw = false;
    return shader_fragment_out_t(is_need_draw, color_t());
  }
  auto color =
      interpolate_color(_shader_fragment_in.color0, _shader_fragment_in.color1,
                        _shader_fragment_in.color2,
                        _shader_fragment_in.barycentric_coord) *
      intensity;
  return shader_fragment_out_t(is_need_draw, color);
}
