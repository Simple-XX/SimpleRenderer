
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

#include <cstdint>
#include <iostream>
#include <span>
#include <string>
#include <vector>

// #include "display.h"
// #include "render.hpp"
#include <simple_renderer.h>

/// obj 文件目录
static const std::string OBJ_FILE_PATH =
    std::string("/home/parallels/github/MRNIU/SimpleRenderer/obj/");

/// @name 默认大小
/// @{
static constexpr const uint64_t WIDTH = 1920;
static constexpr const uint64_t HEIGHT = 1080;
/// @}

// static void pixel(uint64_t _x, uint64_t _y,
//                   const SimpleRenderer::color_t &_color,
//                   std::array<SimpleRenderer::color_t, WIDTH * HEIGHT>
//                   _buffer) {
//   _buffer[_x + _y * WIDTH] = _color;
// }

// @todo 不应该出现明确的类型，应该使用模板
int main(int, char **) {
  auto array = std::shared_ptr<uint32_t[]>(new uint32_t[100],
                                           std::default_delete<uint32_t[]>());

  auto buffer = std::span<uint32_t>(array.get(), 100);
  auto simple_renderer = simple_renderer::SimpleRenderer(1920, 1080, buffer);

  // obj 路径
  std::vector<std::string> objs;
  objs.emplace_back(OBJ_FILE_PATH + "cube.obj");
  objs.emplace_back(OBJ_FILE_PATH + "cube2.obj");
  objs.emplace_back(OBJ_FILE_PATH + "cube3.obj");
  objs.emplace_back(OBJ_FILE_PATH + "cornell_box.obj");
  objs.emplace_back(OBJ_FILE_PATH + "helmet.obj");
  objs.emplace_back(OBJ_FILE_PATH + "african_head.obj");
  objs.emplace_back(OBJ_FILE_PATH + "utah-teapot/utah-teapot.obj");

  // auto scene = SimpleRenderer::scene_t("default", 1920, 1080, 1000);

  // 读取模型与材质
  for (auto &obj : objs) {
    // 添加到场景中
    auto model = simple_renderer::Model(obj);
    // scene.add_model(model, SimpleRenderer::vector3f_t());
  }
  // 设置光照
  // scene.add_light(SimpleRenderer::light_t());

  // auto buffer = std::array<SimpleRenderer::color_t, WIDTH * HEIGHT>();

  // auto render = SimpleRenderer::render_t<WIDTH, HEIGHT>(scene, buffer,
  // pixel);

  //  auto display = display_t(std::ref(framebuffers));

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

  return 0;
}
