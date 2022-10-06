
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

#include "gtest/gtest.h"

#include "matrix.hpp"
#include "vector.hpp"

TEST(matrix_t, test1) {
    float arr1[4][4] = {
        { 5, -2,  2, 7},
        { 1,  0,  0, 3},
        {-3,  1,  5, 0},
        { 3, -1, -9, 4}
    };
    matrix4f_t mat1(arr1);

    // 矩阵与数乘法
    float      res1[4][4] = {
        {10, -4,   4, 14},
        { 2,  0,   0,  6},
        {-6,  2,  10,  0},
        { 6, -2, -18,  8}
    };
    auto res_mat1 = matrix4f_t(res1);
    auto ret      = (res_mat1 == mat1 * 2);
    EXPECT_EQ(ret, true);
    ret = (res_mat1 == 2 * mat1);
    EXPECT_EQ(ret, true);

    // 矩阵与向量乘法
    vector4f_t vec1(1, 2, 3, 4);
    // 矩阵在左，向量在右
    auto       res_vec1 = vector4f_t(35, 13, 14, -10);
    ret                 = (res_vec1 == (mat1 * vec1));
    EXPECT_EQ(ret, true);
    // 向量在左，矩阵在右
    auto res_vec2 = vector4f_t(10, -3, -19, 29);
    ret           = (res_vec2 == (vec1 * mat1));
    EXPECT_EQ(ret, true);

    // 矩阵与矩阵乘法
    float arr2[4][4] = {
        { 1,  -1,   2,  3},
        { 5,   8,  13, 21},
        {-2,   4,   7,  8},
        {19, -12, -14, 17}
    };
    matrix4f_t mat2(arr2);
    // mat1*mat2
    float      res3[4][4] = {
        {124, -97, -100, 108},
        { 58, -37,  -40,  54},
        { -8,  31,   42,  52},
        { 92, -95, -126, -16}
    };
    auto res_mat3 = matrix4f_t(res3);
    ret           = (res_mat3 == (mat1 * mat2));
    EXPECT_EQ(ret, true);
    // mat2*mat1
    float res4[4][4] = {
        {  7,  -3,  -15,  16},
        { 57, -18, -114, 143},
        { -3,   3,  -41,  30},
        {176, -69, -185, 165}
    };
    auto res_mat4 = matrix4f_t(res4);
    ret           = (res_mat4 == (mat2 * mat1));
    EXPECT_EQ(ret, true);

    // 转置矩阵
    float res5[4][4] = {
        { 5, 1, -3,  3},
        {-2, 0,  1, -1},
        { 2, 0,  5, -9},
        { 7, 3,  0,  4}
    };
    auto res_mat5 = matrix4f_t(res5);
    ret           = (res_mat5 == mat1.transpose());
    EXPECT_EQ(ret, true);

    // 逆矩阵
    float res[4][4] = {
        {-0.13636364,  0.86363637,  -0.68181818, -0.40909093},
        {-0.63636362,   2.3636365,  -0.93181818, -0.65909093},
        {0.045454547, 0.045454547, -0.022727273, -0.11363637},
        {0.045454547, 0.045454547,   0.22727273,  0.13636364}
    };

    matrix4f_t res_mat(res);
    // std::cout << res_mat << std::endl;
    // std::cout << mat1.inverse() << std::endl;
    ret = (res_mat == mat1.inverse());

    EXPECT_EQ(ret, true);

    return;
}
