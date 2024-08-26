#ifndef SIMPLERENDER_SRC_INCLUDE_MATH_HPP_
#define SIMPLERENDER_SRC_INCLUDE_MATH_HPP_

#include <glm/glm.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>

#include "log_system.h"

namespace simple_renderer {
using Vector2f = glm::vec2;
using Vector3f = glm::vec3;
using Vector4f = glm::vec4;
using Matrix4f = glm::mat4;
}  // namespace simple_renderer

/**
 * spdlog 输出 Vector3f 实现
 */
template <>
struct fmt::formatter<simple_renderer::Vector3f> : fmt::formatter<std::string> {
  auto format(const simple_renderer::Vector3f &vector,
              fmt::format_context &ctx) const -> decltype(ctx.out()) {
    std::string vector_string = glm::to_string(vector);

    // Format and output the string
    return fmt::format_to(ctx.out(), "\n{}", vector_string);
  }
};

/**
 * spdlog 输出 Vector4f 实现
 */
template <>
struct fmt::formatter<simple_renderer::Vector4f> : fmt::formatter<std::string> {
  auto format(const simple_renderer::Vector4f &vector,
              fmt::format_context &ctx) const -> decltype(ctx.out()) {
    std::string vector_string = glm::to_string(vector);

    // Format and output the string
    return fmt::format_to(ctx.out(), "\n{}", vector_string);
  }
};

/**
 * spdlog 输出矩阵实现
 */
template <>
struct fmt::formatter<simple_renderer::Matrix4f> : fmt::formatter<std::string> {
  auto format(const simple_renderer::Matrix4f &matrix,
              fmt::format_context &ctx) const -> decltype(ctx.out()) {
    // Convert the Matrix4f to a string using glm::to_string
    std::string matrix_str = glm::to_string(matrix);

    // Format and output the string
    return fmt::format_to(ctx.out(), "\n{}", matrix_str);
  }
};

#endif
