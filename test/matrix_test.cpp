
/**
 * @file matrix_test.cpp
 * @brief matrix_t 测试
 * @author Zone.N (Zone.Niuzh@hotmail.com)
 * @version 1.0
 * @date 2022-09-07
 * @copyright MIT LICENSE
 * https://github.com/Simple-XX/SimpleRenderer
 * @par change log:
 * <table>
 * <tr><th>Date<th>Author<th>Description
 * <tr><td>2022-09-07<td>Zone.N<td>创建文件
 * </table>
 */

#include "matrix.hpp"
#include "gtest/gtest.h"

TEST(matrix_t, test1) {
    matrix4f_t mat;
    std::cout << mat << std::endl;
    return;
}
