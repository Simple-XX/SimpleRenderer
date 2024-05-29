
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

#ifndef SIMPLERENDER_TEST_SYSTEM_TEST_DISPLAY_H_
#define SIMPLERENDER_TEST_SYSTEM_TEST_DISPLAY_H_

#include <SDL.h>

#include <cstdint>
#include <future>
#include <span>

/**
 * 显示抽象
 */
class Display {
 public:
  /**
   * 构造函数
   * @param width 宽度
   * @param height 高度
   */
  explicit Display(size_t width, size_t height);

  /**
   * 析构函数
   */
  ~Display();

  /// @name 默认构造/析构函数
  /// @{
  Display() = delete;
  Display(const Display &display) = default;
  Display(Display &&display) = default;
  auto operator=(const Display &display) -> Display & = default;
  auto operator=(Display &&display) -> Display & = default;
  /// @}

  /**
   * 将 framebuffer 中的数据绘制到屏幕上
   * @param _framebuffer 要绘制的 framebuffer
   */
  void fill(const std::span<uint32_t> &buffer);

  /**
   * 运行
   */
  void run();

  /**
   * 显示循环
   */
  void loop(const std::span<uint32_t> &buffer);

 private:
  /// 窗口标题
  static constexpr const char *kWindowTitle = (char *)"SimpleRenderer";
  /// 默认字体大小
  static constexpr const int32_t kDefaultFontSize = 32;

  /// 窗口宽度
  size_t width_;
  /// 窗口高度
  size_t height_;

  /// sdl 窗口
  SDL_Window *sdl_window_;
  /// sdl 渲染器
  SDL_Renderer *sdl_renderer_;
  /// sdl 纹理
  SDL_Texture *sdl_texture_;
};

#endif /* SIMPLERENDER_TEST_SYSTEM_TEST_DISPLAY_H_ */
