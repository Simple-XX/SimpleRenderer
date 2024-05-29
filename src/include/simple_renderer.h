
/**
 * @file simple_renderer.h
 * @brief simple_renderer 接口头文件
 * @author Zone.N (Zone.Niuzh@hotmail.com)
 * @version 1.0
 * @date 2023-10-23
 * @copyright MIT LICENSE
 * https://github.com/Simple-XX/SimpleRenderer
 * @par change log:
 * <table>
 * <tr><th>Date<th>Author<th>Description
 * <tr><td>2023-10-23<td>Zone.N<td>创建文件
 * </table>
 */

#ifndef SIMPLERENDER_SRC_INCLUDE_SIMPLE_RENDER_H_
#define SIMPLERENDER_SRC_INCLUDE_SIMPLE_RENDER_H_

#include <cstdint>
#include <functional>
#include <span>

#include "light.h"
#include "log_system.h"
#include "model.hpp"
#include "shader.h"

namespace simple_renderer {

class SimpleRenderer {
 public:
  typedef std::function<void(size_t, size_t, uint32_t, std::span<uint32_t> &)>
      DrawPixelFunc;

  /**
   * 构造函数
   * @param width
   * @param height
   * @param buffer 要进行绘制的内存区域，大小为 width*height*sizeof(uint32_t)
   */
  SimpleRenderer(size_t width, size_t height, std::span<uint32_t> &buffer,
                 DrawPixelFunc draw_pixel_func);

  /// @name 默认构造/析构函数
  /// @{
  SimpleRenderer(const SimpleRenderer &_simplerenderer) = default;
  SimpleRenderer(SimpleRenderer &&_simplerenderer) = default;
  auto operator=(const SimpleRenderer &_simplerenderer) -> SimpleRenderer & =
                                                               default;
  auto operator=(SimpleRenderer &&_simplerenderer) -> SimpleRenderer & =
                                                          default;
  virtual ~SimpleRenderer() = default;
  /// @}

  bool render(const Model &model);

 private:
  /// 深度类型
  /// @note 不要进行会影响内存的修改
  using Depth = float;

  const size_t height_;
  const size_t width_;
  std::span<uint32_t> &buffer_;
  std::shared_ptr<Depth[]> depth_buffer_;
  DrawPixelFunc draw_pixel_func_;
  LogSystem log_system_;

  /**
   * 画直线 Bresenham 算法
   * @param _x0 第一个点的 x 坐标
   * @param _y0 第一个点的 y 坐标
   * @param _x1 第二个点的 x 坐标
   * @param _y1 第二个点的 y 坐标
   * @param _color 直线颜色
   * @todo 多线程支持
   */
  void DrawLine(float _x0, float _y0, float _x1, float _y1, const Color &_color);

  /**
   * 填充三角形
   * @param shader 要使用的着色器
   * @param light 光照信息
   * @param normal 面的法向量
   * @param face 面
   * @todo 多线程支持
   */
  void DrawTriangle(const ShaderBase &shader, const Light &light,
                const Model::Normal &normal, const Model::Face &face);

  /**
   * 绘制模型
   * @param shader 要使用的着色器
   * @param light 光照信息
   * @param model 模型
   * @param draw_line 是否绘制线框
   * @param draw_triangle 是否绘制三角形
   */
  void DrawModel(const ShaderBase &shader, const Light &light, const Model &model,
             bool draw_line, bool draw_triangle);

  /**
   * 计算重心坐标
   * @param p0 三角形的第一个点
   * @param p1 三角形的第二个点
   * @param p2 三角形的第三个点
   * @param pa 要判断的点
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
  static auto GetBarycentricCoord(const Vector3f &p0, const Vector3f &p1,
                                  const Vector3f &p2, const Vector3f &pa)
      -> std::pair<bool, Vector3f>;

  /**
   * 深度插值，由重心坐标计算出对应点的深度值
   * @param depth0 点 1
   * @param depth1 点 2
   * @param depth2 点 3
   * @param barycentric_coord 重心坐标
   * @return 深度值
   */
  static auto InterpolateDepth(float depth0, float depth1, float depth2,
                               const Vector3f &barycentric_coord) -> float;
};

}  // namespace simple_renderer

#endif /* SIMPLERENDER_SRC_INCLUDE_SIMPLE_RENDER_H_ */
