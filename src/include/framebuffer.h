
/**
 * @file framebuffer.h
 * @brief 缓冲区抽象
 * @author Zone.N (Zone.Niuzh@hotmail.com)
 * @version 1.0
 * @date 2022-09-03
 * @copyright MIT LICENSE
 * https://github.com/Simple-XX/SimpleRenderer
 * @par change log:
 * <table>
 * <tr><th>Date<th>Author<th>Description
 * <tr><td>2022-09-03<td>Zone.N<td>创建文件
 * </table>
 */

#ifndef SIMPLERENDER_FRAMEBUFFER_H
#define SIMPLERENDER_FRAMEBUFFER_H

#include <array>
#include <atomic>
#include <cstdint>
#include <mutex>

#include <omp.h>

#include "buffer_base.hpp"
#include "color.h"
#include "scene.h"
#include "shader.h"

/**
 * 帧缓冲
 * @todo 大小不一样的赋值处理
 */
class framebuffer_t {
public:
  /// 深度类型
  /// @note 不要进行会影响内存的修改
  using depth_t = float;

  using color_buffer_t = buffer_base_t<color_t>;
  using depth_buffer_t = buffer_base_t<depth_t>;

  /// 缓冲区 id
  uint32_t id;

  /// 是否可显示，为 false 时表示不可显示，需要先绘制
  std::atomic_bool displayable = false;

  /**
   * 空构造
   */
  framebuffer_t();

  /**
   * 拷贝构造
   * @param _framebuffer 另一个 framebuffer_t
   */
  framebuffer_t(const framebuffer_t &_framebuffer);

  framebuffer_t(framebuffer_t &&_framebuffer);

  /**
   * 构造函数
   * @param _width 宽度
   * @param _height 高度
   */
  framebuffer_t(uint32_t _width, uint32_t _height);

  /**
   * 赋值
   * @param _framebuffer 另一个 framebuffer_t
   * @return 结果
   */
  auto operator=(const framebuffer_t &_framebuffer) -> framebuffer_t &;

  auto operator=(framebuffer_t &&_framebuffer) -> framebuffer_t &;

  /// @name 默认构造/析构函数
  /// @{
  ~framebuffer_t() = default;
  /// @}

  /**
   * 获取缓冲区宽度
   * @return 缓冲区宽度
   */
  [[nodiscard]] auto get_width() const -> uint32_t;

  /**
   * 获取缓冲区高度
   * @return 缓冲区高度
   */
  [[nodiscard]] auto get_height() const -> uint32_t;

  /**
   * 清空成指定颜色
   * @param _color 指定颜色
   * @param _depth 指定深度
   */
  void clear(const color_t &_color = color_t(),
             const depth_t &_depth = std::numeric_limits<depth_t>::lowest());

  /**
   * 设置像素
   * @param _row 行
   * @param _col 列
   * @param _color 颜色
   * @param _depth 深度
   * @note (0, 0) 在屏幕左上角
   */
  void pixel(uint32_t _row, uint32_t _col, const color_t &_color,
             const depth_t &_depth = std::numeric_limits<depth_t>::lowest());

  /**
   * 获取颜色缓存
   * @return 颜色缓存
   */
  auto get_color_buffer() -> color_buffer_t &;

  /**
   * 获取颜色缓存
   * @return 只读的颜色缓存
   */
  [[nodiscard]] auto get_color_buffer() const -> const color_buffer_t &;

  /**
   * 获取深度缓存
   * @return 深度缓存
   */
  auto get_depth_buffer() -> depth_buffer_t &;

  /**
   * 获取深度缓存
   * @return 只读的深度缓存
   */
  [[nodiscard]] auto get_depth_buffer() const -> const depth_buffer_t &;

  /**
   * 获取深度值
   * @param _row 行
   * @param _col 列
   * @return 对应位置的深度值
   */
  auto get_depth_buffer(uint32_t _row, uint32_t _col) -> depth_t &;

  /**
   * 获取深度值
   * @param _row 行
   * @param _col 列
   * @return 对应位置的深度值
   */
  [[nodiscard]] auto get_depth_buffer(uint32_t _row, uint32_t _col) const
      -> depth_t;

  /**
   * 画直线 Bresenham 算法
   * @param _x0 第一个点的 x 坐标
   * @param _y0 第一个点的 y 坐标
   * @param _x1 第二个点的 x 坐标
   * @param _y1 第二个点的 y 坐标
   * @param _color 直线颜色
   * @todo 多线程支持
   */
  void line(float _x0, float _y0, float _x1, float _y1, const color_t &_color);

  /**
   * 填充三角形
   * @param _config 配置信息
   * @param _shader 要使用的着色器
   * @param _light 光照信息
   * @param _normal 面的法向量
   * @param _face 面
   * @todo 多线程支持
   */
  void triangle(const shader_base_t &_shader, const light_t &_light,
                const model_t::normal_t &_normal, const model_t::face_t &_face);

  /**
   * 绘制模型
   * @param _config 配置信息
   * @param _shader 要使用的着色器
   * @param _light 光照信息
   * @param _model 模型
   */
  void model(const shader_base_t &_shader, const light_t &_light,
             const model_t &_model);

  /**
   * 绘制场景
   * @param _shader 要使用的着色器
   * @param _scene 场景
   */
  void scene(const shader_base_t &_shader, const scene_t &_scene);

private:
  /// 缓冲区计数，用于设置 id
  static uint32_t count;

  /// 窗口宽度
  size_t width = 0;
  /// 窗口高度
  size_t height = 0;

  /// 颜色缓冲区
  color_buffer_t color_buffer = color_buffer_t();

  /// 深度缓冲区
  depth_buffer_t depth_buffer = depth_buffer_t();

  /**
   * 计算重心坐标
   * @param _p0 三角形的第一个点
   * @param _p1 三角形的第二个点
   * @param _p2 三角形的第三个点
   * @param _p 要判断的点
   * @return std::pair<bool, vector4f_t>
   *  第一个返回为 _p 是否在三角形内，第二个为重心坐标
   * @see http://blackpawn.com/texts/pointinpoly/
   * solve
   *     P = A + s * AB + t * AC  -->  AP = s * AB + t * AC
   * then
   *     s = (AC.y * AP.x - AC.x * AP.y) / (AB.x * AC.y - AB.y * AC.x)
   *     t = (AB.x * AP.y - AB.y * AP.x) / (AB.x * AC.y - AB.y * AC.x)
   *
   * notice
   *     P = A + s * AB + t * AC
   *       = A + s * (B - A) + t * (C - A)
   *       = (1 - s - t) * A + s * B + t * C
   * then
   *     weight_A = 1 - s - t
   *     weight_B = s
   *     weight_C = t
   */
  static auto get_barycentric_coord(const vector3f_t &_p0,
                                    const vector3f_t &_p1,
                                    const vector3f_t &_p2, const vector3f_t &_p)
      -> std::pair<bool, vector3f_t>;

  /**
   * 深度插值，由重心坐标计算出对应点的深度值
   * @param _v0 点 1
   * @param _v1 点 2
   * @param _v2 点 3
   * @param _barycentric_coord 重心坐标
   * @return 深度值
   */
  static auto interpolate_depth(float _depth0, float _depth1, float _depth2,
                                const vector3f_t &_barycentric_coord) -> float;
};

#endif /* SIMPLERENDER_FRAMEBUFFER_H */
