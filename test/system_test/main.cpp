
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

#include "buffer.hpp"
#include "display.h"

/// @name 默认大小
/// @{
static constexpr const size_t kWidth = 800;
static constexpr const size_t kHeight = 600;
/// @}

static void pixel(size_t x, size_t y, uint32_t color, uint32_t *buffer) {
  buffer[x + y * kWidth] = color;
}

/// usage:
/// ./bin/system_test ../obj
int main(int argc, char **argv) {
  printf("argc: %d\n", argc);
  for (auto i = 0; i < argc; i++) {
    printf("argv[%d]: %s\n", i, argv[i]);
  }
  auto obj_path = std::string(argv[1]);

  simple_renderer::Buffer buffer(kWidth, kHeight);

  auto simple_renderer = simple_renderer::SimpleRenderer(
      kWidth, kHeight, buffer.framebuffer(), pixel);

  // obj 路径
  std::vector<std::string> objs;
  // objs.emplace_back(obj_path + "/cube.obj");
  // objs.emplace_back(obj_path + "/cube2.obj");
  // objs.emplace_back(obj_path + "/cube3.obj");
  // objs.emplace_back(obj_path + "/cornell_box.obj");
  // objs.emplace_back(obj_path + "/helmet.obj");
  // objs.emplace_back(obj_path + "/african_head.obj");
  objs.emplace_back(obj_path + "/utah-teapot-texture/teapot.obj");
  // load model
  std::vector<simple_renderer::Model> models;
  for (auto &obj : objs) {
    models.emplace_back(simple_renderer::Model(obj));
  }

  auto matrix = simple_renderer::Matrix4f(1.0f);
  simple_renderer::Matrix4f scale_matrix =
      glm::scale(simple_renderer::Matrix4f(1.0f),
                 simple_renderer::Vector3f(10.0f, 10.0f, 10.0f));

  // Translation matrix
  simple_renderer::Matrix4f translation_matrix = glm::translate(
      simple_renderer::Matrix4f(1.0f),
      simple_renderer::Vector3f(kWidth / 2.0f, kHeight / 2.0f, 0.0f));

  simple_renderer::Matrix4f rotation_matrix =
      glm::rotate(simple_renderer::Matrix4f(1.0f), 90.0f,
                  simple_renderer::Vector3f(1.0f, 0.0f, 0.0f));

  // Combined transformation matrix
  matrix = translation_matrix * scale_matrix * rotation_matrix;

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

  for (auto &model : models) {
    model.transform(matrix);
    simple_renderer.render(model);
  }

  auto display = Display(kWidth, kHeight);
  display.loopBegin();
  while (!display.loopShouldClose()) {
    display.handleEvents();

    display.fill(buffer.framebuffer());
  }

  return 0;
}
