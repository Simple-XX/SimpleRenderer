#include "shader.hpp"

namespace simple_renderer {

Vertex Shader::VertexShader(const Vertex& vertex) {
  Matrix4f model_matrix = uniformbuffer_.GetUniform<Matrix4f>("modelMatrix");
  Matrix4f view_matrix = uniformbuffer_.GetUniform<Matrix4f>("viewMatrix");
  Matrix4f projection_matrix =
      uniformbuffer_.GetUniform<Matrix4f>("projectionMatrix");

  Matrix4f mvp_matrix = projection_matrix * view_matrix * model_matrix;
  // auto normal_matrix = model_matrix.inverse().transpose();

  sharedDataInShader_.fragPos_varying =
      Vector3f(model_matrix * vertex.GetPosition());

  return mvp_matrix * vertex;
}

Color Shader::FragmentShader(const Fragment& fragment) const {
  // interpolate Normal, Color and UV
  Color interpolateColor = fragment.color;
  Vector3f normal = glm::normalize(fragment.normal);
  Vector2f uv = fragment.uv;

  // uniform
  Light light = uniformbuffer_.GetUniform<Light>("light");
  Material material = *fragment.material;

  // view direction
  Vector3f view_dir =
      glm::normalize(sharedDataInShader_.fragPos_varying -
                     uniformbuffer_.GetUniform<Vector3f>("cameraPos"));
  Vector3f light_dir = glm::normalize(light.dir);

  auto intensity = std::max(glm::dot(normal, light_dir), 0.0f);
  // texture color
  Color ambient_color, diffuse_color, specular_color;
  if (material.has_ambient_texture) {
    Color texture_color = SampleTexture(material.ambient_texture, uv);
    ambient_color = texture_color;
  } else {
    ambient_color = interpolateColor;
  }

  if (material.has_diffuse_texture) {
    Color texture_color = SampleTexture(material.diffuse_texture, uv);
    diffuse_color = texture_color * intensity;
  } else {
    diffuse_color = interpolateColor * intensity;
  }

  Vector3f halfVector = glm::normalize(light_dir + view_dir);
  float spec = std::pow(std::max(glm::dot(normal, halfVector), 0.0f),
                        material.shininess);
  if (material.has_specular_texture) {
    Color texture_color = SampleTexture(material.specular_texture, uv);
    specular_color = texture_color * spec;
  } else {
    specular_color = Color(1.0f, 1.0f, 1.0f) * spec;
  }

  return ClampColor(ambient_color * 0.1f + diffuse_color +
                    specular_color * 0.2f);
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

Color Shader::ClampColor(const Color color) const {
  uint8_t red =
      color[Color::kColorIndexRed] > 255 ? 255 : color[Color::kColorIndexRed];
  uint8_t green = color[Color::kColorIndexGreen] > 255
                      ? 255
                      : color[Color::kColorIndexGreen];
  uint8_t blue =
      color[Color::kColorIndexBlue] > 255 ? 255 : color[Color::kColorIndexBlue];
  uint8_t alpha = color[Color::kColorIndexAlpha] > 255
                      ? 255
                      : color[Color::kColorIndexAlpha];
  return Color(red, green, blue, alpha);
}

}  // namespace simple_renderer