
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

shader_vertex_in_t::shader_vertex_in_t(void) {
    return;
}

shader_vertex_in_t::shader_vertex_in_t(
  const shader_vertex_in_t& _shader_vertex_in) {
    face = _shader_vertex_in.face;
    return;
}

shader_vertex_in_t::shader_vertex_in_t(const model_t::face_t _face) {
    face = _face;
    return;
}

shader_vertex_in_t::~shader_vertex_in_t(void) {
    return;
}

shader_vertex_in_t&
shader_vertex_in_t::operator=(const shader_vertex_in_t& _shader_vertex_in) {
    if (this == &_shader_vertex_in) {
        throw std::runtime_error(log("this == &_shader_vertex_in"));
    }
    face = _shader_vertex_in.face;
    return *this;
}

shader_vertex_out_t::shader_vertex_out_t(void) {
    return;
}

shader_vertex_out_t::shader_vertex_out_t(const model_t::face_t _face) {
    face = _face;
    return;
}

shader_vertex_out_t::shader_vertex_out_t(
  const shader_vertex_out_t& _shader_vertex_out) {
    face = _shader_vertex_out.face;
    return;
}

shader_vertex_out_t::~shader_vertex_out_t(void) {
    return;
}

shader_vertex_out_t&
shader_vertex_out_t::operator=(const shader_vertex_out_t& _shader_vertex_out) {
    if (this == &_shader_vertex_out) {
        throw std::runtime_error(log("this == &_shader_vertex_out"));
    }
    face = _shader_vertex_out.face;
    return *this;
}

shader_fragment_in_t::shader_fragment_in_t(void) {
    return;
}

shader_fragment_in_t::shader_fragment_in_t(
  const shader_fragment_in_t& _shader_fragment_in) {
    barycentric_coord = _shader_fragment_in.barycentric_coord;
    color             = _shader_fragment_in.color;
    light_dir         = _shader_fragment_in.light_dir;
    return;
}

shader_fragment_in_t::~shader_fragment_in_t(void) {
    return;
}

shader_fragment_in_t& shader_fragment_in_t::operator=(
  const shader_fragment_in_t& _shader_fragment_in) {
    if (this == &_shader_fragment_in) {
        throw std::runtime_error(log("this == &_shader_fragment_in"));
    }
    barycentric_coord = _shader_fragment_in.barycentric_coord;
    color             = _shader_fragment_in.color;
    light_dir         = _shader_fragment_in.light_dir;
    return *this;
}

shader_fragment_out_t::shader_fragment_out_t(void) {
    return;
}

shader_fragment_out_t::shader_fragment_out_t(
  const shader_fragment_out_t& _shader_fragment_out) {
    is_need_draw = _shader_fragment_out.is_need_draw;
    return;
}

shader_fragment_out_t::~shader_fragment_out_t(void) {
    return;
}

shader_fragment_out_t& shader_fragment_out_t::operator=(
  const shader_fragment_out_t& _shader_fragment_out) {
    if (this == &_shader_fragment_out) {
        throw std::runtime_error(log("this == &_shader_fragment_out"));
    }
    is_need_draw = _shader_fragment_out.is_need_draw;
    return *this;
}

shader_data_t::shader_data_t(void) {
    return;
}

shader_data_t::shader_data_t(const shader_data_t& _shader_data) {
    model_matrix   = _shader_data.model_matrix;
    view_matrix    = _shader_data.view_matrix;
    project_matrix = _shader_data.project_matrix;

    return;
}

shader_data_t::~shader_data_t(void) {
    return;
}

shader_data_t& shader_data_t::operator=(const shader_data_t& _shader_data) {
    if (this == &_shader_data) {
        throw std::runtime_error(log("this == &_shader_data"));
    }
    model_matrix   = _shader_data.model_matrix;
    view_matrix    = _shader_data.view_matrix;
    project_matrix = _shader_data.project_matrix;

    return *this;
}

shader_base_t::shader_base_t(void) {
    return;
}

shader_base_t::~shader_base_t(void) {
    return;
}
