
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

#include <iostream>
#include <span>
#include <string>
#include <vector>

#include "config.h"
#include "model.h"
#include "render.h"
#include "scene.h"

auto display = std::make_shared<display_t>(WIDTH, HEIGHT);
auto scene = std::make_shared<scene_t>();
auto input = std::make_shared<input_t>();
auto shader = std::make_shared<default_shader_t>();
auto render = std::make_shared<render_t>(scene, display, input);

auto main(int _argc, char **_argv) -> int {
  auto paras = std::span(_argv, _argc);
  // obj 路径
  std::vector<std::string> objs;
  // 如果没有指定那么使用默认值
  if (paras.size() == 1) {
    // objs = "../../obj/helmet.obj";
    //  objs = "../../obj/cube.obj";
    //  objs = "../../obj/cube2.obj";
    objs.emplace_back(OBJ_FILE_PATH + "cube3.obj");
    // objs = "../../obj/cornell_box.obj";
    // objs.push_back("../../obj/helmet.obj");
    // objs.push_back("../../obj/african_head.obj");
    // objs.push_back("../../obj/utah-teapot/utah-teapot.obj");
  }
  // 否则使用指定的
  else {
    for (auto *para : paras) {
      objs.emplace_back(para);
    }
  }

  // 读取模型与材质
  for (auto &obj : objs) {
    std::cout << obj << '\n';
    model_t const model(obj);

    scene->add_model(model);
  }
  scene->add_light(light_t());

  render->loop();

  return 0;
}
