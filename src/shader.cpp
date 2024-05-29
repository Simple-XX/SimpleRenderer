
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

namespace simple_renderer {

ShaderVertexIn::ShaderVertexIn(const Model::Face &face) : face_(face) {}

ShaderVertexOut::ShaderVertexOut(const Model::Face &face) : face_(face) {}

ShaderFragmentIn::ShaderFragmentIn(const Vector3f &barycentric_coord,
                                   const Vector3f &normal,
                                   const Vector3f &light, const Color &color0,
                                   const Color &color1, const Color &color2)
    : barycentric_coord_(barycentric_coord),
      normal_(normal),
      light_(light),
      color0_(color0),
      color1_(color1),
      color2_(color2) {}

ShaderFragmentOut::ShaderFragmentOut(const bool &is_need_draw,
                                     const Color &color)
    : is_need_draw_(is_need_draw), color_(color) {}

ShaderData::ShaderData(const Matrix4f &model_matrix,
                       const Matrix4f &view_matrix,
                       const Matrix4f &project_matrix)
    : model_matrix_(model_matrix),
      view_matrix_(view_matrix),
      project_matrix_(project_matrix) {}

}  // namespace simple_renderer
