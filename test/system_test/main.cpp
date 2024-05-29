
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

static void pixel(size_t x, size_t y, uint32_t color,
                  std::span<uint32_t> &buffer) {
  buffer[x + y * kWidth] = color;
}

int main(int, char **) {
  auto array = std::shared_ptr<uint32_t[]>(new uint32_t[kWidth * kHeight],
                                           std::default_delete<uint32_t[]>());

  auto buffer = std::span<uint32_t>(array.get(), kWidth * kHeight);
  auto simple_renderer =
      simple_renderer::SimpleRenderer(kWidth, kHeight, buffer, pixel);

  // obj 路径
  std::vector<std::string> objs;
  objs.emplace_back(OBJ_FILE_PATH + "cube.obj");

  // 读取模型与材质
  for (auto &obj : objs) {
    // 添加到场景中
    auto model = simple_renderer::Model(obj);
    simple_renderer.render(model);
  }

  auto display = Display(kWidth, kHeight);
  display.loop(buffer);

  return 0;
}
