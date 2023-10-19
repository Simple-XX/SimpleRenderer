
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
#include "exception.hpp"
#include "framebuffer.h"
#include "model.h"
#include "render.h"
#include "scene.h"
#include "status.h"

// @todo 不应该出现明确的类型，应该使用模板
auto main(int, char **) -> int {
  log_init();
  // obj 路径
  std::vector<std::string> objs;
  // objs = "../../obj/helmet.obj";
  //  objs = "../../obj/cube.obj";
  //  objs = "../../obj/cube2.obj";
  objs.emplace_back(OBJ_FILE_PATH + "cube3.obj");
  // objs = "../../obj/cornell_box.obj";
  // objs.push_back("../../obj/helmet.obj");
  // objs.push_back("../../obj/african_head.obj");
  // objs.push_back("../../obj/utah-teapot/utah-teapot.obj");

  auto scene = std::make_shared<scene_t>();
  // 读取模型与材质
  for (auto &obj : objs) {
    // 添加到场景中
    auto model = model_t(obj);
    scene->add_model(model);
  }
  // 添加光照
  scene->add_light(light_t());

  auto state = std::make_shared<state_t>();
  auto input = std::make_shared<input_t>();
  std::vector<std::shared_ptr<framebuffer_t>> framebuffers;
  framebuffers.emplace_back(std::make_shared<framebuffer_t>(WIDTH, HEIGHT));
  framebuffers.emplace_back(std::make_shared<framebuffer_t>(WIDTH, HEIGHT));
  framebuffers.emplace_back(std::make_shared<framebuffer_t>(WIDTH, HEIGHT));
  auto render = render_t(std::ref(state), std::ref(scene), std::ref(input),
                         std::ref(framebuffers));
  auto display =
      display_t(std::ref(state), std::ref(input), std::ref(framebuffers));

  // 计算线程
  auto render_ret = render.run();
  // 显示线程
  auto display_ret = display.run();

  if (render_ret.get() != state_t::STOP) {
    throw SimpleRenderer::exception("render thread exit with error");
  }
  if (display_ret.get() != state_t::STOP) {
    throw SimpleRenderer::exception("display thread exit with error");
  }

  return 0;
}
