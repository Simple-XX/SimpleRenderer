
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
    color_t rgba((uint8_t)1, 2, 3, 40, color_t::COLOR_ORDER_RGBA);
    EXPECT_EQ(rgba[0], 1);
    EXPECT_EQ(rgba[1], 2);
    EXPECT_EQ(rgba[2], 3);
    EXPECT_EQ(rgba[3], 40);

    auto rgba2rgba = rgba.to_rgba();
    EXPECT_EQ(rgba2rgba[0], 1);
    EXPECT_EQ(rgba2rgba[1], 2);
    EXPECT_EQ(rgba2rgba[2], 3);
    EXPECT_EQ(rgba2rgba[3], 40);

    auto rgba2argb = rgba.to_argb();
    EXPECT_EQ(rgba2argb[0], 40);
    EXPECT_EQ(rgba2argb[1], 1);
    EXPECT_EQ(rgba2argb[2], 2);
    EXPECT_EQ(rgba2argb[3], 3);

    auto rgba2arr = rgba.to_arr();
    EXPECT_EQ(rgba2arr[0], 1);
    EXPECT_EQ(rgba2arr[1], 2);
    EXPECT_EQ(rgba2arr[2], 3);
    EXPECT_EQ(rgba2arr[3], 40);

    return;
}

TEST(color_t, argb) {
    color_t argb((uint8_t)40, 1, 2, 3, color_t::COLOR_ORDER_ARGB);

    EXPECT_EQ(argb[0], 40);
    EXPECT_EQ(argb[1], 1);
    EXPECT_EQ(argb[2], 2);
    EXPECT_EQ(argb[3], 3);

    auto argb2rgba = argb.to_rgba();
    EXPECT_EQ(argb2rgba[0], 1);
    EXPECT_EQ(argb2rgba[1], 2);
    EXPECT_EQ(argb2rgba[2], 3);
    EXPECT_EQ(argb2rgba[3], 40);

    auto argb2argb = argb.to_argb();
    EXPECT_EQ(argb2argb[0], 40);
    EXPECT_EQ(argb2argb[1], 1);
    EXPECT_EQ(argb2argb[2], 2);
    EXPECT_EQ(argb2argb[3], 3);

    auto argb2arr = argb.to_arr();
    EXPECT_EQ(argb2arr[0], 40);
    EXPECT_EQ(argb2arr[1], 1);
    EXPECT_EQ(argb2arr[2], 2);
    EXPECT_EQ(argb2arr[3], 3);

    return;
}
