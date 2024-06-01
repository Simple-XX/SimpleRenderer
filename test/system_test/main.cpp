
/**
 * @file main.cpp
 * @brief 入口
 * @author Zone.N (Zone.Niuzh@hotmail.com)
 * @version 1.0
 * @date 2022-06-06
 * @copyright MIT LICENSE
 * https://github.com/Simple-XX/SimpleRenderer
 * @par change log:
 * <table>
 * <tr><th>Date<th>Author<th>Description
 * <tr><td>2022-06-06<td>Zone.N<td>迁移到 doxygen
 * </table>
 */

#include <simple_renderer.h>

#include <cstdint>
#include <iostream>
#include <span>
#include <string>
#include <vector>

#include "display.h"

/// obj 文件目录
static const std::string OBJ_FILE_PATH =
    std::string("/home/parallels/github/MRNIU/SimpleRenderer/obj/");

/// @name 默认大小
/// @{
static constexpr const size_t kWidth = 1920;
static constexpr const size_t kHeight = 1080;
/// @}

static void pixel(size_t x, size_t y, uint32_t color, uint32_t *buffer) {
  buffer[x + y * kWidth] = color;
}

int main(int argc, char **argv) {
  auto buffer = std::shared_ptr<uint32_t[]>(new uint32_t[kWidth * kHeight],
                                            std::default_delete<uint32_t[]>());

  auto simple_renderer =
      simple_renderer::SimpleRenderer(kWidth, kHeight, buffer.get(), pixel);

  // obj 路径
  std::vector<std::string> objs;
  // objs.emplace_back(OBJ_FILE_PATH + "cube.obj");
  // objs.emplace_back(OBJ_FILE_PATH + "cube2.obj");
  // objs.emplace_back(OBJ_FILE_PATH + "cube3.obj");
  // objs.emplace_back(OBJ_FILE_PATH + "cornell_box.obj");
  // objs.emplace_back(OBJ_FILE_PATH + "helmet.obj");
  // objs.emplace_back(OBJ_FILE_PATH + "african_head.obj");
  objs.emplace_back(OBJ_FILE_PATH + "utah-teapot/utah-teapot.obj");

  auto matrix =
      simple_renderer::Matrix4f(simple_renderer::Matrix4f::Identity());
  matrix.diagonal() << 500, 500, 500, 1;
  matrix.col(matrix.cols() - 1) << kWidth / 2, kHeight / 2, 0, 1;

  // 矩阵运算的顺序
  // 归一化
  // 坐标空间

  /// @todo 旋转
  // 旋转轴+旋转角度
  // 四元数
  // (0.3, 0.2, 0.1) 90
  // Tran/Scal/rota
  // 1. 物体原地旋转
  // 2. 移动到屏幕中央
  // 3. 移动到屏幕左上角
  // 4. 缩放物体

  // 读取模型与材质
  for (auto &obj : objs) {
    // 添加到场景中
    auto model = simple_renderer::Model(obj);
    model = model * matrix;
    simple_renderer.render(model);
  }

  auto display = Display(kWidth, kHeight);
  display.loop(buffer.get());

  return 0;
}
