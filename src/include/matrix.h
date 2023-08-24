
/**
 * @file matrix4.h
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

#ifndef SIMPLERENDER_MATRIX_H
#define SIMPLERENDER_MATRIX_H

#include "Eigen/Dense"

template <class T> using matrix4_t = Eigen::Matrix<T, 4, 4>;

using matrix4f_t = matrix4_t<float>;

#endif /* SIMPLERENDER_MATRIX_H */
