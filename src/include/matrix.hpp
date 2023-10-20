
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

#ifndef SIMPLERENDER_MATRIX_HPP
#define SIMPLERENDER_MATRIX_HPP

#include <Eigen/Dense>

#include "log.h"

template <class T> using matrix4_t = Eigen::Matrix<T, 4, 4>;

using matrix4f_t = matrix4_t<float>;

/**
 * spdlog 输出矩阵实现
 */
template <> struct fmt::formatter<matrix4f_t> : fmt::formatter<std::string> {
  auto format(matrix4f_t _matrix, format_context &_format_context) const
      -> decltype(_format_context.out()) {
    std::stringstream buf;
    buf << _matrix;
    return format_to(_format_context.out(), "\n{}", buf.str());
  }
};

#endif /* SIMPLERENDER_MATRIX_HPP */
