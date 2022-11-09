
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

#include "event_callback.h"
#include "framebuffer.h"

/**
 * @brief 对 sdl 的抽象
 * @note 要操作屏幕显示的内容只能通过访问 framebuffer 实现
 */
class display_t {
private:
    /// @brief 字体文件
    static constexpr const char*    font_file_path = "../../wqy-zenhei.ttc";
    /// @brief fps 内容
    static constexpr const char*    FPS            = "FPS: ";
    /// @brief fps x 位置
    static constexpr const uint32_t FPS_POS_X      = 0;
    /// @brief fps y 位置
    static constexpr const uint32_t FPS_POS_Y      = 0;
    /// @brief fps x 长度
    static constexpr const uint32_t FPS_SIZE_X     = 160;
    /// @brief fps y 高度
    static constexpr const uint32_t FPS_SIZE_Y     = 90;
    /// @brief fps 位置
    static constexpr const SDL_Rect FPS_RECT       = {
        FPS_POS_X,
        FPS_POS_Y,
        FPS_SIZE_X,
        FPS_SIZE_Y,
    };

    /// @brief 窗口标题
    static constexpr const char* WINDOW_TITLE = (char*)"SimpleRenderer";
    /// @brief sdl 窗口
    SDL_Window*                  sdl_window;
    /// @brief sdl 渲染器
    SDL_Renderer*                sdl_renderer;
    /// @brief sdl 纹理
    SDL_Texture*                 sdl_texture;
    /// @brief 字体指针
    TTF_Font*                    font;
    /// @brief 字体大小
    uint8_t                      font_size      = 32;

    /// @brief fps 颜色 绿色
    SDL_Color                    fps_color      = SDL_Color { 0, 255, 0, 255 };

    /// @brief 标识窗口是否需要退出
    bool                         is_should_quit = false;
    /// @brief sdl 事件
    SDL_Event                    sdl_event;
    /// @brief 保存要显示的 framebuffer
    framebuffer_t&               framebuffer;
    /// @brief 相机
    surround_camera_t&                    camera;
    /// @brief 事件处理
    event_callback_t&            event_callback;
    /// @brief 窗口宽度
    uint32_t                     width;
    /// @brief 窗口高度
    uint32_t                     height;

    /**
     * @brief 处理输入
     */
    void                         input_handler(void);

    /**
     * @brief 将 framebuffer 中的数据绘制到屏幕上
     */
    void                         fill(void);

    /**
     * @brief 在屏幕左上角显示 fps
     * @param  _fps             fps 值
     * @todo fps 计算方式可能有误
     */
    void                         show_fps(const uint32_t _fps);

public:
    /**
     * @brief 不使用空构造函数
     */
    display_t(void) = delete;

    /**
     * @brief 构造函数
     * @param  _framebuffer     要绑定的缓冲区
     * @param  _camera          要绑定的相机
     * @param  _event_callback  事件处理
     */
    display_t(framebuffer_t& _framebuffer, surround_camera_t& _camera,
              event_callback_t& _event_callback);

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
