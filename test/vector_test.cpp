
/**
 * @file vector_test.cpp
 * @brief vector 测试
 * @author Zone.N (Zone.Niuzh@hotmail.com)
 * @version 1.0
 * @date 2022-06-07
 * @copyright MIT LICENSE
 * https://github.com/Simple-XX/SimpleRenderer
 * @par change log:
 * <table>
 * <tr><th>Date<th>Author<th>Description
 * <tr><td>2022-06-07<td>Zone.N<td>创建文件
 * </table>
 */

#include "gtest/gtest.h"

#include "vector.hpp"

// 构造
TEST(vector4f_t, ctor) {
    [[maybe_unused]] auto vec1 = vector4f_t();
    [[maybe_unused]] auto vec2 = vector4f_t(1, 2, 3, 4);
    [[maybe_unused]] auto vec3 = vector4f_t(1, 2, 3, 4);
    [[maybe_unused]] auto vec4 = vector4f_t(vec3);

    return;
}

// 加法
TEST(vector4f_t, plus) {
    auto vec1 = vector4f_t(2, 3, 4, 5);
    auto vec2 = vector4f_t(4, 5, 6, 7);
    auto res  = vector4f_t(6, 8, 10, 12);
    EXPECT_EQ((res == (vec1 + vec2)), true);

    auto vec3 = vector4f_t(-1, -100, 101, 233);
    res       = vector4f_t(1, -97, 105, 238);
    EXPECT_EQ((res == (vec1 + vec3)), true);
    return;
}

// 减法
TEST(vector4f_t, sub) {
    auto vec1 = vector4f_t(9, 10, 11, 12);
    auto vec2 = vector4f_t(9, 10, 11, 12);
    auto res  = vector4f_t(0, 0, 0, 0);
    EXPECT_EQ((res == (vec1 - vec2)), true);

    auto vec3 = vector4f_t(-1, -100, 3, 4);
    res       = vector4f_t(10, 110, 8, 8);
    EXPECT_EQ((res == (vec1 - vec3)), true);

    return;
}

// 乘法
TEST(vector4f_t, mult) {
    auto vec1 = vector4f_t(1, 2, 3, 4);
    auto res  = vector4f_t(2, 4, 6, 8);
    EXPECT_EQ((res == (vec1 * 2)), true);

    auto vec2 = vector4f_t(5, 6, 7, 8);
    EXPECT_EQ((70 == (vec1 * vec2)), true);
    EXPECT_EQ((70 == (vec2 * vec1)), true);

    return;
}

// 除法
/// @todo 暂无除法，用 1/a 代替
TEST(vector4f_t, div) {
    auto vec1 = vector4f_t(1, 2, 3, 4);
    auto res  = vector4f_t(2, 4, 6, 8);
    EXPECT_EQ((res == (vec1 * 2)), true);

    auto vec2 = vector4f_t(5, 6, 7, 8);
    EXPECT_EQ((70 == (vec1 * vec2)), true);
    EXPECT_EQ((70 == (vec2 * vec1)), true);

    return;
}

// 逆向量
TEST(vector4f_t, inv) {
    auto vec1 = vector4f_t(9, 10, 11, 12);
    auto res  = vector4f_t(-9, -10, -11, -12);
    EXPECT_EQ((res == (-vec1)), true);

    return;
}

// 长度
TEST(vector4f_t, length) {
    auto vec1 = vector4f_t(9, 10, 11, 12);
    auto res  = std::sqrt(302.);
    EXPECT_EQ((std::abs(vec1.length() - res)
               < std::numeric_limits<decltype(vec1.length())>::epsilon()),
              true);

    auto vec2 = vector4f_t(9, 10, 11, 0);
    res       = std::sqrt(302);
    EXPECT_EQ((std::abs(vec2.length() - res)
               < std::numeric_limits<decltype(vec1.length())>::epsilon()),
              true);

    return;
}

// 标准化
TEST(vector4f_t, normalize) {
    auto vec1 = vector4f_t(9, 10, 11, 12).normalize();
    auto res
      = vector4f_t(0.517891803883592, 0.575435337648436, 0.63297887141328, 12);
    EXPECT_EQ((res == vec1), true);

    auto vec2 = vector4f_t(9, 10, 11, 0).normalize();
    res = vector4f_t(0.517891803883592, 0.575435337648436, 0.63297887141328, 0);
    EXPECT_EQ((res == vec2), true);

    return;
}

// 叉积
TEST(vector4f_t, cross) {
    auto vec1 = vector4f_t(9, 10, 11, 12);
    auto vec2 = vector4f_t(-1, -2, 2, 1);
    auto res  = vector4f_t(42, -29, -8, 12);
    EXPECT_EQ((res == (vec1 ^ vec2)), true);

    res = vector4f_t(-42, 29, 8, 1);
    EXPECT_EQ((res == (vec2 ^ vec1)), true);

    return;
}
