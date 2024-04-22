
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

#ifndef SIMPLERENDER_VECTOR_HPP
#define SIMPLERENDER_VECTOR_HPP

#include <Eigen/Dense>

#include "log.h"

namespace SimpleRenderer {

template <class T> using vector2_t = Eigen::Vector<T, 2>;
template <class T> using vector3_t = Eigen::Vector<T, 3>;
template <class T> using vector4_t = Eigen::Vector<T, 4>;

using vector2f_t = vector2_t<float>;
using vector3f_t = vector3_t<float>;
using vector4f_t = vector4_t<float>;

} // namespace SimpleRenderer

/**
 * spdlog 输出 vector3f_t 实现
 */
template <>
struct fmt::formatter<SimpleRenderer::vector3f_t>
    : fmt::formatter<std::string> {
  auto format(SimpleRenderer::vector3f_t _vector,
              format_context &_format_context) const
      -> decltype(_format_context.out()) {
    std::stringstream buf;
    buf << _vector;
    return fmt::format_to(_format_context.out(), "\n{}", buf.str());
  }
};

#endif /* SIMPLERENDER_VECTOR_HPP */
