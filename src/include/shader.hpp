#ifndef SIMPLERENDER_SRC_INCLUDE_SHADER_HPP_
#define SIMPLERENDER_SRC_INCLUDE_SHADER_HPP_

#include "light.h"
#include "material.hpp"
#include "vertex.hpp"

namespace simple_renderer {

using UniformValue = std::variant<int, float, Vector2f, Vector3f, Vector4f,
                                  Matrix3f, Matrix4f, Material, Light>;

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
  T getUniform(const std::string &name) const {
    return std::get<T>(uniforms_.at(name));
  }

  template <typename T>
  bool hasUniform(const std::string &name) const {
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
};

/**
 * @brief Shader Class 着色器类
 *
 */
class Shader {
 public:
  Shader() = default;
  Shader(const Shader &shader) = default;
  Shader(Shader &&shader) = default;
  auto operator=(const Shader &shader) -> Shader & = default;
  auto operator=(Shader &&shader) -> Shader & = default;
  virtual ~Shader() = default;

  // Input Data -> Vertex Shader -> Screen Space Coordiante
  Vertex VertexShader(const Vertex &vertex) const;
  // Input Data -> Fragment Shader -> Color
  Color FragmentShader(const Fragment &fragment) const;

  // set uniform value
  template <typename T>
  void SetUniform(const std::string &name, const T &value) {
    uniformbuffer_.SetUniform(name, value);
  }

 private:
  // UniformBuffer
  UniformBuffer uniformbuffer_;

  Color sampleTexture(const Texture &texture, const Vector2f &uv) const;
};

uint8_t float_to_uint8_t(float val);

}  // namespace simple_renderer

#endif /* SIMPLERENDER_SRC_INCLUDE_SHADER_H_ */