#include "shader.hpp"

namespace simple_renderer {

Vertex Shader::VertexShader(const Vertex& vertex) const {
  Matrix4f model_matrix = uniformbuffer_.GetUniform<Matrix4f>("model_matrix");
  Matrix4f view_matrix = uniformbuffer_.GetUniform<Matrix4f>("view_matrix");
  Matrix4f projection_matrix =
      uniformbuffer_.GetUniform<Matrix4f>("projection_matrix");

  Matrix4f mvp_matrix = projection_matrix * view_matrix * model_matrix;
  // auto normal_matrix = model_matrix.inverse().transpose();

  return mvp_matrix * vertex;
}

Color Shader::FragmentShader(const Fragment& fragment) const {
  // interpolate Normal, Color and UV
  Color interpolateColor = fragment.color;
  Vector3f normal = fragment.normal;
  Vector2f uv = fragment.uv;

  // uniform
  Light light = uniformbuffer_.GetUniform<Light>("light");
  Material material = uniformbuffer_.GetUniform<Material>("material");

  // 只绘制正面，背面intensity为负 = 0
  auto intensity = std::max(glm::dot(normal, light.dir), 0.0f);
  // texture color
  Color ambient_color, diffuse_color;
  if (material.has_ambient_texture) {
    Color texture_color = SampleTexture(material.ambient_texture, uv);
    ambient_color = texture_color * intensity;
  } else {
    ambient_color = interpolateColor * intensity;
  }

  if (material.has_diffuse_texture) {
    Color texture_color = SampleTexture(material.diffuse_texture, uv);
    diffuse_color = texture_color * intensity;
  } else {
    diffuse_color = interpolateColor * intensity;
  }
  return ambient_color * 0.1f + diffuse_color;
}

// 将浮点数转换为 uint8_t
// 对浮点数进行四舍五入
uint8_t FloatToUint8_t(float val) {
  float adjusted_val = val > 0.0f ? val + 0.5f : val - 0.5f;
  return static_cast<uint8_t>(adjusted_val);
}

// SampleTexture
// 纹理采样
Color Shader::SampleTexture(const Texture& texture, const Vector2f& uv) const {
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
  return texture.GetPixel(x, y);
}

}  // namespace simple_renderer