
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

namespace simple_renderer {
using Matrix4f = glm::mat4;
}  // namespace simple_renderer

/**
 * spdlog 输出矩阵实现
 */
template <>
struct fmt::formatter<simple_renderer::Matrix4f> : fmt::formatter<std::string> {
  auto format(const simple_renderer::Matrix4f &matrix,
              fmt::format_context &ctx) const -> decltype(ctx.out()) {
    // Convert the Matrix4f to a string using glm::to_string
    // 转化矩阵为字符串
    std::string matrix_str = glm::to_string(matrix);

    // Format and output the string
    // 输出格式化后的字符串
    return fmt::format_to(ctx.out(), "\n{}", matrix_str);
  }
};

#endif /* SIMPLERENDER_SRC_INCLUDE_MATRIX_HPP_ */
