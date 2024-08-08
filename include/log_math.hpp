
/**
 * @file matrix.hpp
 * @brief 矩阵
 * @author Zone.N (Zone.Niuzh@hotmail.com)
 * @version 1.0
 * @date 2022-09-07
 * @copyright MIT LICENSE
 * https://github.com/Simple-XX/SimpleRenderer
 * @par change log:
 * <table>
 * <tr><th>Date<th>Author<th>Description
 * <tr><td>2022-09-07<td>Zone.N<td>迁移到 doxygen
 * </table>
 */

#ifndef SIMPLERENDER_SRC_INCLUDE_MATRIX_HPP_
#define SIMPLERENDER_SRC_INCLUDE_MATRIX_HPP_

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>

#include "log_system.h"

/**
 * spdlog 输出 Vector3f 实现
 */
template <>
struct fmt::formatter<glm::vec3> : fmt::formatter<std::string> {
    auto format(const glm::vec3 &vector, fmt::format_context &ctx) const -> decltype(ctx.out()) {
        std::string vector_string = glm::to_string(vector);
    
        // Format and output the string
        return fmt::format_to(ctx.out(), "\n{}", vector_string);
    }
};

/**
 * spdlog 输出 Vector4f 实现
 */
template <>
struct fmt::formatter<glm::vec4> : fmt::formatter<std::string> {
    auto format(const glm::vec4 &vector, fmt::format_context &ctx) const -> decltype(ctx.out()) {
        std::string vector_string = glm::to_string(vector);
    
        // Format and output the string
        return fmt::format_to(ctx.out(), "\n{}", vector_string);
    }
};

/**
 * spdlog 输出矩阵实现
 */
template <>
struct fmt::formatter<glm::mat4> : fmt::formatter<std::string> {
  auto format(const glm::mat4 &matrix, fmt::format_context &ctx) const -> decltype(ctx.out()) {
    // Convert the glm::mat4 to a string using glm::to_string
    std::string matrix_str = glm::to_string(matrix);
    
    // Format and output the string
    return fmt::format_to(ctx.out(), "\n{}", matrix_str);
  }
};

#endif /* SIMPLERENDER_SRC_INCLUDE_MATRIX_HPP_ */
