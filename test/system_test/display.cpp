
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
  sdl_renderer_ = SDL_CreateRenderer(sdl_window_, -1, SDL_RENDERER_ACCELERATED);
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

void Display::fill(const uint32_t* buffer) {
  // 更新 texture
  auto res = SDL_UpdateTexture(sdl_texture_, nullptr, buffer, width_ * 4);
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

void Display::loopBegin() { is_exit_ = false; }

bool Display::loopShouldClose() { return is_exit_; }

void Display::handleKeyboardEvent(SDL_Event& event,
                                  simple_renderer::Camera& camera) {
  switch (event.key.keysym.sym) {
    case SDLK_ESCAPE:
    case SDLK_q:
      is_exit_ = true;
      break;
    case SDLK_UP:
      camera.MoveUp(1.0f);
      break;
    case SDLK_DOWN:
      camera.MoveUp(-1.0f);
      break;
    case SDLK_LEFT:
      camera.MoveRight(-1.0f);
      break;
    case SDLK_RIGHT:
      camera.MoveRight(1.0f);
      break;
    default:
      break;
  }
}

void Display::handleEvents(simple_renderer::Camera& camera) {
  SDL_Event event = SDL_Event();
  while (SDL_PollEvent(&event) != 0) {
    switch (event.type) {
      case SDL_QUIT:
        is_exit_ = true;
        break;
      case SDL_KEYDOWN:
        handleKeyboardEvent(event, camera);
        break;
    }
  }
}