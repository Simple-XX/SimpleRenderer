
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

#include <Eigen/Dense>

#include "log_system.h"

namespace simple_renderer {

using Matrix4f = Eigen::Matrix4f;

}  // namespace simple_renderer

/**
 * spdlog 输出矩阵实现
 */
template <>
struct fmt::formatter<simple_renderer::Matrix4f> : fmt::formatter<std::string> {
  auto format(simple_renderer::Matrix4f matrix, format_context &format_context)
      const -> decltype(format_context.out()) {
    std::stringstream buf;
    buf << matrix;
    return fmt::format_to(format_context.out(), "\n{}", buf.str());
  }
};

#endif /* SIMPLERENDER_SRC_INCLUDE_MATRIX_HPP_ */
