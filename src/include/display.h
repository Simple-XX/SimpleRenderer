
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
#include "SDL_ttf.h"

#include "framebuffer.h"

/**
 * @brief 对 sdl 的抽象
 * @note 要操作屏幕显示的内容只能通过访问 framebuffer 实现
 */
class display_t {
private:
    /// @brief 窗口标题
    static constexpr const char*   WINDOW_TITLE = (char*)"SimpleRenderer";
    /// @brief 保存要显示的 framebuffer
    std::shared_ptr<framebuffer_t> framebuffer;
    /// @brief sdl 窗口
    SDL_Window*                    sdl_window;
    /// @brief 字体文件
    std::string                    font_file_path = "../../wqy-zenhei.ttc";
    /// @brief 字体指针
    TTF_Font*                      font;
    /// @brief 字体大小
    uint8_t                        fone_size = 32;
    /// @brief 当前 fps
    uint32_t                       fps       = 0;

    /// @brief fps 文字
    SDL_Surface*                   fps_surface;
    /// @brief fps 内容
    std::string                    fps_text  = "FPS: " + std::to_string(fps);
    /// @brief fps 颜色 绿色
    SDL_Color                      fps_color = SDL_Color { 0, 255, 0, 255 };

    /// @brief 标识窗口是否需要退出
    bool                           is_should_quit = false;
    /// @brief sdl 事件
    SDL_Event                      sdl_event;
    /// 窗口宽度
    uint32_t                       width;
    /// 窗口高度
    uint32_t                       height;

    /**
     * @brief 从 argb32 中获取 alpha 通道
     * @param  _color           argb32
     * @return uint8_t          alpha
     */
    static uint8_t                 ARGB2A(const uint32_t _color);

    /**
     * @brief 从 argb32 中获取 red 通道
     * @param  _color           argb32
     * @return uint8_t          red
     */
    static uint8_t                 ARGB2R(const uint32_t _color);

    /**
     * @brief 从 argb32 中获取 green 通道
     * @param  _color           argb32
     * @return uint8_t          green
     */
    static uint8_t                 ARGB2G(const uint32_t _color);

    /**
     * @brief 从 argb32 中获取 blue 通道
     * @param  _color           argb32
     * @return uint8_t          blue
     */
    static uint8_t                 ARGB2B(const uint32_t _color);

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
    void pixel(SDL_Surface* _surface, const uint32_t _x, const uint32_t _y,
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

    /**
     * @brief 将 _src 绘制到当前窗口 _x, _y 处
     * @param  _x               横坐标
     * @param  _y               纵坐标
     * @param  _src             要绘制的内容
     */
    void apply_surface(uint32_t _x, uint32_t _y, SDL_Surface* _src) const;

    /**
     * @brief 在屏幕左上角显示 fps
     * @todo fps 计算方式可能有误
     */
    void show_fps(void);

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
