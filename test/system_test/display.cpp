
/**
 * @file display.cpp
 * @brief 显示
 * @author Zone.N (Zone.Niuzh@hotmail.com)
 * @version 1.0
 * @date 2022-09-05
 * @copyright MIT LICENSE
 * https://github.com/Simple-XX/SimpleRenderer
 * @par change log:
 * <table>
 * <tr><th>Date<th>Author<th>Description
 * <tr><td>2022-09-05<td>Zone.N<td>创建文件
 * </table>
 */

#include <exception>
#include <future>
#include <iostream>

#include "display.h"

/// @todo
display_t::display_t(size_t _width, size_t _height)
    : width(_width), height(_height) {
  // 初始化 sdl
  auto ret = SDL_Init(SDL_INIT_VIDEO);
  if (ret != 0) {
    throw std::runtime_error(SDL_GetError());
  }
  // 创建窗口，居中，可见
  sdl_window =
      SDL_CreateWindow(WINDOW_TITLE, SDL_WINDOWPOS_CENTERED,
                       SDL_WINDOWPOS_CENTERED, static_cast<int32_t>(width),
                       static_cast<int32_t>(height), SDL_WINDOW_SHOWN);
  if (sdl_window == nullptr) {
    SDL_Quit();
    throw std::runtime_error(SDL_GetError());
  }

  // 创建渲染器
  sdl_renderer = SDL_CreateRenderer(sdl_window, -1, SDL_RENDERER_SOFTWARE);
  if (sdl_renderer == nullptr) {
    SDL_DestroyWindow(sdl_window);
    SDL_Quit();
    throw std::runtime_error(SDL_GetError());
  }
  // 设置 alpha 通道有效
  SDL_SetRenderDrawBlendMode(sdl_renderer, SDL_BLENDMODE_BLEND);

  // 创建 texture
  sdl_texture = SDL_CreateTexture(
      sdl_renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STREAMING,
      static_cast<int32_t>(width), static_cast<int32_t>(height));
  if (sdl_texture == nullptr) {
    SDL_DestroyRenderer(sdl_renderer);
    SDL_DestroyWindow(sdl_window);
    SDL_Quit();
    throw std::runtime_error(SDL_GetError());
  }
}

display_t::~display_t() {
  // 回收资源
  SDL_DestroyTexture(sdl_texture);
  SDL_DestroyRenderer(sdl_renderer);
  SDL_DestroyWindow(sdl_window);
  SDL_Quit();
}

void display_t::fill(
    const std::shared_ptr<SimpleRenderer::framebuffer_t> &_framebuffer) {
  // 更新 texture
  auto res = SDL_UpdateTexture(
      sdl_texture, nullptr, _framebuffer->get_color_buffer().data(),
      static_cast<int32_t>(width * _framebuffer->get_color_buffer().BPP));
  if (res != 0) {
    throw std::runtime_error(SDL_GetError());
  }

  // 复制到渲染器
  res = SDL_RenderCopy(sdl_renderer, sdl_texture, nullptr, nullptr);
  if (res != 0) {
    throw std::runtime_error(SDL_GetError());
  }

  // 刷新
  SDL_RenderPresent(sdl_renderer);
}

/// @todo 验证 std::condition_variable 的正确性
/// @todo 保证时序正确
void display_t::loop() {
  SDL_Event event = SDL_Event();
  bool is_exit = false;
  while (is_exit == false) {
    bool is_mouse_down = false;

    while (SDL_PollEvent(&event) != 0) {
      if (event.type == SDL_QUIT) {
        is_exit = true;
      }
      switch (event.type) {
      // 键盘事件
      case SDL_KEYDOWN: {
        switch (event.key.keysym.sym) {
        case SDLK_SPACE: {
          break;
        }
        case SDLK_TAB: {
          state->obj_index++;
          std::cout << "obj_index: " << state->obj_index << '\n';
          break;
        }
        case SDLK_1: {
          state->draw_line = !state->draw_line;
          std::cout << "draw_line: " << state->draw_line << '\n';
          break;
        }
        case SDLK_2: {
          state->draw_triangle = !state->draw_triangle;
          std::cout << "draw_triangle: " << state->draw_triangle << '\n';
          break;
        }
        default: {
          // 输出按键名
          std::cout << "key " << SDL_GetKeyName(event.key.keysym.sym)
                    << " down!\n";
          break;
        }
        }
        break;
      }
      // 鼠标移动
      case SDL_MOUSEMOTION: {
        // 鼠标拖动
        if (is_mouse_down) {
          std::cout << "鼠标拖动 " << event.motion.xrel << " "
                    << event.motion.yrel << '\n';
        }
        std::cout << "鼠标移动 " << event.motion.xrel << " "
                  << event.motion.yrel << '\n';
        break;
      }
      // 鼠标按下
      case SDL_MOUSEBUTTONDOWN: {
        is_mouse_down = true;
        std::cout << "鼠标点击 " << event.button.x << " " << event.button.y
                  << '\n';
        break;
      }
        // 鼠标释放
      case SDL_MOUSEBUTTONUP: {
        is_mouse_down = false;
        break;
      }
      }
    }

    // 等待获取锁
    // for (const auto &i : framebuffers) {
    //   while (!i->displayable.load()) {
    //     ;
    //   }
    //   // 填充窗口
    //   fill(i);
    //   i->displayable = false;
    // }
  }
}

void display_t::run() {
  std::async(std::launch::async, &display_t::loop, this);
}
