
/**
 * @file shader.cpp
 * @brief 着色器实现
 * @author Zone.N (Zone.Niuzh@hotmail.com)
 * @version 1.0
 * @date 2022-10-15
 * @copyright MIT LICENSE
 * https://github.com/Simple-XX/SimpleRenderer
 * @par change log:
 * <table>
 * <tr><th>Date<th>Author<th>Description
 * <tr><td>2022-10-15<td>Zone.N<td>创建文件
 * </table>
 */

#include "shader.h"

shader_vertex_in_t::shader_vertex_in_t(const model_t::face_t &_face) {
  face = _face;
}

shader_vertex_out_t::shader_vertex_out_t(const model_t::face_t &_face) {
  face = _face;
  return;
}

shader_fragment_in_t::shader_fragment_in_t(const vector4f_t &_barycentric_coord,
                                           const vector4f_t &_normal,
                                           const vector4f_t &_light,
                                           const color_t &_color0,
                                           const color_t &_color1,
                                           const color_t &_color2)
    : barycentric_coord(_barycentric_coord), normal(_normal), light(_light),
      color0(_color0), color1(_color1), color2(_color2) {
  return;
}

shader_fragment_out_t::shader_fragment_out_t(const bool &_is_need_draw,
                                             const color_t &_color)
    : is_need_draw(_is_need_draw), color(_color) {}

shader_data_t::shader_data_t(const matrix4f_t &_model_matrix,
                             const matrix4f_t &_view_matrix,
                             const matrix4f_t &_project_matrix)
    : model_matrix(_model_matrix), view_matrix(_view_matrix),
      project_matrix(_project_matrix) {}
