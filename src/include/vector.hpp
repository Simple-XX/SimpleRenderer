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

#include <Eigen/Dense>

#include "log_system.h"

namespace simple_renderer {

using Vector2f = Eigen::Vector2f;
using Vector3f = Eigen::Vector3f;
using Vector4f = Eigen::Vector4f;

}  // namespace simple_renderer

/**
 * spdlog 输出 Vector3f 实现
 */
template <>
struct fmt::formatter<simple_renderer::Vector3f> : fmt::formatter<std::string> {
  auto format(simple_renderer::Vector3f vector, format_context &format_context)
      const -> decltype(format_context.out()) {
    std::stringstream buf;
    buf << vector;
    return fmt::format_to(format_context.out(), "\n{}", buf.str());
  }
};

/**
 * spdlog 输出 Vector4f 实现
 */
template <>
struct fmt::formatter<simple_renderer::Vector4f> : fmt::formatter<std::string> {
  auto format(simple_renderer::Vector4f vector, format_context &format_context)
      const -> decltype(format_context.out()) {
    std::stringstream buf;
    buf << vector;
    return fmt::format_to(format_context.out(), "\n{}", buf.str());
  }
};

#endif /* SIMPLERENDER_SRC_INCLUDE_VECTOR_HPP_ */
