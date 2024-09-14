
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
  // auto obj_path = std::string(
  // "/Users/hezhohao/Programming/GitRepo/SimpleRenderer/obj");  // just for
  // debuging

  simple_renderer::Buffer buffer(kWidth, kHeight);

  auto simple_renderer = simple_renderer::SimpleRenderer(
      kWidth, kHeight, buffer.GetFramebuffer(), pixel);

  // obj 路径
  std::vector<std::string> objs;
  objs.emplace_back(obj_path + "/utah-teapot-texture/teapot.obj");
  // add more obj .. if you want
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

  simple_renderer::Shader shader;
  shader.SetUniform("model_matrix", matrix);
  shader.SetUniform("view_matrix", simple_renderer::Matrix4f(1.0f));
  shader.SetUniform("projection_matrix", simple_renderer::Matrix4f(1.0f));
  simple_renderer::Light light;
  shader.SetUniform("light", light);

  for (auto &model : models) {
    simple_renderer.render(model, shader);
  }

  auto display = Display(kWidth, kHeight);
  display.loopBegin();
  while (!display.loopShouldClose()) {
    display.handleEvents();

    display.fill(buffer.GetFramebuffer());
  }

  return 0;
}
