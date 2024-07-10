
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

#include "simple_renderer.h"

#include <cstdint>
#include <iostream>
#include <span>
#include <string>
#include <vector>

#include "Window.hpp"

static constexpr const size_t kWidth = 1920;
static constexpr const size_t kHeight = 1080;


int main(int argc, char **argv) {
  printf("argc: %d\n", argc);
  for (auto i = 0; i < argc; i++) {
    printf("argv[%d]: %s\n", i, argv[i]);
  }

  // auto buffer = std::shared_ptr<uint32_t[]>(new uint32_t[kWidth * kHeight],
                                            // std::default_delete<uint32_t[]>());

  auto window = Window(kWidth, kHeight, "SimpleRenderer");
  
  // main loop
  while(!window.shouldClose()) {
    // render .. 
    window.pollEvents();
    window.swapBuffers();
  }

  return 0;
}
