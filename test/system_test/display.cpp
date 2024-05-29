
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

#include "display.h"

#include <exception>
#include <future>
#include <iostream>
#include <span>

/// @todo
Display::Display(size_t width, size_t height) : width_(width), height_(height) {
  // 初始化 sdl
  auto ret = SDL_Init(SDL_INIT_VIDEO);
  if (ret != 0) {
    throw std::runtime_error(SDL_GetError());
  }
  // 创建窗口，居中，可见
  sdl_window_ =
      SDL_CreateWindow(kWindowTitle, SDL_WINDOWPOS_CENTERED,
                       SDL_WINDOWPOS_CENTERED, static_cast<int32_t>(width_),
                       static_cast<int32_t>(height_), SDL_WINDOW_SHOWN);
  if (sdl_window_ == nullptr) {
    SDL_Quit();
    throw std::runtime_error(SDL_GetError());
  }

  // 创建渲染器
  sdl_renderer_ = SDL_CreateRenderer(sdl_window_, -1, SDL_RENDERER_SOFTWARE);
  if (sdl_renderer_ == nullptr) {
    SDL_DestroyWindow(sdl_window_);
    SDL_Quit();
    throw std::runtime_error(SDL_GetError());
  }
  // 设置 alpha 通道有效
  SDL_SetRenderDrawBlendMode(sdl_renderer_, SDL_BLENDMODE_BLEND);

  // 创建 texture
  sdl_texture_ = SDL_CreateTexture(
      sdl_renderer_, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STREAMING,
      static_cast<int32_t>(width_), static_cast<int32_t>(height_));
  if (sdl_texture_ == nullptr) {
    SDL_DestroyRenderer(sdl_renderer_);
    SDL_DestroyWindow(sdl_window_);
    SDL_Quit();
    throw std::runtime_error(SDL_GetError());
  }
}

Display::~Display() {
  // 回收资源
  SDL_DestroyTexture(sdl_texture_);
  SDL_DestroyRenderer(sdl_renderer_);
  SDL_DestroyWindow(sdl_window_);
  SDL_Quit();
}

void Display::fill(const std::span<uint32_t> &buffer) {
  // 更新 texture
  auto res = SDL_UpdateTexture(sdl_texture_, nullptr, buffer.data(), 4);
  if (res != 0) {
    throw std::runtime_error(SDL_GetError());
  }

  // 复制到渲染器
  res = SDL_RenderCopy(sdl_renderer_, sdl_texture_, nullptr, nullptr);
  if (res != 0) {
    throw std::runtime_error(SDL_GetError());
  }

  // 刷新
  SDL_RenderPresent(sdl_renderer_);
}

/// @todo 验证 std::condition_variable 的正确性
/// @todo 保证时序正确
void Display::loop(const std::span<uint32_t> &buffer) {
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
            // case SDLK_SPACE: {
            //   break;
            // }
            // case SDLK_TAB: {
            //   state->obj_index++;
            //   std::cout << "obj_index: " << state->obj_index << '\n';
            //   break;
            // }
            // case SDLK_1: {
            //   state->draw_line = !state->draw_line;
            //   std::cout << "draw_line: " << state->draw_line << '\n';
            //   break;
            // }
            // case SDLK_2: {
            //   state->draw_triangle = !state->draw_triangle;
            //   std::cout << "draw_triangle: " << state->draw_triangle << '\n';
            //   break;
            // }
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

    fill(buffer);

    // // 等待获取锁
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

// void Display::run() { std::async(std::launch::async, &Display::loop, this); }
