#include "shader.hpp"

#include <algorithm>
#include <cmath>
#include <mutex>
#include <shared_mutex>

namespace simple_renderer {

Shader::Shader(const Shader& shader) {
  std::shared_lock lock(shader.specular_cache_mutex_);
  uniformbuffer_ = shader.uniformbuffer_;
  sharedDataInShader_ = shader.sharedDataInShader_;
  vertex_uniform_cache_ = shader.vertex_uniform_cache_;
  fragment_uniform_cache_ = shader.fragment_uniform_cache_;
  specular_lut_cache_ = shader.specular_lut_cache_;
}

Shader::Shader(Shader&& shader) noexcept {
  std::unique_lock lock(shader.specular_cache_mutex_);
  uniformbuffer_ = std::move(shader.uniformbuffer_);
  sharedDataInShader_ = shader.sharedDataInShader_;
  vertex_uniform_cache_ = shader.vertex_uniform_cache_;
  fragment_uniform_cache_ = shader.fragment_uniform_cache_;
  specular_lut_cache_ = std::move(shader.specular_lut_cache_);
}

auto Shader::operator=(const Shader& shader) -> Shader& {
  if (this == &shader) {
    return *this;
  }
  std::shared_lock lock(shader.specular_cache_mutex_);
  uniformbuffer_ = shader.uniformbuffer_;
  sharedDataInShader_ = shader.sharedDataInShader_;
  vertex_uniform_cache_ = shader.vertex_uniform_cache_;
  fragment_uniform_cache_ = shader.fragment_uniform_cache_;
  specular_lut_cache_ = shader.specular_lut_cache_;
  return *this;
}

auto Shader::operator=(Shader&& shader) noexcept -> Shader& {
  if (this == &shader) {
    return *this;
  }
  std::unique_lock lock(shader.specular_cache_mutex_);
  uniformbuffer_ = std::move(shader.uniformbuffer_);
  sharedDataInShader_ = shader.sharedDataInShader_;
  vertex_uniform_cache_ = shader.vertex_uniform_cache_;
  fragment_uniform_cache_ = shader.fragment_uniform_cache_;
  specular_lut_cache_ = std::move(shader.specular_lut_cache_);
  return *this;
}

Vertex Shader::VertexShader(const Vertex& vertex) {
  const bool cache_ready = vertex_uniform_cache_.derived_valid;

  const Matrix4f* model_ptr = nullptr;
  const Matrix4f* mvp_ptr = nullptr;
  const Matrix3f* normal_ptr = nullptr;

  Matrix4f fallback_model;
  Matrix4f fallback_mvp;
  Matrix3f fallback_normal;

  if (cache_ready) { // 如果所有派生矩阵已预计算并可直接复用
    // 直接复用缓存矩阵，避免逐顶点哈希查询
    model_ptr = &vertex_uniform_cache_.model;
    mvp_ptr = &vertex_uniform_cache_.mvp;
    normal_ptr = &vertex_uniform_cache_.normal;
  } else { // 如果缓存尚未建立
    fallback_model = uniformbuffer_.GetUniform<Matrix4f>("modelMatrix");
    Matrix4f view_matrix = uniformbuffer_.GetUniform<Matrix4f>("viewMatrix");
    Matrix4f projection_matrix =
        uniformbuffer_.GetUniform<Matrix4f>("projectionMatrix");
    fallback_mvp = projection_matrix * view_matrix * fallback_model;
    fallback_normal =
        glm::transpose(glm::inverse(Matrix3f(fallback_model)));
    model_ptr = &fallback_model;
    mvp_ptr = &fallback_mvp;
    normal_ptr = &fallback_normal;
  }

  const Matrix4f& model_matrix = *model_ptr;
  const Matrix4f& mvp_matrix = *mvp_ptr;
  const Matrix3f& normal_matrix = *normal_ptr;

  const Vector4f position = vertex.GetPosition();
  Vector4f world_position = model_matrix * position;
  Vector3f transformed_normal = normal_matrix * vertex.GetNormal();

  // 将世界空间位置写入共享数据供片元阶段使用
  sharedDataInShader_.fragPos_varying = Vector3f(world_position);

  // 计算裁剪空间坐标
  Vector4f clip_position = mvp_matrix * position;

  // 返回变换后的顶点（包含变换后的法向量和裁剪坐标）
  return Vertex(clip_position, transformed_normal, vertex.GetTexCoords(),
                vertex.GetColor(),
                clip_position);  // 同时保存裁剪空间坐标用于后续裁剪
}

void Shader::UpdateMatrixCache(const std::string& name,
                               const Matrix4f& value) {
  if (name == "modelMatrix") {
    vertex_uniform_cache_.model = value;
    vertex_uniform_cache_.has_model = true;
  } else if (name == "viewMatrix") {
    vertex_uniform_cache_.view = value;
    vertex_uniform_cache_.has_view = true;
  } else if (name == "projectionMatrix") {
    vertex_uniform_cache_.projection = value;
    vertex_uniform_cache_.has_projection = true;
  } else {
    return;
  }

  // 任一基础矩阵更新后，标记派生矩阵失效等待重算
  vertex_uniform_cache_.derived_valid = false;
  if (vertex_uniform_cache_.has_model && vertex_uniform_cache_.has_view &&
      vertex_uniform_cache_.has_projection) {
    RecalculateDerivedMatrices();
  }
}

void Shader::RecalculateDerivedMatrices() {
  // 预计算 Model-View、MVP 以及法线矩阵，供顶点着色器复用
  vertex_uniform_cache_.model_view =
      vertex_uniform_cache_.view * vertex_uniform_cache_.model;
  vertex_uniform_cache_.mvp = vertex_uniform_cache_.projection *
                              vertex_uniform_cache_.model_view;
  vertex_uniform_cache_.normal = glm::transpose(glm::inverse(
      Matrix3f(vertex_uniform_cache_.model)));
  vertex_uniform_cache_.derived_valid = true;
}

void Shader::UpdateFragmentCache(const std::string& name,
                                 const Light& value) {
  if (name != "light") {
    return;
  }
  fragment_uniform_cache_.light = value;
  fragment_uniform_cache_.has_light = true;
  fragment_uniform_cache_.derived_valid = false;
  if (fragment_uniform_cache_.has_light && fragment_uniform_cache_.has_camera) {
    RecalculateFragmentDerived();
  }
}

void Shader::UpdateFragmentCache(const std::string& name,
                                 const Vector3f& value) {
  if (name != "cameraPos") {
    return;
  }
  fragment_uniform_cache_.camera_pos = value;
  fragment_uniform_cache_.has_camera = true;
  fragment_uniform_cache_.derived_valid = false;
  if (fragment_uniform_cache_.has_light && fragment_uniform_cache_.has_camera) {
    RecalculateFragmentDerived();
  }
}

void Shader::RecalculateFragmentDerived() {
  fragment_uniform_cache_.light_dir_normalized =
      glm::normalize(fragment_uniform_cache_.light.dir);
  fragment_uniform_cache_.derived_valid = true;
}

void Shader::PrepareUniformCaches() {
  PrepareVertexUniformCache();
  PrepareFragmentUniformCache();
}

void Shader::PrepareVertexUniformCache() {
  if (vertex_uniform_cache_.derived_valid) {
    return;
  }
  // 在进入渲染阶段前一次性取出常用矩阵并填充缓存
  if (uniformbuffer_.HasUniform<Matrix4f>("modelMatrix") &&
      uniformbuffer_.HasUniform<Matrix4f>("viewMatrix") &&
      uniformbuffer_.HasUniform<Matrix4f>("projectionMatrix")) {
    vertex_uniform_cache_.model =
        uniformbuffer_.GetUniform<Matrix4f>("modelMatrix");
    vertex_uniform_cache_.view =
        uniformbuffer_.GetUniform<Matrix4f>("viewMatrix");
    vertex_uniform_cache_.projection =
        uniformbuffer_.GetUniform<Matrix4f>("projectionMatrix");
    vertex_uniform_cache_.has_model = true;
    vertex_uniform_cache_.has_view = true;
    vertex_uniform_cache_.has_projection = true;
    RecalculateDerivedMatrices();
  }
}

void Shader::PrepareFragmentUniformCache() {
  if (fragment_uniform_cache_.derived_valid) {
    return;
  }
  if (uniformbuffer_.HasUniform<Light>("light") &&
      uniformbuffer_.HasUniform<Vector3f>("cameraPos")) {
    fragment_uniform_cache_.light =
        uniformbuffer_.GetUniform<Light>("light");
    fragment_uniform_cache_.camera_pos =
        uniformbuffer_.GetUniform<Vector3f>("cameraPos");
    fragment_uniform_cache_.has_light = true;
    fragment_uniform_cache_.has_camera = true;
    RecalculateFragmentDerived();
  }
}

auto Shader::BuildSpecularLUT(float shininess) const -> SpecularLUT {
  SpecularLUT lut;
  if (shininess <= 0.0f) {
    lut.values.fill(1.0f);
    return lut;
  }

  for (size_t i = 0; i < kSpecularLutResolution; ++i) {
    float cos_theta = static_cast<float>(i) /
                      static_cast<float>(kSpecularLutResolution - 1);
    lut.values[i] = cos_theta <= 0.0f ? 0.0f : std::pow(cos_theta, shininess);
  }
  return lut;
}

auto Shader::GetSpecularLUT(float shininess) const -> const SpecularLUT& {
  uint32_t key = std::bit_cast<uint32_t>(shininess);
  {
    std::shared_lock lock(specular_cache_mutex_);
    auto it = specular_lut_cache_.find(key);
    if (it != specular_lut_cache_.end()) {
      return it->second;
    }
  }

  SpecularLUT lut = BuildSpecularLUT(shininess);
  std::unique_lock lock(specular_cache_mutex_);
  auto [it, inserted] = specular_lut_cache_.emplace(key, std::move(lut));
  return it->second;
}

auto Shader::EvaluateSpecular(float cos_theta, float shininess) const -> float {
  cos_theta = std::clamp(cos_theta, 0.0f, 1.0f);
  if (shininess <= 0.0f) {
    return 1.0f;
  }
  if (cos_theta <= 0.0f) {
    return 0.0f;
  }

  const auto& lut = GetSpecularLUT(shininess);
  float scaled = cos_theta * static_cast<float>(kSpecularLutResolution - 1);
  size_t index = static_cast<size_t>(scaled);
  float frac = scaled - static_cast<float>(index);

  const float v0 = lut.values[index];
  const float v1 = lut.values[std::min(index + 1, kSpecularLutResolution - 1)];
  return v0 + (v1 - v0) * frac;
}

Color Shader::FragmentShader(const Fragment& fragment) const {
  // interpolate Normal, Color and UV
  Color interpolateColor = fragment.color;
  Vector3f normal = glm::normalize(fragment.normal);
  Vector2f uv = fragment.uv;

  // uniform
  Light light;
  Vector3f light_dir;
  Vector3f camera_pos;
  if (fragment_uniform_cache_.derived_valid) {
    light = fragment_uniform_cache_.light;
    light_dir = fragment_uniform_cache_.light_dir_normalized;
    camera_pos = fragment_uniform_cache_.camera_pos;
  } else {
    light = uniformbuffer_.GetUniform<Light>("light");
    camera_pos = uniformbuffer_.GetUniform<Vector3f>("cameraPos");
    light_dir = glm::normalize(light.dir);
  }
  Material material = *fragment.material;

  // view direction
  Vector3f view_dir =
      glm::normalize(sharedDataInShader_.fragPos_varying - camera_pos);

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
  float cos_theta = std::max(glm::dot(normal, halfVector), 0.0f);
  float spec = EvaluateSpecular(cos_theta, material.shininess);
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
