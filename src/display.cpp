
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

#include <future>
#include <iostream>

#include "camera.h"
#include "config.h"
#include "display.h"
#include "exception.hpp"
#include "log.h"
#include "status.h"

/// @todo
display_t::display_t(
    const std::shared_ptr<state_t> &_state,
    const std::shared_ptr<input_t> &_input,
    const std::vector<std::shared_ptr<framebuffer_t>> &_framebuffers)
    : state(_state), input(_input), framebuffers(_framebuffers), width(WIDTH),
      height(HEIGHT) {
  // 初始化 sdl
  try {
    auto ret = SDL_Init(SDL_INIT_VIDEO);
    if (ret != 0) {
      throw SimpleRenderer::exception(SDL_GetError());
    }
    // 创建窗口，居中，可见
    sdl_window =
        SDL_CreateWindow(WINDOW_TITLE, SDL_WINDOWPOS_CENTERED,
                         SDL_WINDOWPOS_CENTERED, static_cast<int32_t>(width),
                         static_cast<int32_t>(height), SDL_WINDOW_SHOWN);
    if (sdl_window == nullptr) {
      SDL_Quit();
      throw SimpleRenderer::exception(SDL_GetError());
    }

    // 创建渲染器
    sdl_renderer = SDL_CreateRenderer(sdl_window, -1, SDL_RENDERER_SOFTWARE);
    if (sdl_renderer == nullptr) {
      SDL_DestroyWindow(sdl_window);
      SDL_Quit();
      throw SimpleRenderer::exception(SDL_GetError());
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
      throw SimpleRenderer::exception(SDL_GetError());
    }

    // 文字显示
    if (TTF_Init() != 0) {
      SDL_DestroyTexture(sdl_texture);
      SDL_DestroyRenderer(sdl_renderer);
      SDL_DestroyWindow(sdl_window);
      SDL_Quit();
      throw SimpleRenderer::exception(TTF_GetError());
    }
    // 打开字体库
    font = TTF_OpenFont(FONT_FILE_PATH.data(), font_size);
    if (font == nullptr) {
      TTF_Quit();
      SDL_DestroyTexture(sdl_texture);
      SDL_DestroyRenderer(sdl_renderer);
      SDL_DestroyWindow(sdl_window);
      SDL_Quit();
      throw SimpleRenderer::exception(TTF_GetError());
    }
  } catch (const std::runtime_error &e) {
    std::cerr << e.what() << '\n';
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
    throw SimpleRenderer::exception(SDL_GetError());
  }

  // 复制到渲染器
  res = SDL_RenderCopy(sdl_renderer, sdl_texture, nullptr, nullptr);
  if (res != 0) {
    throw SimpleRenderer::exception(SDL_GetError());
  }

  // 刷新
  SDL_RenderPresent(sdl_renderer);
}

/// @todo 验证 std::condition_variable 的正确性
/// @todo 保证时序正确
auto display_t::loop() -> state_t::status_t {
  while (state->status.load() != state_t::STOP) {
    // 记录输入，等待 render 处理
    auto is_running = input->process();
    if (!is_running) {
      state->status.store(state_t::STOP);
    }
    // 等待获取锁
    for (const auto &i : framebuffers) {
      while (!i->displayable.load()) {
        ;
      }
      // 填充窗口
      fill(i);
      i->displayable = false;
    }
  }
  return state_t::STOP;
}

auto display_t::run() -> std::future<state_t::status_t> {
  return std::async(std::launch::async, &display_t::loop, this);
}
