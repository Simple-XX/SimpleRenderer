
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
#include "framebuffer.h"
#include "model.h"
#include "render.h"
#include "scene.h"
#include "status.h"


#include "spdlog/spdlog.h"
/// @todo 不应该出现明确的类型，应该使用模板

auto main(int _argc, char **_argv) -> int {
  // log_init();

  spdlog::info("Welcome to spdlog!");
  spdlog::error("Some error message with arg: {}", 1);
  
  spdlog::warn("Easy padding in numbers like {:08d}", 12);
  spdlog::critical("Support for int: {0:d};  hex: {0:x};  oct: {0:o}; bin: {0:b}", 42);
  spdlog::info("Support for floats {:03.2f}", 1.23456);
  spdlog::info("Positional args are {1} {0}..", "too", "supported");
  spdlog::info("{:<30}", "left aligned");
  
  spdlog::set_level(spdlog::level::debug); // Set global log level to debug
  spdlog::debug("This message should be displayed..");    
  
  // change log pattern
  spdlog::set_pattern("[%H:%M:%S %z] [%n] [%^---%L---%$] [thread %t] %v");
  
  // Compile time log levels
  // define SPDLOG_ACTIVE_LEVEL to desired level
  SPDLOG_TRACE("Some trace message with param {}", 42);
  SPDLOG_DEBUG("Some debug message");

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

  auto scene = std::make_shared<scene_t>();
  // 读取模型与材质
  for (auto &obj : objs) {
    std::cout << obj << '\n';
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
  auto display = display_t(std::ref(state), std::ref(framebuffers));

  // 计算线程
  auto render_ret = render.run();
  // 显示线程
  auto display_ret = display.run();

  if (render_ret.get() != state_t::STOP) {
    std::cout << "render thread exit with error\n";
  }
  if (display_ret.get() != state_t::STOP) {
    std::cout << "display thread exit with error\n";
  }

  return 0;
}
