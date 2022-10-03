
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
    float src[4][4] = {
        { 5, -2,  2, 7},
        { 1,  0,  0, 3},
        {-3,  1,  5, 0},
        { 3, -1, -9, 4}
    };

    matrix4f_t src_mat(src);

    float      res[4][4] = {
        {-0.1364,  0.8636,  -0.6818, -0.4091},
        {-0.6364,   2.364,  -0.9318, -0.6591},
        {0.04545, 0.04545, -0.02273, -0.1136},
        {0.04545, 0.04545,   0.2273,  0.1364}
    };

    matrix4f_t res_mat(res);

    std::cout << src_mat.inverse() << std::endl;

    auto ret = (res_mat == src_mat.inverse());

    EXPECT_EQ(ret, true);

    return;
}
