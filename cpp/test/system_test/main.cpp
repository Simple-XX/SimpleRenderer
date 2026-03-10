
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

#include <renderer.h>

#include <cstdint>
#include <iostream>
#include <span>
#include <string>
#include <vector>

#include "buffer.hpp"
#include "camera.h"
#include "display.h"

/// @name 默认大小
/// @{
static constexpr const size_t kWidth = 800;
static constexpr const size_t kHeight = 600;
/// @}

/// usage:
/// ./bin/system_test ../obj
int main(int argc, char **argv) {
  printf("argc: %d\n", argc);
  for (auto i = 0; i < argc; i++) {
    printf("argv[%d]: %s\n", i, argv[i]);
  }
  auto obj_path = std::string(argv[1]);

  simple_renderer::Buffer buffer(kWidth, kHeight);

  auto simple_renderer = simple_renderer::SimpleRenderer(kWidth, kHeight);

  // obj 路径
  std::vector<std::string> objs;
  objs.emplace_back(obj_path + "/utah-teapot-texture/teapot.obj");
  // add more obj .. if you want
  // load model
  std::vector<simple_renderer::Model> models;
  for (auto &obj : objs) {
    models.emplace_back(simple_renderer::Model(obj));
  }

  auto modelMatrix = simple_renderer::Matrix4f(1.0f);
  
  simple_renderer::Matrix4f scale_matrix =
      glm::scale(simple_renderer::Matrix4f(1.0f),
                 simple_renderer::Vector3f(.02f, .02f, .02f));

  simple_renderer::Matrix4f translation_matrix =
      glm::translate(simple_renderer::Matrix4f(1.0f),
                     simple_renderer::Vector3f(0.0f, -5.0f, 0.0f));

  simple_renderer::Matrix4f rotation_matrix =
      glm::rotate(simple_renderer::Matrix4f(1.0f), glm::radians(-105.0f),
                  simple_renderer::Vector3f(1.0f, 0.0f, 0.0f));

  modelMatrix = scale_matrix* translation_matrix * rotation_matrix ;

  simple_renderer::Shader shader;
  shader.SetUniform("modelMatrix", modelMatrix);

  // 多光源
  std::vector<simple_renderer::Light> lights;
  {
    simple_renderer::Light l0; l0.dir = simple_renderer::Vector3f( 1.0f,  5.0f,  1.0f); lights.push_back(l0);
    simple_renderer::Light l1; l1.dir = simple_renderer::Vector3f(-3.0f, -2.0f,  2.0f); lights.push_back(l1);
    simple_renderer::Light l2; l2.dir = simple_renderer::Vector3f( 2.0f,  1.0f, -1.0f); lights.push_back(l2);
  }
  shader.SetLights(lights);

  simple_renderer::Camera camera(simple_renderer::Vector3f(0.0f, 0.0f, 1.0f));

  // 设置渲染模式（可选：PER_TRIANGLE、TILE_BASED 或 DEFERRED）
  simple_renderer.SetRenderingMode(simple_renderer::RenderingMode::TILE_BASED);
  
  // 输出当前渲染模式
  std::string current_mode_name = simple_renderer::RenderingModeToString(
      simple_renderer.GetRenderingMode());
  SPDLOG_INFO("当前渲染模式: {}", current_mode_name);

  auto display = Display(kWidth, kHeight);
  display.loopBegin();

  while (!display.loopShouldClose()) {
    display.handleEvents(camera);

    shader.SetUniform("cameraPos", camera.GetPosition());
    shader.SetUniform("viewMatrix", camera.GetViewMatrix());
    shader.SetUniform("projectionMatrix",
                      camera.GetProjectionMatrix(60.0f, static_cast<float>(kWidth) / static_cast<float>(kHeight), 0.1f, 100.0f));

    buffer.ClearDrawBuffer(simple_renderer::Color::kBlack);
    for (auto &model : models) {
      simple_renderer.DrawModel(model, shader, buffer.GetDrawBuffer());
    }

    buffer.SwapBuffer();

    display.fill(buffer.GetDisplayBuffer());
  }

  return 0;
}
