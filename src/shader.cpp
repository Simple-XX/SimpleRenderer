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
  if (name != "light") { return; }
  fragment_uniform_cache_.lights.clear();
  fragment_uniform_cache_.lights.push_back(value);
  fragment_uniform_cache_.has_lights = true;
  fragment_uniform_cache_.derived_valid = false;
  if (fragment_uniform_cache_.has_lights && fragment_uniform_cache_.has_camera) {
    RecalculateFragmentDerived();
  }
}

void Shader::UpdateFragmentCache(const std::string& name,
                                 const std::vector<Light>& value) {
  if (name != "lights") { return; }
  fragment_uniform_cache_.lights = value;
  fragment_uniform_cache_.has_lights = true;
  fragment_uniform_cache_.derived_valid = false;
  if (fragment_uniform_cache_.has_lights && fragment_uniform_cache_.has_camera) {
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
  if (fragment_uniform_cache_.has_lights && fragment_uniform_cache_.has_camera) {
    RecalculateFragmentDerived();
  }
}

void Shader::RecalculateFragmentDerived() {
  fragment_uniform_cache_.light_dirs_normalized.clear();
  fragment_uniform_cache_.light_dirs_normalized.reserve(
      fragment_uniform_cache_.lights.size());
  for (const auto& l : fragment_uniform_cache_.lights) {
    fragment_uniform_cache_.light_dirs_normalized.push_back(glm::normalize(l.dir));
  }
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
  // 优先多光源
  if (uniformbuffer_.HasUniform<std::vector<Light>>("lights") &&
      uniformbuffer_.HasUniform<Vector3f>("cameraPos")) {
    fragment_uniform_cache_.lights =
        uniformbuffer_.GetUniform<std::vector<Light>>("lights");
    fragment_uniform_cache_.has_lights = true;
    fragment_uniform_cache_.camera_pos =
        uniformbuffer_.GetUniform<Vector3f>("cameraPos");
    fragment_uniform_cache_.has_camera = true;
    RecalculateFragmentDerived();
    return;
  }
  // 兼容单光源
  if (uniformbuffer_.HasUniform<Light>("light") &&
      uniformbuffer_.HasUniform<Vector3f>("cameraPos")) {
    fragment_uniform_cache_.lights.clear();
    fragment_uniform_cache_.lights.push_back(
        uniformbuffer_.GetUniform<Light>("light"));
    fragment_uniform_cache_.camera_pos =
        uniformbuffer_.GetUniform<Vector3f>("cameraPos");
    fragment_uniform_cache_.has_lights = true;
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
  // Helper: 将 Color 转为 [0,1] 归一化向量
  auto color_to_vec = [](const Color& c) -> Vector3f {
    constexpr float inv255 = 1.0f / 255.0f;
    return Vector3f(static_cast<float>(c[Color::kColorIndexRed]) * inv255,
                    static_cast<float>(c[Color::kColorIndexGreen]) * inv255,
                    static_cast<float>(c[Color::kColorIndexBlue]) * inv255);
  };

  // 输入插值属性
  Vector3f base_color = color_to_vec(fragment.color);
  Vector3f normal = glm::normalize(fragment.normal);
  Vector2f uv = fragment.uv;

  // uniform（优先缓存）
  std::vector<Light> lights;
  std::vector<Vector3f> light_dirs;
  Vector3f camera_pos;
  if (fragment_uniform_cache_.derived_valid) {
    lights = fragment_uniform_cache_.lights;
    light_dirs = fragment_uniform_cache_.light_dirs_normalized;
    camera_pos = fragment_uniform_cache_.camera_pos;
  } else {
    if (uniformbuffer_.HasUniform<std::vector<Light>>("lights")) {
      lights = uniformbuffer_.GetUniform<std::vector<Light>>("lights");
      light_dirs.reserve(lights.size());
      for (const auto& l : lights) light_dirs.push_back(glm::normalize(l.dir));
    } else if (uniformbuffer_.HasUniform<Light>("light")) {
      lights = {uniformbuffer_.GetUniform<Light>("light")};
      light_dirs = {glm::normalize(lights[0].dir)};
    }
    camera_pos = uniformbuffer_.GetUniform<Vector3f>("cameraPos");
  }

  Material material = *fragment.material;

  // 视线方向
  Vector3f view_dir = glm::normalize(sharedDataInShader_.fragPos_varying - camera_pos);

  // ambient（只计算一次，使用纹理或顶点颜色）
  Vector3f ambient_rgb;
  if (material.has_ambient_texture) {
    ambient_rgb = color_to_vec(SampleTexture(material.ambient_texture, uv));
  } else {
    ambient_rgb = base_color;
  }

  // diffuse/specular 累加（float 归一化空间，避免 8bit 溢出与截断）
  Vector3f diffuse_accum(0.0f);
  Vector3f specular_accum(0.0f);
  for (size_t i = 0; i < light_dirs.size(); ++i) {
    const Vector3f& ldir = light_dirs[i];
    float intensity = std::max(glm::dot(normal, ldir), 0.0f);

    // diffuse
    Vector3f kd = material.has_diffuse_texture
                      ? color_to_vec(SampleTexture(material.diffuse_texture, uv))
                      : base_color;
    diffuse_accum += kd * intensity;

    // specular
    Vector3f halfVector = glm::normalize(ldir + view_dir);
    float cos_theta = std::max(glm::dot(normal, halfVector), 0.0f);
    float spec = EvaluateSpecular(cos_theta, material.shininess);
    Vector3f ks = material.has_specular_texture
                      ? color_to_vec(SampleTexture(material.specular_texture, uv))
                      : Vector3f(1.0f);
    specular_accum += ks * spec;
  }

  Vector3f out_rgb = ambient_rgb * 0.1f + diffuse_accum + specular_accum * 0.2f;
  // clamp 到 [0,1]
  out_rgb.x = std::clamp(out_rgb.x, 0.0f, 1.0f);
  out_rgb.y = std::clamp(out_rgb.y, 0.0f, 1.0f);
  out_rgb.z = std::clamp(out_rgb.z, 0.0f, 1.0f);
  return Color(out_rgb.x, out_rgb.y, out_rgb.z, 1.0f);
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
