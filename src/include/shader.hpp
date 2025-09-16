#ifndef SIMPLERENDER_SRC_INCLUDE_SHADER_HPP_
#define SIMPLERENDER_SRC_INCLUDE_SHADER_HPP_

#include <array>
#include <bit>
#include <shared_mutex>
#include <unordered_map>
#include <variant>

#include "light.h"
#include "material.hpp"
#include "vertex.hpp"

namespace simple_renderer {

using UniformValue = std::variant<int, float, Vector2f, Vector3f, Vector4f,
                                  Matrix3f, Matrix4f, Material, Light>;

inline constexpr size_t kSpecularLutResolution = 256;

class UniformBuffer {
 public:
  template <typename T>
  void SetUniform(const std::string &name, const T &value) {
    static_assert(
        std::is_same_v<T, int> || std::is_same_v<T, float> ||
            std::is_same_v<T, Vector2f> || std::is_same_v<T, Vector3f> ||
            std::is_same_v<T, Vector4f> || std::is_same_v<T, Matrix3f> ||
            std::is_same_v<T, Matrix4f> || std::is_same_v<T, Material> ||
            std::is_same_v<T, Light>,
        "Type not supported by UniformValue");
    uniforms_[name] = value;
  }

  template <typename T>
  T GetUniform(const std::string &name) const {
    return std::get<T>(uniforms_.at(name));
  }

  template <typename T>
  bool HasUniform(const std::string &name) const {
    return uniforms_.find(name) != uniforms_.end();
  }

 private:
  std::unordered_map<std::string, UniformValue> uniforms_;
};

/**
 * @brief Fragment 结构体
 * as input of Fragment Shader
 * 片段结构体作为片段着色器的输入
 */

struct Fragment {
  std::array<int32_t, 2> screen_coord;
  Vector3f normal;
  Vector2f uv;
  Color color;
  float depth;
  const Material *material;
};

/**
 * @brief Shared Variables 共享变量
 *
 */
struct SharedDataInShader {
  Vector3f fragPos_varying = Vector3f(0.0f);
};

struct VertexUniformCache {
  Matrix4f model = Matrix4f(1.0f);
  Matrix4f view = Matrix4f(1.0f);
  Matrix4f projection = Matrix4f(1.0f);
  Matrix4f model_view = Matrix4f(1.0f);
  Matrix4f mvp = Matrix4f(1.0f);
  Matrix3f normal = Matrix3f(1.0f);
  bool has_model = false;
  bool has_view = false;
  bool has_projection = false;
  bool derived_valid = false;
};

struct FragmentUniformCache {
  Light light{};
  Vector3f camera_pos = Vector3f(0.0f);
  Vector3f light_dir_normalized = Vector3f(0.0f);
  bool has_light = false;
  bool has_camera = false;
  bool derived_valid = false;
};

struct SpecularLUT {
  std::array<float, kSpecularLutResolution> values{};
};

/**
 * @brief Shader Class 着色器类
 *
 */
class Shader {
 public:
  Shader() = default;
  Shader(const Shader &shader);
  Shader(Shader &&shader) noexcept;
  auto operator=(const Shader &shader) -> Shader &;
  auto operator=(Shader &&shader) noexcept -> Shader &;
  virtual ~Shader() = default;

  // Input Data -> Vertex Shader -> Screen Space Coordiante
  Vertex VertexShader(const Vertex &vertex);
  // Input Data -> Fragment Shader -> Color
  Color FragmentShader(const Fragment &fragment) const;

  // set uniform value
  template <typename T>
  void SetUniform(const std::string &name, const T &value) {
    uniformbuffer_.SetUniform(name, value);
    if constexpr (std::is_same_v<T, Matrix4f>) {
      UpdateMatrixCache(name, value);
    } else if constexpr (std::is_same_v<T, Light>) {
      UpdateFragmentCache(name, value);
    } else if constexpr (std::is_same_v<T, Vector3f>) {
      UpdateFragmentCache(name, value);
    }
  }

  void PrepareUniformCaches();

 private:
  // UniformBuffer
  UniformBuffer uniformbuffer_;

  // Shared Variables
  // 共享变量
  SharedDataInShader sharedDataInShader_;
  VertexUniformCache vertex_uniform_cache_;
  FragmentUniformCache fragment_uniform_cache_;
  mutable std::unordered_map<uint32_t, SpecularLUT> specular_lut_cache_;
  mutable std::shared_mutex specular_cache_mutex_;

  void UpdateMatrixCache(const std::string &name, const Matrix4f &value);
  void UpdateFragmentCache(const std::string &name, const Light &value);
  void UpdateFragmentCache(const std::string &name, const Vector3f &value);
  void RecalculateDerivedMatrices();
  void RecalculateFragmentDerived();
  void PrepareVertexUniformCache();
  void PrepareFragmentUniformCache();

  // LUT相关
  [[nodiscard]] auto BuildSpecularLUT(float shininess) const -> SpecularLUT;
  [[nodiscard]] auto GetSpecularLUT(float shininess) const -> const SpecularLUT &;
  [[nodiscard]] auto EvaluateSpecular(float cos_theta, float shininess) const -> float;

  Color SampleTexture(const Texture &texture, const Vector2f &uv) const;
  Color ClampColor(const Color color) const;
};

uint8_t FloatToUint8_t(float val);

}  // namespace simple_renderer

#endif /* SIMPLERENDER_SRC_INCLUDE_SHADER_H_ */
