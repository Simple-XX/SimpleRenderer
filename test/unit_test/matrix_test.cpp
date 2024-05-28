

/**
 * @file matrix_test.cpp
 * @brief matrix.hpp 测试
 * @author Zone.N (Zone.Niuzh@hotmail.com)
 * @version 1.0
 * @date 2022-09-03
 * @copyright MIT LICENSE
 * https://github.com/Simple-XX/SimpleRenderer
 * @par change log:
 * <table>
 * <tr><th>Date<th>Author<th>Description
 * <tr><td>2022-09-03<td>Zone.N<td>创建文件
 * </table>
 */

#include "matrix.hpp"

#include "gtest/gtest.h"

TEST(model_t_scale_half, 函数测试) {
  auto delta_xyz_max = 1.983879f;

  // 缩放倍数
  auto multi = 540;
  // 归一化并乘倍数
  auto scale = multi / delta_xyz_max;
  EXPECT_FLOAT_EQ(scale, 272.19403);

  // 缩放
  auto scale_mat = simple_renderer::Matrix4f();
  scale_mat.setIdentity();
  scale_mat.diagonal()[0] = scale;
  scale_mat.diagonal()[1] = scale;
  scale_mat.diagonal()[2] = scale;
  std::cout << scale_mat << std::endl;

  // 从左上角移动到指定位置
  auto translate_mat = simple_renderer::Matrix4f();
  translate_mat.setIdentity();
  translate_mat(0, 3) = 1920 / 2;
  translate_mat(1, 3) = 1080 / 2;
  translate_mat(2, 3) = 0;
  std::cout << translate_mat << std::endl;

  auto matrix = translate_mat * scale_mat;

  std::cout << matrix << std::endl;
}
