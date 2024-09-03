
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

namespace simple_renderer {

// 将浮点数转换为 uint8_t
// 对浮点数进行四舍五入
uint8_t float_to_uint8_t(float val) {
  float adjusted_val = val > 0.0f ? val + 0.5f : val - 0.5f;
  return static_cast<uint8_t>(adjusted_val);
}

auto DefaultShader::InterpolateColor(
    const Color &color0, const Color &color1, const Color &color2,
    const Vector3f &barycentric_coord) -> Color {
  // 插值颜色 (r, g, b)
  auto color_r = float_to_uint8_t(
    static_cast<float>(color0[Color::kColorIndexRed]) * barycentric_coord.x +
    static_cast<float>(color1[Color::kColorIndexRed]) * barycentric_coord.y +
    static_cast<float>(color2[Color::kColorIndexRed]) * barycentric_coord.z);
  auto color_g = float_to_uint8_t(
    static_cast<float>(color0[Color::kColorIndexGreen]) * barycentric_coord.x +
    static_cast<float>(color1[Color::kColorIndexGreen]) * barycentric_coord.y +
    static_cast<float>(color2[Color::kColorIndexGreen]) * barycentric_coord.z);
  auto color_b = float_to_uint8_t(
    static_cast<float>(color0[Color::kColorIndexBlue]) * barycentric_coord.x +
    static_cast<float>(color1[Color::kColorIndexBlue]) * barycentric_coord.y +
    static_cast<float>(color2[Color::kColorIndexBlue]) * barycentric_coord.z);
  return Color(color_r, color_g, color_b);
}

/// @todo 巨大性能开销
auto DefaultShader::Vertex(const ShaderVertexIn &shader_vertex_in) const
    -> ShaderVertexOut {
  auto face(shader_vertex_in.face_);

  face.transform(shader_data_.project_matrix_ * shader_data_.view_matrix_ *
                 shader_data_.model_matrix_);

  /// @todo 变换贴图
  return ShaderVertexOut(face);
}

auto DefaultShader::Fragment(const ShaderFragmentIn &shader_fragment_in) const
    -> ShaderFragmentOut {
  auto intensity =
      glm::dot(shader_fragment_in.normal_, shader_fragment_in.light_);
  auto is_need_draw = true;
  // 光照方向为正，不绘制背面
  if (intensity <= 0) {
    is_need_draw = false;
    return ShaderFragmentOut(is_need_draw, Color());
  }
  intensity *= 255;
  auto color =
      InterpolateColor(shader_fragment_in.color0_, shader_fragment_in.color1_,
                       shader_fragment_in.color2_,
                       shader_fragment_in.barycentric_coord_) *
      intensity;
  return ShaderFragmentOut(is_need_draw, color);
}

}  // namespace simple_renderer
