
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

#include "vector.hpp"
#include "gtest/gtest.h"

TEST(vector4f_t, test1) {
    vector4f_t test(2, 3);
    vector4f_t test2(-1, 3);
    vector4f_t test3 = test - test2;
    EXPECT_EQ(test.x, 2);
    EXPECT_EQ(test.y, 3);
    EXPECT_EQ(test2.x, -1);
    EXPECT_EQ(test2.y, 3);
    EXPECT_EQ(test3.x, 3);
    EXPECT_EQ(test3.y, 0);
    EXPECT_EQ((test3 * 3).x, 9);
    EXPECT_EQ((test3 * 3).y, 0);
    EXPECT_EQ((vector4f_t(0, 1) * vector4f_t(1, 0)), 0);
    EXPECT_EQ(test.length_squared(), 13);
    EXPECT_EQ((-test).x, -2);
    EXPECT_EQ((-test).y, -3);
    vector4f_t test5(2, 3, 4);
    vector4f_t test6(-1, 3, -2);
    vector4f_t test7 = test5 - test6;
    EXPECT_EQ(test5.x, 2);
    EXPECT_EQ(test5.y, 3);
    EXPECT_EQ(test5.z, 4);
    EXPECT_EQ(test6.x, -1);
    EXPECT_EQ(test6.y, 3);
    EXPECT_EQ(test6.z, -2);
    EXPECT_EQ(test7.x, 3);
    EXPECT_EQ(test7.y, 0);
    EXPECT_EQ(test7.z, 6);
    EXPECT_EQ((test7 * 3).x, 9);
    EXPECT_EQ((test7 * 3).y, 0);
    EXPECT_EQ((test7 * 3).z, 18);
    EXPECT_EQ((vector4f_t(0, 1, 0) * vector4f_t(1, 0, 0)), 0);
    EXPECT_EQ(test5.length_squared(), 29);
    EXPECT_EQ((-test5).x, -2);
    EXPECT_EQ((-test5).y, -3);
    EXPECT_EQ((-test5).z, -4);
    vector4f_t test8;
    vector4f_t test9(1, 1, 1);
    test9 = test5 ^ test6;
    EXPECT_EQ(test9.x, -18);
    EXPECT_EQ(test9.y, 0);
    EXPECT_EQ(test9.z, 9);
    vector4f_t test10(1, 2, 3);
    EXPECT_EQ(test10.x, 1);
    EXPECT_EQ(test10.y, 2);
    EXPECT_EQ(test10.z, 3);
    vector4f_t test11(test10);
    EXPECT_EQ(test11.x, 1);
    EXPECT_EQ(test11.y, 2);
    EXPECT_EQ(test11.z, 3);
    return;
}
