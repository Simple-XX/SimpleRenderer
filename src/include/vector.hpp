
/**
 * @file vector.hpp
 * @brief 向量模版
 * @author Zone.N (Zone.Niuzh@hotmail.com)
 * @version 1.0
 * @date 2022-06-07
 * @copyright MIT LICENSE
 * https://github.com/Simple-XX/SimpleRenderer
 * @par change log:
 * <table>
 * <tr><th>Date<th>Author<th>Description
 * <tr><td>2022-06-07<td>Zone.N<td>迁移到 doxygen
 * </table>
 */

#ifndef SIMPLERENDER_SRC_INCLUDE_VECTOR_HPP_
#define SIMPLERENDER_SRC_INCLUDE_VECTOR_HPP_

#include <glm/glm.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>

#include "log_system.h"

namespace simple_renderer {
using Vector2f = glm::vec2;
using Vector3f = glm::vec3;
using Vector4f = glm::vec4;
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

#endif /* SIMPLERENDER_SRC_INCLUDE_VECTOR_HPP_ */
