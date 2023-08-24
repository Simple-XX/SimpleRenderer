
/**
 * @file vector.h
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

#ifndef SIMPLERENDER_VECTOR_H
#define SIMPLERENDER_VECTOR_H

#include "Eigen/Dense"

template <class T> using vector2_t = Eigen::Vector<T, 2>;
template <class T> using vector3_t = Eigen::Vector<T, 3>;
template <class T> using vector4_t = Eigen::Vector<T, 4>;

using vector2f_t = vector2_t<float>;
using vector3f_t = vector3_t<float>;
using vector4f_t = vector4_t<float>;

#endif /* SIMPLERENDER_VECTOR_H */
