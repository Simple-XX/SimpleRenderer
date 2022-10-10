
/**
 * @file matrix_test.cpp
 * @brief matrix4_t 测试
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

// 矩阵与数乘法
TEST(matrix_t, matrix_num) {
    float mat_arr[4][4] = {
        { 5, -2,  2, 7},
        { 1,  0,  0, 3},
        {-3,  1,  5, 0},
        { 3, -1, -9, 4}
    };
    matrix4f_t mat(mat_arr);
    float      res_arr[4][4] = {
        {10, -4,   4, 14},
        { 2,  0,   0,  6},
        {-6,  2,  10,  0},
        { 6, -2, -18,  8}
    };
    auto res_mat = matrix4f_t(res_arr);
    EXPECT_EQ((res_mat == mat * 2), true);
    EXPECT_EQ((res_mat == 2 * mat), true);
    return;
}

// 矩阵与向量乘法
TEST(matrix_t, matrix_vector) {
    float mat_arr[4][4] = {
        { 5, -2,  2, 7},
        { 1,  0,  0, 3},
        {-3,  1,  5, 0},
        { 3, -1, -9, 4}
    };
    matrix4f_t mat(mat_arr);
    // 矩阵在左，向量在右
    vector4f_t vec1(1, 2, 3, 4);
    auto       res_vec1 = vector4f_t(35, 13, 14, -10);
    EXPECT_EQ((res_vec1 == (mat * vec1)), true);
    // 向量在左，矩阵在右
    auto res_vec2 = vector4f_t(10, -3, -19, 29);
    EXPECT_EQ((res_vec2 == (vec1 * mat)), true);
    return;
}

// 矩阵与矩阵乘法
TEST(matrix_t, matrix_matrix) {
    float mat_arr1[4][4] = {
        { 5, -2,  2, 7},
        { 1,  0,  0, 3},
        {-3,  1,  5, 0},
        { 3, -1, -9, 4}
    };
    matrix4f_t mat1(mat_arr1);
    float      mat_arr2[4][4] = {
        { 1,  -1,   2,  3},
        { 5,   8,  13, 21},
        {-2,   4,   7,  8},
        {19, -12, -14, 17}
    };
    matrix4f_t mat2(mat_arr2);

    // mat1*mat2
    float      res_arr1[4][4] = {
        {124, -97, -100, 108},
        { 58, -37,  -40,  54},
        { -8,  31,   42,  52},
        { 92, -95, -126, -16}
    };
    auto res_mat1 = matrix4f_t(res_arr1);
    EXPECT_EQ((res_mat1 == (mat1 * mat2)), true);
    // mat2*mat1
    float res_arr2[4][4] = {
        {  7,  -3,  -15,  16},
        { 57, -18, -114, 143},
        { -3,   3,  -41,  30},
        {176, -69, -185, 165}
    };
    auto res_mat2 = matrix4f_t(res_arr2);
    EXPECT_EQ((res_mat2 == (mat2 * mat1)), true);
    return;
}

// 转置矩阵
TEST(matrix_t, transpose) {
    float mat_arr[4][4] = {
        { 5, -2,  2, 7},
        { 1,  0,  0, 3},
        {-3,  1,  5, 0},
        { 3, -1, -9, 4}
    };
    matrix4f_t mat(mat_arr);
    float      res_arr[4][4] = {
        { 5, 1, -3,  3},
        {-2, 0,  1, -1},
        { 2, 0,  5, -9},
        { 7, 3,  0,  4}
    };
    auto res_mat = matrix4f_t(res_arr);
    EXPECT_EQ((res_mat == mat.transpose()), true);
    return;
}

// 逆矩阵
TEST(matrix_t, inverse) {
    float mat_arr[4][4] = {
        { 5, -2,  2, 7},
        { 1,  0,  0, 3},
        {-3,  1,  5, 0},
        { 3, -1, -9, 4}
    };
    matrix4f_t mat(mat_arr);
    float      res_arr[4][4] = {
        {-0.13636364,  0.86363637,  -0.68181818, -0.40909093},
        {-0.63636362,   2.3636365,  -0.93181818, -0.65909093},
        {0.045454547, 0.045454547, -0.022727273, -0.11363637},
        {0.045454547, 0.045454547,   0.22727273,  0.13636364}
    };
    matrix4f_t res_mat(res_arr);
    EXPECT_EQ((res_mat == mat.inverse()), true);
    return;
}

// 缩放
TEST(matrix_t, scale) {
    auto vec     = vector4f_t(1, 2, 3, vector4f_t::W_POINT);
    auto mat     = matrix4f_t().scale(1, 2, 3);
    auto res_vec = vector4f_t(1, 4, 9, vector4f_t::W_POINT);
    EXPECT_EQ((res_vec == mat * vec), true);
    return;
}

// 旋转
TEST(matrix_t, rotate) {
    auto mat1 = matrix4f_t().rotate(
      vector4f_t(1, 0, 0, vector4f_t::W_VECTOR).normalize(), 45);
    float res_arr1[4][4] = {
        {1,                 0,                  0, 0},
        {0, 0.707106781186548, -0.707106781186547, 0},
        {0, 0.707106781186547,  0.707106781186548, 0},
        {0,                 0,                  0, 1}
    };
    matrix4f_t res_mat1(res_arr1);
    EXPECT_EQ((res_mat1 == mat1), true);
    auto vec1     = vector4f_t(1, 2, 3, 4);
    /// @todo 精度问题
    auto res_vec1 = vector4f_t(1, -0.7071068, 3.53553390593274, 4);
    EXPECT_EQ((res_vec1 == (mat1 * vec1)), true);

    auto mat2 = matrix4f_t().rotate(
      vector4f_t(0, 1, 0, vector4f_t::W_VECTOR).normalize(), 45);
    float res_arr2[4][4] = {
        { 0.707106781186548, 0, 0.707106781186547, 0},
        {                 0, 1,                 0, 0},
        {-0.707106781186547, 0, 0.707106781186548, 0},
        {                 0, 0,                 0, 1}
    };
    matrix4f_t res_mat2(res_arr2);
    EXPECT_EQ((res_mat2 == mat2), true);
    auto vec2     = vector4f_t(1, 2, 3, 4);
    /// @todo 精度问题
    auto res_vec2 = vector4f_t(2.8284272, 2, 1.4142134, 4);
    EXPECT_EQ((res_vec2 == (mat2 * vec2)), true);

    auto mat3 = matrix4f_t().rotate(
      vector4f_t(0, 0, 1, vector4f_t::W_VECTOR).normalize(), 45);
    float res_arr3[4][4] = {
        {0.707106781186548, -0.707106781186547, 0, 0},
        {0.707106781186547,  0.707106781186548, 0, 0},
        {                0,                  0, 1, 0},
        {                0,                  0, 0, 1}
    };
    matrix4f_t res_mat3(res_arr3);
    EXPECT_EQ((res_mat3 == mat3), true);
    auto vec3     = vector4f_t(1, 2, 3, 4);
    auto res_vec3 = vector4f_t(-0.707106781186547, 2.12132034355964, 3, 4);
    EXPECT_EQ((res_vec3 == (mat3 * vec3)), true);

    auto mat4 = matrix4f_t().rotate(
      vector4f_t(1, 1, 0, vector4f_t::W_VECTOR).normalize(), 45);
    float res_arr4[4][4] = {
        {0.853553390593274, 0.146446609406726,               0.5, 0},
        {0.146446609406726, 0.853553390593274,              -0.5, 0},
        {             -0.5,               0.5, 0.707106781186548, 0},
        {                0,                 0,                 0, 1}
    };
    matrix4f_t res_mat4(res_arr4);
    EXPECT_EQ((res_mat4 == mat4), true);
    auto vec4 = vector4f_t(1, 2, 3, 4);
    auto res_vec4
      = vector4f_t(2.64644660940673, 0.353553390593274, 2.62132034355964, 4);
    EXPECT_EQ((res_vec4 == (mat4 * vec4)), true);

    auto mat5 = matrix4f_t().rotate(
      vector4f_t(1, 0, 1, vector4f_t::W_VECTOR).normalize(), 45);
    float res_arr5[4][4] = {
        {0.853553390593274,              -0.5, 0.146446609406726, 0},
        {              0.5, 0.707106781186548,              -0.5, 0},
        {0.146446609406726,               0.5, 0.853553390593274, 0},
        {                0,                 0,                 0, 1}
    };
    matrix4f_t res_mat5(res_arr5);
    EXPECT_EQ((res_mat5 == mat5), true);
    auto vec5 = vector4f_t(1, 2, 3, 4);
    /// @todo 精度问题
    auto res_vec5
      = vector4f_t(0.292893218813453, 0.414213562373095, 3.707107, 4);
    EXPECT_EQ((res_vec5 == (mat5 * vec5)), true);

    auto mat6 = matrix4f_t().rotate(
      vector4f_t(0, 1, 1, vector4f_t::W_VECTOR).normalize(), 45);
    float res_arr6[4][4] = {
        {0.707106781186548,              -0.5,               0.5, 0},
        {              0.5, 0.853553390593274, 0.146446609406726, 0},
        {             -0.5, 0.146446609406726, 0.853553390593274, 0},
        {                0,                 0,                 0, 1}
    };
    matrix4f_t res_mat6(res_arr6);
    EXPECT_EQ((res_mat6 == mat6), true);
    auto vec6 = vector4f_t(1, 2, 3, 4);
    /// @todo 精度问题
    auto res_vec6
      = vector4f_t(1.20710678118655, 2.64644660940673, 2.3535535, 4);
    EXPECT_EQ((res_vec6 == (mat6 * vec6)), true);

    auto mat7 = matrix4f_t().rotate(
      vector4f_t(1, 1, 1, vector4f_t::W_VECTOR).normalize(), 45);
    float res_arr7[4][4] = {
        { 0.804737854124365, -0.310617217526046,   0.50587936340168, 0},
        {  0.50587936340168,  0.804737854124365, -0.310617217526046, 0},
        {-0.310617217526046,   0.50587936340168,  0.804737854124365, 0},
        {                 0,                  0,                  0, 1}
    };
    matrix4f_t res_mat7(res_arr7);
    EXPECT_EQ((res_mat7 == mat7), true);
    auto vec7 = vector4f_t(1, 2, 3, 4);
    /// @todo 精度问题
    auto res_vec7
      = vector4f_t(1.70114150927732, 1.18350341907227, 3.1153552, 4);
    EXPECT_EQ((res_vec7 == (mat7 * vec7)), true);

    auto mat8 = matrix4f_t().rotate(
      vector4f_t(1, 1, 1, vector4f_t::W_VECTOR).normalize(), 99);
    float res_arr8[4][4] = {
        {  0.22904368997318, -0.184763974304647,  0.955720284331468, 0},
        { 0.955720284331468,   0.22904368997318, -0.184763974304647, 0},
        {-0.184763974304647,  0.955720284331468,   0.22904368997318, 0},
        {                 0,                  0,                  0, 1}
    };
    matrix4f_t res_mat8(res_arr8);
    EXPECT_EQ((res_mat8 == mat8), true);
    auto vec8     = vector4f_t(1, 2, 3, 4);
    /// @todo 精度问题
    auto res_vec8 = vector4f_t(2.7266762, 0.85951554, 2.4138073, 4);
    EXPECT_EQ((res_vec8 == (mat8 * vec8)), true);

    auto mat9 = matrix4f_t().rotate(
      vector4f_t(1, 1, 1, vector4f_t::W_VECTOR).normalize(), 15);
    float res_arr9[4][4] = {
        { 0.977283884192712, -0.138071187457698,  0.160787303264986, 0},
        { 0.160787303264986,  0.977283884192712, -0.138071187457698, 0},
        {-0.138071187457698,  0.160787303264986,  0.977283884192712, 0},
        {                 0,                  0,                  0, 1}
    };
    matrix4f_t res_mat9(res_arr9);
    EXPECT_EQ((res_mat9 == mat9), true);
    auto vec9 = vector4f_t(1, 2, 3, 4);
    auto res_vec9
      = vector4f_t(1.18350341907227, 1.70114150927732, 3.11535507165041, 4);
    EXPECT_EQ((res_vec9 == (mat9 * vec9)), true);

    auto mat10 = matrix4f_t().rotate(
      vector4f_t(1, 1, 1, vector4f_t::W_VECTOR).normalize(), 81);
    float res_arr10[4][4] = {
        { 0.437622976693487, -0.289053617664801,  0.851430640971314, 0},
        { 0.851430640971314,  0.437622976693487, -0.289053617664801, 0},
        {-0.289053617664801,  0.851430640971314,  0.437622976693487, 0},
        {                 0,                  0,                  0, 1}
    };
    matrix4f_t res_mat10(res_arr10);
    EXPECT_EQ((res_mat10 == mat10), true);
    auto vec10     = vector4f_t(1, 2, 3, 4);
    /// @todo 精度问题
    auto res_vec10 = vector4f_t(2.4138073, 0.8595156, 2.7266764, 4);
    EXPECT_EQ((res_vec10 == (mat10 * vec10)), true);

    auto mat11 = matrix4f_t().rotate(
      vector4f_t(5, 10, 4, vector4f_t::W_VECTOR).normalize(), 34);
    float res_arr11[4][4] = {
        { 0.859350059690673,  -0.1277451768906,  0.495175367613159, 0},
        { 0.248995125433123, 0.950287521097565, -0.186962709535318, 0},
        {-0.446675388196149, 0.283962668369336,  0.848437564321845, 0},
        {                 0,                 0,                  0, 1}
    };
    matrix4f_t res_mat11(res_arr11);
    EXPECT_EQ((res_mat11 == mat11), true);
    auto vec11 = vector4f_t(1, 2, 3, 4);
    /// @todo 精度问题
    auto res_vec11
      = vector4f_t(2.08938580874895, 1.5886820390223, 2.6665628, 4);
    EXPECT_EQ((res_vec11 == (mat11 * vec11)), true);

    return;
}

// 平移
TEST(matrix_t, translate) {
    auto vec     = vector4f_t(1, 2, 3, vector4f_t::W_POINT);
    auto mat     = matrix4f_t().translate(5, 6, 7);
    auto res_vec = vector4f_t(6, 8, 10, vector4f_t::W_POINT);
    EXPECT_EQ((res_vec == mat * vec), true);

    return;
}

// 缩放 + 旋转
TEST(matrix_t, scale_rotate) {
    auto vec     = vector4f_t(1, 2, 3, vector4f_t::W_POINT);
    auto mat     = matrix4f_t().scale(5, 6, 7);
    auto res_vec = vector4f_t(5, 12, 21, vector4f_t::W_POINT);
    EXPECT_EQ((res_vec == mat * vec), true);

    mat     = mat.rotate(vector4f_t(0, 0, 1).normalize(), 90);
    /// @todo 精度问题
    res_vec = vector4f_t(-12, 4.9999995, 20.999998, vector4f_t::W_POINT);
    EXPECT_EQ((res_vec == mat * vec), true);

    return;
}

// 旋转 + 缩放
TEST(matrix_t, rotate_scale) {
    auto vec     = vector4f_t(1, 2, 3, vector4f_t::W_POINT);
    auto mat     = matrix4f_t().rotate(vector4f_t(0, 0, 1).normalize(), 90);
    /// @todo 精度问题
    auto res_vec = vector4f_t(-2, 0.99999994, 2.9999997, vector4f_t::W_POINT);
    EXPECT_EQ((res_vec == mat * vec), true);

    mat     = mat.scale(5, 6, 7);
    /// @todo 精度问题
    res_vec = vector4f_t(-10, 5.9999995, 20.999998, vector4f_t::W_POINT);
    EXPECT_EQ((res_vec == mat * vec), true);

    return;
}

// 平移 + 缩放
TEST(matrix_t, translate_scale) {
    auto vec     = vector4f_t(1, 2, 3, vector4f_t::W_POINT);
    auto mat     = matrix4f_t().translate(5, 6, 7);
    auto res_vec = vector4f_t(6, 8, 10, vector4f_t::W_POINT);
    auto ret     = (res_vec == mat * vec);
    EXPECT_EQ(ret, true);

    mat     = mat.scale(4, 5, 6);
    res_vec = vector4f_t(24, 40, 60, vector4f_t::W_POINT);
    ret     = (res_vec == mat * vec);
    EXPECT_EQ(ret, true);

    mat = matrix4f_t().translate(5, 6, 7).scale(4, 5, 6);
    ret = (res_vec == mat * vec);
    EXPECT_EQ(ret, true);

    return;
}

// 缩放 + 平移
TEST(matrix_t, scale_translate) {
    auto vec     = vector4f_t(1, 2, 3, vector4f_t::W_POINT);
    auto mat     = matrix4f_t().scale(5, 6, 7);
    auto res_vec = vector4f_t(5, 12, 21, vector4f_t::W_POINT);
    auto ret     = (res_vec == mat * vec);
    EXPECT_EQ(ret, true);

    mat     = mat.translate(4, 5, 6);
    res_vec = vector4f_t(9, 17, 27, vector4f_t::W_POINT);
    ret     = (res_vec == mat * vec);
    EXPECT_EQ(ret, true);

    mat = matrix4f_t().scale(5, 6, 7).translate(4, 5, 6);
    ret = (res_vec == mat * vec);
    EXPECT_EQ(ret, true);

    return;
}

// 平移 + 旋转
TEST(matrix_t, translate_rotate) {
    auto vec     = vector4f_t(0, 0, 0, vector4f_t::W_POINT);
    auto mat     = matrix4f_t().translate(1, 2, 3);
    auto res_vec = vector4f_t(1, 2, 3, vector4f_t::W_POINT);
    auto ret     = (res_vec == mat * vec);
    EXPECT_EQ(ret, true);

    mat     = mat.rotate(vector4f_t(0, 0, 1).normalize(), 90);
    /// @todo 精度问题
    res_vec = vector4f_t(-2, 0.99999994, 2.9999997, vector4f_t::W_POINT);
    EXPECT_EQ((res_vec == mat * vec), true);

    return;
}

// 旋转 + 平移
TEST(matrix_t, rotate_translate) {
    auto vec     = vector4f_t(1, 2, 3, vector4f_t::W_POINT);
    auto mat     = matrix4f_t().rotate(vector4f_t(0, 0, 1).normalize(), 90);
    /// @todo 精度问题
    auto res_vec = vector4f_t(-2, 0.99999994, 2.9999997, vector4f_t::W_POINT);
    auto ret     = (res_vec == mat * vec);
    EXPECT_EQ(ret, true);

    mat     = mat.translate(1, 2, 3);
    res_vec = vector4f_t(-1, 3, 6, vector4f_t::W_POINT);
    EXPECT_EQ((res_vec == mat * vec), true);

    return;
}

// 缩放 + 平移 + 旋转
TEST(matrix_t, scale_translate_rotate) {
    auto vec     = vector4f_t(1, 2, 3, vector4f_t::W_POINT);
    auto mat     = matrix4f_t().scale(5, 6, 7);
    auto res_vec = vector4f_t(5, 12, 21, vector4f_t::W_POINT);
    EXPECT_EQ((res_vec == mat * vec), true);

    mat     = mat.translate(1, 2, 3);
    res_vec = vector4f_t(6, 14, 24, vector4f_t::W_POINT);
    EXPECT_EQ((res_vec == mat * vec), true);

    mat     = mat.rotate(vector4f_t(0, 0, 1).normalize(), 90);
    // @todo 精度问题
    res_vec = vector4f_t(-14, 5.9999995, 23.999998, vector4f_t::W_POINT);
    EXPECT_EQ((res_vec == mat * vec), true);

    return;
}

// 缩放 + 旋转 + 平移
TEST(matrix_t, scale_rotate_translate) {
    auto vec     = vector4f_t(1, 2, 3, vector4f_t::W_POINT);
    auto mat     = matrix4f_t().scale(5, 6, 7);
    auto res_vec = vector4f_t(5, 12, 21, vector4f_t::W_POINT);
    EXPECT_EQ((res_vec == mat * vec), true);

    mat     = mat.rotate(vector4f_t(0, 0, 1).normalize(), 90);
    // @todo 精度问题
    res_vec = vector4f_t(-12, 4.9999995, 20.999998, vector4f_t::W_POINT);
    EXPECT_EQ((res_vec == mat * vec), true);

    mat     = mat.translate(1, 2, 3);
    res_vec = vector4f_t(-11, 6.9999995, 23.999998, vector4f_t::W_POINT);
    EXPECT_EQ((res_vec == mat * vec), true);

    return;
}

// 平移 + 缩放 + 旋转
TEST(matrix_t, translate_scale_rotate) {
    auto vec     = vector4f_t(0, 0, 0, vector4f_t::W_POINT);
    auto mat     = matrix4f_t().translate(1, 2, 3);
    auto res_vec = vector4f_t(1, 2, 3, vector4f_t::W_POINT);
    EXPECT_EQ((res_vec == mat * vec), true);

    mat     = mat.scale(4, 5, 6);
    res_vec = vector4f_t(4, 10, 18, vector4f_t::W_POINT);
    EXPECT_EQ((res_vec == mat * vec), true);

    mat     = mat.rotate(vector4f_t(0, 0, 1).normalize(), 90);
    // @todo 精度问题
    res_vec = vector4f_t(-10, 3.9999995, 17.999998, vector4f_t::W_POINT);
    EXPECT_EQ((res_vec == mat * vec), true);

    return;
}

// 平移 + 旋转 + 缩放
TEST(matrix_t, translate_rotate_scale) {
    auto vec     = vector4f_t(0, 0, 0, vector4f_t::W_POINT);
    auto mat     = matrix4f_t().translate(1, 2, 3);
    auto res_vec = vector4f_t(1, 2, 3, vector4f_t::W_POINT);
    EXPECT_EQ((res_vec == mat * vec), true);

    mat     = mat.rotate(vector4f_t(0, 0, 1).normalize(), 90);
    // @todo 精度问题
    res_vec = vector4f_t(-2, 0.99999994, 2.9999997, vector4f_t::W_POINT);
    EXPECT_EQ((res_vec == mat * vec), true);

    mat     = mat.scale(4, 5, 6);
    res_vec = vector4f_t(-8, 4.9999995, 17.999998, vector4f_t::W_POINT);
    EXPECT_EQ((res_vec == mat * vec), true);

    return;
}

// 旋转 + 缩放 + 平移
TEST(matrix_t, rotate_scale_translate) {
    auto vec     = vector4f_t(1, 2, 3, vector4f_t::W_POINT);
    auto mat     = matrix4f_t().rotate(vector4f_t(0, 0, 1).normalize(), 90);
    auto res_vec = vector4f_t(-2, 0.99999994, 2.9999997, vector4f_t::W_POINT);
    EXPECT_EQ((res_vec == mat * vec), true);

    mat     = mat.scale(5, 6, 7);
    // @todo 精度问题
    res_vec = vector4f_t(-10, 5.9999995, 20.999998, vector4f_t::W_POINT);
    EXPECT_EQ((res_vec == mat * vec), true);

    mat     = mat.translate(1, 2, 3);
    res_vec = vector4f_t(-9, 7.9999995, 23.999998, vector4f_t::W_POINT);
    EXPECT_EQ((res_vec == mat * vec), true);

    return;
}

// 旋转 + 平移 + 缩放
TEST(matrix_t, rotate_translate_scale) {
    auto vec     = vector4f_t(1, 2, 3, vector4f_t::W_POINT);
    auto mat     = matrix4f_t().rotate(vector4f_t(0, 0, 1).normalize(), 90);
    auto res_vec = vector4f_t(-2, 0.99999994, 2.9999997, vector4f_t::W_POINT);
    EXPECT_EQ((res_vec == mat * vec), true);

    mat     = mat.translate(1, 2, 3);
    res_vec = vector4f_t(-1, 3, 6, vector4f_t::W_POINT);
    EXPECT_EQ((res_vec == mat * vec), true);

    mat     = mat.scale(5, 6, 7);
    res_vec = vector4f_t(-5, 18, 42, vector4f_t::W_POINT);
    EXPECT_EQ((res_vec == mat * vec), true);

    return;
}

// 旋转到
TEST(matrix_t, rotate_from_to) {
    auto  vec           = vector4f_t(1, 2, 0, 0);
    auto  mat           = matrix4f_t().rotate_from_to(vector4f_t(1, 2, 0, 0),
                                                      vector4f_t(-2, 1, 0, 0));

    float res_arr[4][4] = {
        {0, -1, 0, 0},
        {1,  0, 0, 0},
        {0,  0, 1, 0},
        {0,  0, 0, 1}
    };
    matrix4f_t res_mat(res_arr);
    EXPECT_EQ((res_mat == mat), true);
    auto res_vec = vector4f_t(-2, 1, 0, 0);
    EXPECT_EQ((res_vec == mat * vec), true);

    return;
}
