
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

#include <SDL.h>
#include <SDL_ttf.h>

#include "config.h"
#include "framebuffer.h"
#include "input.h"
#include "status.h"

/**
 * 显示抽象
 */
class display_t {
public:
  /**
   * 构造函数
   * @param _state 运行状态
   * @param _framebuffers 缓冲区
   */
  explicit display_t(
      const std::shared_ptr<state_t> &_state,
      const std::vector<std::shared_ptr<framebuffer_t>> &_framebuffers);

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
  void fill(const std::shared_ptr<framebuffer_t> &_framebuffer);

  /**
   * 运行
   */
  void run();

private:
  /// 窗口标题
  static constexpr const char *WINDOW_TITLE = (char *)"SimpleRenderer";
  /// 默认字体大小
  static constexpr const int32_t DEFAULT_FONT_SIZE = 32;

  /// 输入
  std::shared_ptr<state_t> state;
  /// 缓冲
  std::vector<std::shared_ptr<framebuffer_t>> framebuffers;
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
  /// 字体指针
  TTF_Font *font;
  /// 字体大小
  uint8_t font_size = DEFAULT_FONT_SIZE;

  /**
   * 显示循环
   */
  enum state_t::status_t loop();
};

#endif /* SIMPLERENDER_DISPLAY_H */
