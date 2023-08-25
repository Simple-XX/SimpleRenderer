
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

#include <iostream>

#include "camera.h"
#include "config.h"
#include "display.h"
#include "log.hpp"

display_t::display_t(size_t _width, size_t _height)
    : width(_width), height(_height) {
  // 初始化 sdl
  try {
    auto ret = SDL_Init(SDL_INIT_VIDEO);
    if (ret != 0) {
      throw std::runtime_error(log(SDL_GetError()));
    }
    // 创建窗口，居中，可见
    sdl_window =
        SDL_CreateWindow(WINDOW_TITLE, SDL_WINDOWPOS_CENTERED,
                         SDL_WINDOWPOS_CENTERED, static_cast<int32_t>(width),
                         static_cast<int32_t>(height), SDL_WINDOW_SHOWN);
    if (sdl_window == nullptr) {
      SDL_Quit();
      throw std::runtime_error(log(SDL_GetError()));
    }

    // 创建渲染器
    sdl_renderer = SDL_CreateRenderer(sdl_window, -1, SDL_RENDERER_SOFTWARE);
    if (sdl_renderer == nullptr) {
      SDL_DestroyWindow(sdl_window);
      SDL_Quit();
      throw std::runtime_error(log(SDL_GetError()));
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
      throw std::runtime_error(log(SDL_GetError()));
    }

    // 文字显示
    if (TTF_Init() != 0) {
      SDL_DestroyTexture(sdl_texture);
      SDL_DestroyRenderer(sdl_renderer);
      SDL_DestroyWindow(sdl_window);
      SDL_Quit();
      throw std::runtime_error(log(TTF_GetError()));
    }
    // 打开字体库
    std::cout << FONT_FILE_PATH << std::endl;
    font = TTF_OpenFont(FONT_FILE_PATH.data(), font_size);
    if (font == nullptr) {
      TTF_Quit();
      SDL_DestroyTexture(sdl_texture);
      SDL_DestroyRenderer(sdl_renderer);
      SDL_DestroyWindow(sdl_window);
      SDL_Quit();
      throw std::runtime_error(log(TTF_GetError()));
    }
  } catch (const std::runtime_error &e) {
    std::cerr << e.what() << std::endl;
    return;
  }
}

display_t::~display_t() {
  // 回收资源
  TTF_CloseFont(font);
  TTF_Quit();
  SDL_DestroyTexture(sdl_texture);
  SDL_DestroyRenderer(sdl_renderer);
  SDL_DestroyWindow(sdl_window);
  SDL_Quit();
}

void display_t::fill(const std::shared_ptr<framebuffer_t> &_framebuffer) {
  // 更新 texture
  auto res = SDL_UpdateTexture(sdl_texture, nullptr,
                               _framebuffer->get_color_buffer().data(),
                               static_cast<int32_t>(width * color_t::bpp()));
  if (res != 0) {
    throw std::runtime_error(log(SDL_GetError()));
  }

  // 复制到渲染器
  res = SDL_RenderCopy(sdl_renderer, sdl_texture, nullptr, nullptr);
  if (res != 0) {
    throw std::runtime_error(log(SDL_GetError()));
  }

  // 刷新
  SDL_RenderPresent(sdl_renderer);
}
