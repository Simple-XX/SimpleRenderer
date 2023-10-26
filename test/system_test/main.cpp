
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

#include <SimpleRenderer.h>

#include "display.h"

/// obj 文件目录
static const std::string OBJ_FILE_PATH =
    std::string("/Users/nzh/Documents/SimpleRenderer/obj/");

/// @name 默认大小
/// @{
static constexpr const size_t WIDTH = 1920;
static constexpr const size_t HEIGHT = 1080;
/// @}

// @todo 不应该出现明确的类型，应该使用模板
auto main(int, char **) -> int {
  // obj 路径
  std::vector<std::string> objs;
  objs.emplace_back(OBJ_FILE_PATH + "cube.obj");
  objs.emplace_back(OBJ_FILE_PATH + "cube2.obj");
  objs.emplace_back(OBJ_FILE_PATH + "cube3.obj");
  objs.emplace_back(OBJ_FILE_PATH + "cornell_box.obj");
  objs.emplace_back(OBJ_FILE_PATH + "helmet.obj");
  objs.emplace_back(OBJ_FILE_PATH + "african_head.obj");
  objs.emplace_back(OBJ_FILE_PATH + "utah-teapot/utah-teapot.obj");

  // auto scene = std::make_shared<scene_t>();
  // // 读取模型与材质
  // for (auto &obj : objs) {
  //   // 添加到场景中
  //   auto model = model_t(obj);
  //   scene->add_model(model);
  // }
  // // 设置光照
  // scene->set_light(light_t());

  // auto state = std::make_shared<state_t>();
  // std::vector<std::shared_ptr<framebuffer_t>> framebuffers;
  // framebuffers.emplace_back(std::make_shared<framebuffer_t>(WIDTH, HEIGHT));
  // framebuffers.emplace_back(std::make_shared<framebuffer_t>(WIDTH, HEIGHT));
  // framebuffers.emplace_back(std::make_shared<framebuffer_t>(WIDTH, HEIGHT));
  // auto render =
  //     render_t(std::ref(state), std::ref(scene), std::ref(framebuffers));
  // auto display = display_t(std::ref(state), std::ref(framebuffers));

  // // 计算线程
  // auto render_ret = render.run();
  // // 显示线程
  // auto display_ret = display.run();

  // if (render_ret.get() != state_t::STOP) {
  //   throw SimpleRenderer::exception("render thread exit with error");
  // }
  // if (display_ret.get() != state_t::STOP) {
  //   throw SimpleRenderer::exception("display thread exit with error");
  // }

  SimpleRenderer::SimpleRenderer<WIDTH, HEIGHT> render;
  for (auto &obj : objs) {
    render.add_model(obj);
  }

  return 0;
}
