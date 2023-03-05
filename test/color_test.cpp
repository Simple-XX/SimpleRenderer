
/**
 * @file color_test.cpp
 * @brief color_t 测试
 * @author Zone.N (Zone.Niuzh@hotmail.com)
 * @version 1.0
 * @date 2022-11-02
 * @copyright MIT LICENSE
 * https://github.com/Simple-XX/SimpleRenderer
 * @par change log:
 * <table>
 * <tr><th>Date<th>Author<th>Description
 * <tr><td>2022-11-02<td>Zone.N<td>创建文件
 * </table>
 */

#include "gtest/gtest.h"

#include "color.h"

TEST(color_t, rgba) {
    color_t rgba((uint8_t)1, 2, 3, 40);
    EXPECT_EQ(rgba[0], 1);
    EXPECT_EQ(rgba[1], 2);
    EXPECT_EQ(rgba[2], 3);
    EXPECT_EQ(rgba[3], 40);

    return;
}
