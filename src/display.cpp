
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

#include "iostream"

#include "camera.h"
#include "display.h"
#include "log.hpp"

void display_t::fill(void) {
    // 设置清屏颜色
    auto res = SDL_SetRenderDrawColor(sdl_renderer, 0, 0, 0, 255);
    if (res != 0) {
        throw std::runtime_error(log(SDL_GetError()));
    }
    // 清屏
    res = SDL_RenderClear(sdl_renderer);
    if (res != 0) {
        throw std::runtime_error(log(SDL_GetError()));
    }

    // 更新 texture
    res = SDL_UpdateTexture(sdl_texture, nullptr,
                            framebuffer.get_color_buffer().to_arr(),
                            width * color_t::bpp());
    if (res != 0) {
        throw std::runtime_error(log(SDL_GetError()));
    }

    // 复制到渲染器
    res = SDL_RenderCopy(sdl_renderer, sdl_texture, nullptr, nullptr);
    if (res != 0) {
        throw std::runtime_error(log(SDL_GetError()));
    }

    return;
}

void display_t::show_fps(const uint32_t _fps) {
    // 生成字符串
    auto fps_text    = FPS + std::to_string(_fps);
    // 生成 surface
    auto fps_surface = TTF_RenderText_Solid(font, fps_text.c_str(), fps_color);
    if (fps_surface == nullptr) {
        throw std::runtime_error(log(TTF_GetError()));
    }
    // 生成 texture
    auto msg = SDL_CreateTextureFromSurface(sdl_renderer, fps_surface);
    if (msg == nullptr) {
        throw std::runtime_error(log(SDL_GetError()));
    }
    // 复制到 renderer
    auto res = SDL_RenderCopy(sdl_renderer, msg, nullptr, &FPS_RECT);
    if (res != 0) {
        throw std::runtime_error(log(SDL_GetError()));
    }

    // 释放资源
    SDL_FreeSurface(fps_surface);
    SDL_DestroyTexture(msg);
    return;
}

display_t::display_t(framebuffer_t& _framebuffer, camera_t& _camera,
                     event_callback_t& _event_callback)
    : framebuffer(_framebuffer),
      camera(_camera),
      event_callback(_event_callback) {
    width  = framebuffer.get_width();
    height = framebuffer.get_height();
    // 初始化 sdl
    try {
        auto ret = SDL_Init(SDL_INIT_VIDEO);
        if (ret != 0) {
            throw std::runtime_error(log(SDL_GetError()));
        }
        // 创建窗口，居中，可见
        sdl_window = SDL_CreateWindow(WINDOW_TITLE, SDL_WINDOWPOS_CENTERED,
                                      SDL_WINDOWPOS_CENTERED, width, height,
                                      SDL_WINDOW_SHOWN);
        if (sdl_window == nullptr) {
            SDL_Quit();
            throw std::runtime_error(log(SDL_GetError()));
        }

        // 创建渲染器
        sdl_renderer
          = SDL_CreateRenderer(sdl_window, -1, SDL_RENDERER_SOFTWARE);
        if (sdl_renderer == nullptr) {
            SDL_DestroyWindow(sdl_window);
            SDL_Quit();
            throw std::runtime_error(log(SDL_GetError()));
        }
        // 设置 alpha 通道有效
        SDL_SetRenderDrawBlendMode(sdl_renderer, SDL_BLENDMODE_BLEND);

        // 创建 texture
        sdl_texture
          = SDL_CreateTexture(sdl_renderer, SDL_PIXELFORMAT_RGBA32,
                              SDL_TEXTUREACCESS_STREAMING, width, height);
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
        font = TTF_OpenFont(font_file_path, font_size);
        if (font == nullptr) {
            TTF_Quit();
            SDL_DestroyTexture(sdl_texture);
            SDL_DestroyRenderer(sdl_renderer);
            SDL_DestroyWindow(sdl_window);
            SDL_Quit();
            throw std::runtime_error(log(TTF_GetError()));
        }
    } catch (const std::runtime_error& e) {
        std::cerr << e.what() << std::endl;
        return;
    }

    return;
}

display_t::~display_t(void) {
    // 回收资源
    TTF_CloseFont(font);
    TTF_Quit();
    SDL_DestroyTexture(sdl_texture);
    SDL_DestroyRenderer(sdl_renderer);
    SDL_DestroyWindow(sdl_window);
    SDL_Quit();
}

void display_t::input_handler(void) {
    // 捕获事件
    while (SDL_PollEvent(&sdl_event)) {
        switch (sdl_event.type) {
            // 键盘事件
            case SDL_KEYDOWN: {
                switch (sdl_event.key.keysym.sym) {
                    case SDLK_ESCAPE: {
                        // 如果是 esc 键则将 is_should_quit 置位
                        is_should_quit = true;
                        break;
                    }
                    case SDLK_a: {
                        event_callback.key_a();
                        break;
                    }
                    case SDLK_d: {
                        event_callback.key_d();
                        break;
                    }
                    case SDLK_SPACE: {
                        event_callback.key_space();
                        break;
                    }
                    case SDLK_z: {
                        event_callback.key_z();
                        break;
                    }
                    case SDLK_LCTRL: {
                        event_callback.key_left_ctrl();
                        break;
                    }
                    case SDLK_w: {
                        event_callback.key_w();
                        break;
                    }
                    case SDLK_s: {
                        event_callback.key_s();
                        break;
                    }
                    case SDLK_LSHIFT: {
                        event_callback.key_left_shift();
                        break;
                    }
                    default: {
                        // 输出按键名
                        printf("key %s down！\n",
                               SDL_GetKeyName(sdl_event.key.keysym.sym));
                    }
                }
                break;
            }
            // 鼠标移动
            case SDL_MOUSEMOTION: {
                event_callback.key_mouse_motion();
                break;
            }
            // 鼠标点击
            case SDL_MOUSEBUTTONDOWN: {
                break;
            }
            // 鼠标滚轮
            case SDL_MOUSEWHEEL: {
                break;
            }
            case SDL_QUIT: {
                is_should_quit = true;
                break;
            }
        }
    }
    return;
}

void display_t::loop(void) {
    uint64_t sec    = 0;
    uint32_t frames = 0;
    uint32_t fps    = 0;
    auto     start  = us();
    auto     end    = us();
    // 主循环
    while (is_should_quit == false) {
        start = us();
        // 处理输入
        input_handler();
        // 清屏
        /// @todo 巨大性能开销
        framebuffer.clear();
        // 填充窗口
        fill();
        // 显示 fps
        /// @todo 巨大性能开销
        show_fps(fps);
        // 刷新窗口
        SDL_RenderPresent(sdl_renderer);
        frames++;
        end = us();
        sec += end - start;
        if (sec >= US2S) {
            // std::cout << "fps_window: " << fps << std::endl;
            fps    = frames;
            frames = 0;
            sec    = 0;
        }
    }
    return;
}
