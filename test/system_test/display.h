
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

#include <cstdint>
#include <future>

#include <SDL.h>


#include "framebuffer.h"

/**
 * 显示抽象
 */
class display_t {
public:
  /**
   * 构造函数
   * @param _state 运行状态
   * @param _width 宽度
   * @param _height 高度
   */
  explicit display_t(size_t _width, size_t _height);

  /**
   * 析构函数
   */
  ~display_t();

  /// @name 默认构造/析构函数
  /// @{
  display_t() = delete;
  display_t(const display_t &_display) = default;
  display_t(display_t &&_display) = default;
  auto operator=(const display_t &_display) -> display_t & = default;
  auto operator=(display_t &&_display) -> display_t & = default;
  /// @}

  /**
   * 将 framebuffer 中的数据绘制到屏幕上
   * @param _framebuffer 要绘制的 framebuffer
   */
  void fill(const std::shared_ptr<SimpleRenderer::framebuffer_t> &_framebuffer);

  /**
   * 运行
   */
  void run();

private:
  /// 窗口标题
  static constexpr const char *WINDOW_TITLE = (char *)"SimpleRenderer";
  /// 默认字体大小
  static constexpr const int32_t DEFAULT_FONT_SIZE = 32;

  /// 窗口宽度
  size_t width;
  /// 窗口高度
  size_t height;

  /// sdl 窗口
  SDL_Window *sdl_window;
  /// sdl 渲染器
  SDL_Renderer *sdl_renderer;
  /// sdl 纹理
  SDL_Texture *sdl_texture;

  /**
   * 显示循环
   */
  void loop();
};

#endif /* SIMPLERENDER_DISPLAY_H */
