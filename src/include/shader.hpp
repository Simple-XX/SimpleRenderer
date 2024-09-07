#ifndef SIMPLERENDER_SRC_INCLUDE_SHADER_HPP_
#define SIMPLERENDER_SRC_INCLUDE_SHADER_HPP_

#include "material.hpp"
#include "vertex.hpp"

namespace simple_renderer {

using UniformValue = std::variant<int, float, Vector2f, Vector3f, Vector4f,
                                  Matrix3f, Matrix4f, Material>;

class UniformBuffer {
 public:
  template <typename T>
  void SetUniform(const std::string &name, const T &value) {
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
};

/**
 * @brief Shader Class 着色器类
 *
 */
class Shader {
 public:
  Shader() = default;
  Shader(const Shader &shader_base) = default;
  Shader(Shader &&shader_base) = default;
  auto operator=(const Shader &shader_base) -> Shader & = default;
  auto operator=(Shader &&shader_base) -> Shader & = default;
  virtual ~Shader() = default;

  // Input Data -> Vertex Shader -> Screen Space Coordiante
  Vertex VertexShader(const Vertex &vertex) const;
  // Input Data -> Fragment Shader -> Color
  Color FragmentShader(const Fragment &fragment) const;

 private:
  // UniformBuffer
  // TODO: add setter function and getter function under Shader class level
  UniformBuffer uniformbuffer_;

  Color sampleTexture(const Texture &texture, const Vector2f &uv) const;
};

uint8_t float_to_uint8_t(float val);

}  // namespace simple_renderer

#endif /* SIMPLERENDER_SRC_INCLUDE_SHADER_H_ */