
/**
 * @file shader_base.cpp
 * @brief 着色器基类实现
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

#include "shader_base.h"

namespace simple_renderer {

ShaderVertexIn::ShaderVertexIn(const Face &face) : face_(face) {}

ShaderVertexOut::ShaderVertexOut(const Face &face) : face_(face) {}

ShaderFragmentIn::ShaderFragmentIn(const Vector3f &barycentric_coord,
                                   const Face &face, const Vector3f &light)
    : barycentric_coord_(barycentric_coord),
      face_(face),
      light_(light) {}  // TODO: normal calculation in the Vertex SHader

ShaderFragmentOut::ShaderFragmentOut(const Color &color) : color_(color) {}

ShaderData::ShaderData(const Matrix4f &model_matrix,
                       const Matrix4f &view_matrix,
                       const Matrix4f &project_matrix)
    : model_matrix_(model_matrix),
      view_matrix_(view_matrix),
      project_matrix_(project_matrix) {}

}  // namespace simple_renderer
