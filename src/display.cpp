
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
#include "display.h"

uint8_t display_t::ARGB2A(uint32_t _color) {
    return (_color >> 24) & 0xFF;
}

uint8_t display_t::ARGB2R(uint32_t _color) {
    return (_color >> 16) & 0xFF;
}

uint8_t display_t::ARGB2G(uint32_t _color) {
    return (_color >> 8) & 0xFF;
}

uint8_t display_t::ARGB2B(uint32_t _color) {
    return (_color >> 0) & 0xFF;
}

void display_t::pixel(SDL_Surface *_surface, const uint32_t _x,
                      const uint32_t _y, const uint8_t _a, const uint8_t _r,
                      const uint8_t _g, const uint8_t _b) {
    // 加锁
    SDL_LockSurface(_surface);
    // 判断颜色深度
    int bpp = _surface->format->BytesPerPixel;
    if (bpp != 4) {
        std::cerr << "Only support "
                  << SDL_GetPixelFormatName(
                         SDL_GetWindowPixelFormat(sdl_window))
                  << std::endl;
        return;
    }
    // 计算像素位置
    uint8_t *p = (uint8_t *)_surface->pixels + _y * _surface->pitch + _x * bpp;
    *(uint32_t *)p = SDL_MapRGBA(_surface->format, _r, _g, _b, _a);
    SDL_UnlockSurface(_surface);
    return;
}

display_t::display_t(framebuffer_t &_framebuffer) : framebuffer(_framebuffer) {
    width  = framebuffer.width;
    height = framebuffer.height;
    // 初始化 sdl
    SDL_Init(SDL_INIT_VIDEO);
    // 创建窗口，居中，可见
    sdl_window = SDL_CreateWindow("SimpleRenderer", SDL_WINDOWPOS_CENTERED,
                                  SDL_WINDOWPOS_CENTERED, width, height,
                                  SDL_WINDOW_SHOWN);
    return;
}

display_t::~display_t(void) {
    // 回收资源
    SDL_DestroyWindow(sdl_window);
    SDL_Quit();
}

void display_t::input_handler(void) {
    // 捕获事件
    while (SDL_PollEvent(&sdl_event)) {
        switch (sdl_event.type) {
                // 键盘事件
            case SDL_KEYDOWN: {
                // 输出按键名
                printf("key %s down！\n",
                       SDL_GetKeyName(sdl_event.key.keysym.sym));
                // 如果是 esc 键则将 is_shoule_quit 置位
                if (sdl_event.key.keysym.sym == SDLK_ESCAPE) {
                    is_shoule_quit = true;
                }
                break;
            }
            case SDL_QUIT: {
                is_shoule_quit = true;
                break;
            }
        }
    }
    return;
}

void display_t::fill(void) {
    // 获取 framebuffer 颜色缓存
    auto     color_buffer = framebuffer.get_color_buffer();
    uint32_t color        = 0x00000000;
    auto     surface      = SDL_GetWindowSurface(sdl_window);
    // 填充整个屏幕
    for (uint32_t i = 0; i < width; i++) {
        for (uint32_t j = 0; j < height; j++) {
            color = color_buffer[j * width + i];
            pixel(surface, i, j, ARGB2A(color), ARGB2R(color), ARGB2G(color),
                  ARGB2B(color));
        }
    }
    return;
}

void display_t::loop(void) {
    // 主循环
    while (is_shoule_quit == false) {
        // 处理输入
        input_handler();
        // 填充窗口
        fill();
        // 刷新窗口
        SDL_UpdateWindowSurface(sdl_window);
    }
    return;
}
