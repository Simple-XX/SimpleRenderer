
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

#ifndef _DISPLAY_H_
#define _DISPLAY_H_

#include "cstdint"
#include "SDL.h"
#include "framebuffer.h"

/**
 * @brief 对 sdl 的抽象
 * @note 要操作屏幕显示的内容只能通过访问 framebuffer 实现
 */
class display_t {
private:
    /// @brief 保存要显示的 framebuffer
    std::shared_ptr<framebuffer_t> framebuffer;
    /// @brief sdl 窗口
    SDL_Window *sdl_window;
    /// @brief 标识窗口是否需要退出
    bool is_shoule_quit = false;
    /// @brief sdl 事件
    SDL_Event sdl_event;
    /// 窗口宽度
    uint32_t width;
    /// 窗口高度
    uint32_t height;

    /**
     * @brief 从 argb32 中获取 alpha 通道
     * @param  _color           argb32
     * @return uint8_t          alpha
     */
    static uint8_t ARGB2A(uint32_t _color);

    /**
     * @brief 从 argb32 中获取 red 通道
     * @param  _color           argb32
     * @return uint8_t          red
     */
    static uint8_t ARGB2R(uint32_t _color);

    /**
     * @brief 从 argb32 中获取 green 通道
     * @param  _color           argb32
     * @return uint8_t          green
     */
    static uint8_t ARGB2G(uint32_t _color);

    /**
     * @brief 从 argb32 中获取 blue 通道
     * @param  _color           argb32
     * @return uint8_t          blue
     */
    static uint8_t ARGB2B(uint32_t _color);

    /**
     * @brief 设置像素
     * @param  _surface         绘制面
     * @param  _x               横坐标
     * @param  _y               纵坐标
     * @param  _a               alpha
     * @param  _r               红色
     * @param  _g               绿色
     * @param  _b               蓝色
     * @note (0, 0) 为屏幕左上角
     * @note alpha 通道无效
     */
    void pixel(SDL_Surface *_surface, const uint32_t _x, const uint32_t _y,
               const uint8_t _a, const uint8_t _r, const uint8_t _g,
               const uint8_t _b);

    /**
     * @brief 处理输入
     */
    void input_handler(void);

    /**
     * @brief 将 framebuffer 中的数据绘制到屏幕上
     */
    void fill(void);

public:
    /**
     * @brief 不使用空构造函数
     */
    display_t(void) = delete;

    /**
     * @brief 构造函数
     * @param  _framebuffer     要绑定的缓冲
     */
    display_t(std::shared_ptr<framebuffer_t> _framebuffer);

    /**
     * @brief 析构函数
     */
    ~display_t(void);

    /**
     * @brief 主循环
     */
    void loop(void);
};

#endif /* _DISPLAY_H_ */
