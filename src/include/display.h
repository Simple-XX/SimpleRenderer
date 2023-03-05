
/**
 * @file display.h
 * @brief 显示
 * @author Zone.N (Zone.Niuzh@hotmail.com)
 * @version 1.0
 * @date 2022-09-04
 * @copyright MIT LICENSE
 * https://github.com/Simple-XX/SimpleRenderer
 * @par change log:
 * <table>
 * <tr><th>Date<th>Author<th>Description
 * <tr><td>2022-09-04<td>Zone.N<td>创建文件
 * </table>
 */

#ifndef SIMPLERENDER_DISPLAY_H
#define SIMPLERENDER_DISPLAY_H

#include "cstdint"

#include "SDL.h"
#include "SDL_ttf.h"

#include "config.h"
#include "framebuffer.h"

/**
 * @brief 显示抽象
 */
class display_t {
private:
    /// @brief 窗口标题
    static constexpr const char* WINDOW_TITLE = (char*)"SimpleRenderer";

    /// @brief 窗口宽度
    uint32_t                     width;
    /// @brief 窗口高度
    uint32_t                     height;

    /// @brief sdl 窗口
    SDL_Window*                  sdl_window;
    /// @brief sdl 渲染器
    SDL_Renderer*                sdl_renderer;
    /// @brief sdl 纹理
    SDL_Texture*                 sdl_texture;
    /// @brief 字体指针
    TTF_Font*                    font;
    /// @brief 字体大小
    uint8_t                      font_size = 32;

public:
    /**
     * @brief 空构造函数
     */
    display_t(void) = delete;

    display_t(const display_t& _display);

    /**
     * @brief 构造函数
     * @param  _config          配置信息
     */
    display_t(const config_t& _config);

    /**
     * @brief 构造函数
     * @param  _width           宽度
     * @param  _height          高度
     */
    display_t(uint32_t _width, uint32_t _height);

    /**
     * @brief 析构函数
     */
    ~display_t(void);

    /**
     * @brief 将 framebuffer 中的数据绘制到屏幕上
     * @param  _framebuffer     要绘制的 framebuffer
     */
    void fill(const std::shared_ptr<framebuffer_t>& _framebuffer);
};

#endif /* SIMPLERENDER_DISPLAY_H */
