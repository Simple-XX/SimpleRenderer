
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
uint8_t DefaultShader::float_to_uint8_t(float val) {
  float adjusted_val = val > 0.0f ? val + 0.5f : val - 0.5f;
  return static_cast<uint8_t>(adjusted_val);
}

// sampleTexture
// 纹理采样
Color DefaultShader::sampleTexture(const Texture &texture, const Vector2f &uv) {
  // wrap u,v to [0. 1]
  // 将 u, v 包装到 [0. 1]
  auto u = uv.x - std::floor(uv.x);
  auto v = uv.y - std::floor(uv.y);

  // convert u,v to pixel space coordinates
  // 将 u, v 转换为像素空间坐标
  auto x = static_cast<int>(u * texture.width);
  auto y = static_cast<int>(v * texture.height);

  // Ensure x, y are within bounds
  // 确保 x, y 在范围内
  x = std::clamp(x, 0, texture.width - 1);
  y = std::clamp(y, 0, texture.height - 1);

  // Get pixel color
  // 获取像素颜色
  return texture.getPixel(x, y);
}

// 插值颜色 (r, g, b)
auto DefaultShader::InterpolateColor(
    const Color &color0, const Color &color1, const Color &color2,
    const Vector3f &barycentric_coord) -> Color {
  auto color_r = float_to_uint8_t(
      static_cast<float>(color0[Color::kColorIndexRed]) * barycentric_coord.x +
      static_cast<float>(color1[Color::kColorIndexRed]) * barycentric_coord.y +
      static_cast<float>(color2[Color::kColorIndexRed]) * barycentric_coord.z);
  auto color_g =
      float_to_uint8_t(static_cast<float>(color0[Color::kColorIndexGreen]) *
                           barycentric_coord.x +
                       static_cast<float>(color1[Color::kColorIndexGreen]) *
                           barycentric_coord.y +
                       static_cast<float>(color2[Color::kColorIndexGreen]) *
                           barycentric_coord.z);
  auto color_b = float_to_uint8_t(
      static_cast<float>(color0[Color::kColorIndexBlue]) * barycentric_coord.x +
      static_cast<float>(color1[Color::kColorIndexBlue]) * barycentric_coord.y +
      static_cast<float>(color2[Color::kColorIndexBlue]) * barycentric_coord.z);
  return Color(color_r, color_g, color_b);
}

// 插值 UV
auto DefaultShader::InterpolateUV(
    const Vector2f &uv0, const Vector2f &uv1, const Vector2f &uv2,
    const Vector3f &barycentric_coord) -> Vector2f {
  auto uv_x = uv0.x * barycentric_coord.x + uv1.x * barycentric_coord.y +
              uv2.x * barycentric_coord.z;
  auto uv_y = uv0.y * barycentric_coord.x + uv1.y * barycentric_coord.y +
              uv2.y * barycentric_coord.z;
  return Vector2f(uv_x, uv_y);
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
  // interpolate Normal, Color and UV
  // 插值法线、颜色和 UV
  Vector3f interpolateNormal =
      shader_fragment_in.face_
          .normal();  // OR barycentric interpolation with vertex normals
                      // 或者使用顶点法线进行重心插值
  Color interpolateColor =
      InterpolateColor(shader_fragment_in.face_.vertex(0).color(),
                       shader_fragment_in.face_.vertex(1).color(),
                       shader_fragment_in.face_.vertex(2).color(),
                       shader_fragment_in.barycentric_coord_);
  Vector2f interpolateUV =
      InterpolateUV(shader_fragment_in.face_.vertex(0).texCoords(),
                    shader_fragment_in.face_.vertex(1).texCoords(),
                    shader_fragment_in.face_.vertex(2).texCoords(),
                    shader_fragment_in.barycentric_coord_);

  // 只绘制正面，背面intensity为负 = 0
  auto intensity =
      std::max(glm::dot(interpolateNormal, shader_fragment_in.light_), 0.0f);
  // texture color
  // 纹理颜色
  Color final_color, ambient_color, diffuse_color, specular_color;
  if (shader_fragment_in.face_.material().has_ambient_texture) {
    Color texture_color = sampleTexture(
        shader_fragment_in.face_.material().ambient_texture, interpolateUV);
    ambient_color = texture_color * intensity;
  } else {
    ambient_color = interpolateColor * intensity;
  }

  if (shader_fragment_in.face_.material().has_diffuse_texture) {
    Color texture_color = sampleTexture(
        shader_fragment_in.face_.material().diffuse_texture, interpolateUV);
    diffuse_color = texture_color * intensity;
  } else {
    diffuse_color = interpolateColor * intensity;
  }

  // TODO: add specular texture <- get view direction <- have a camera system
  // 添加高光纹理 <- 获取视图方向 <- 有一个相机系统
  // TODO: and I want to refactor the shader system
  // 我想重构着色器系统
  final_color = ambient_color * 0.1f + diffuse_color;
  return ShaderFragmentOut(final_color);
}

}  // namespace simple_renderer
