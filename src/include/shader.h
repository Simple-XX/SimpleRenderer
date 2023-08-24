
/**
 * @file shader.h
 * @brief 着色器头文件
 * @author Zone.N (Zone.Niuzh@hotmail.com)
 * @version 1.0
 * @date 2022-10-15
 * @copyright MIT LICENSE
 * https://github.com/Simple-XX/SimpleRenderer
 * @par change log:
 * <table>
 * <tr><th>Date<th>Author<th>Description
 * <tr><td>2022-10-15<td>Zone.N<td>创建文件
 * </table>
 */

#ifndef SIMPLERENDER_SHADER_H
#define SIMPLERENDER_SHADER_H

#include "matrix.h"
#include "model.h"
#include "vector.h"

/**
 * 顶点着色器输入数据
 */
class shader_vertex_in_t {
public:
  /// 面信息
  model_t::face_t face;

  /**
   * 构造函数
   * @param _face 面信息
   */
  explicit shader_vertex_in_t(const model_t::face_t &_face);

  /// @name 默认构造/析构函数
  /// @{
  shader_vertex_in_t() = default;
  shader_vertex_in_t(const shader_vertex_in_t &_shader_vertex_in) = default;
  shader_vertex_in_t(shader_vertex_in_t &&_shader_vertex_in) = default;
  auto operator=(const shader_vertex_in_t &_shader_vertex_in)
      -> shader_vertex_in_t & = default;
  auto operator=(shader_vertex_in_t &&_shader_vertex_in)
      -> shader_vertex_in_t & = default;
  ~shader_vertex_in_t() = default;
  /// @}
};

/**
 * 顶点着色器输出数据
 */
class shader_vertex_out_t {
public:
  /// 面信息
  model_t::face_t face;

  /**
   * 构造函数
   * @param _face            面信息
   */
  explicit shader_vertex_out_t(const model_t::face_t &_face);

  /// @name 默认构造/析构函数
  /// @{
  shader_vertex_out_t() = default;
  shader_vertex_out_t(const shader_vertex_out_t &_shader_vertex_out) = default;
  shader_vertex_out_t(shader_vertex_out_t &&_shader_vertex_out) = default;
  auto operator=(const shader_vertex_out_t &_shader_vertex_out)
      -> shader_vertex_out_t & = default;
  auto operator=(shader_vertex_out_t &&_shader_vertex_out)
      -> shader_vertex_out_t & = default;
  ~shader_vertex_out_t() = default;
  /// @}
};

/**
 * 片段着色器输入
 */
class shader_fragment_in_t {
public:
  /// 重心坐标
  vector4f_t barycentric_coord;
  /// 法线方向
  vector4f_t normal;
  /// 光照方向
  vector4f_t light;

  /// @name 三个顶点的颜色
  /// @{
  color_t color0;
  color_t color1;
  color_t color2;
  /// @}

  /**
   * 构造函数
   * @param _barycentric_coord 重心坐标
   * @param _normal 法线
   * @param _light 光照
   * @param _color0 顶点 0 颜色
   * @param _color1 顶点 1 颜色
   * @param _color2 顶点 2 颜色
   */
  explicit shader_fragment_in_t(const vector4f_t &_barycentric_coord,
                                const vector4f_t &_normal,
                                const vector4f_t &_light,
                                const color_t &_color0, const color_t &_color1,
                                const color_t &_color2);

  /// @name 默认构造/析构函数
  /// @{
  shader_fragment_in_t() = default;
  shader_fragment_in_t(const shader_fragment_in_t &_shader_fragment_in) =
      default;
  shader_fragment_in_t(shader_fragment_in_t &&_shader_fragment_in) = default;
  auto operator=(const shader_fragment_in_t &_shader_fragment_in)
      -> shader_fragment_in_t & = default;
  auto operator=(shader_fragment_in_t &&_shader_fragment_in)
      -> shader_fragment_in_t & = default;
  ~shader_fragment_in_t() = default;
  /// @}
};

/**
 * 片段着色器输出
 */
class shader_fragment_out_t {
public:
  /// 是否需要绘制
  bool is_need_draw = false;

  /// 颜色
  color_t color;

  /**
   * 构造函数
   * @param _is_need_draw        是否需要绘制
   * @param _color               颜色
   */
  explicit shader_fragment_out_t(const bool &_is_need_draw,
                                 const color_t &_color);

  /// @name 默认构造/析构函数
  /// @{
  shader_fragment_out_t() = default;
  shader_fragment_out_t(const shader_fragment_out_t &_shader_fragment_out) =
      default;
  shader_fragment_out_t(shader_fragment_out_t &&_shader_fragment_out) = default;
  auto operator=(const shader_fragment_out_t &_shader_fragment_out)
      -> shader_fragment_out_t & = default;
  auto operator=(shader_fragment_out_t &&_shader_fragment_out)
      -> shader_fragment_out_t & = default;
  ~shader_fragment_out_t() = default;
  /// @}
};

/**
 * 着色器数据
 */
class shader_data_t {
public:
  /// 模型变换矩阵
  matrix4f_t model_matrix;
  /// 视图变换矩阵
  matrix4f_t view_matrix;
  /// 正交变换矩阵
  matrix4f_t project_matrix;

  /**
   * 构造函数
   * @param _model_matrix 模型变换矩阵
   * @param _view_matrix 视图变换矩阵
   * @param _project_matrix 正交变换矩阵
   */
  explicit shader_data_t(const matrix4f_t &_model_matrix,
                         const matrix4f_t &_view_matrix,
                         const matrix4f_t &_project_matrix);

  /// @name 默认构造/析构函数
  /// @{
  shader_data_t() = default;
  shader_data_t(const shader_data_t &_shader_data) = default;
  shader_data_t(shader_data_t &&_shader_data) = default;
  auto operator=(const shader_data_t &_shader_data)
      -> shader_data_t & = default;
  auto operator=(shader_data_t &&_shader_data) -> shader_data_t & = default;
  ~shader_data_t() = default;
  /// @}
};

/**
 * 着色器基类
 */
class shader_base_t {
public:
  /// 着色器数据
  shader_data_t shader_data;

  /// @name 默认构造/析构函数
  /// @{
  shader_base_t() = default;
  shader_base_t(const shader_base_t &_shader_base) = default;
  shader_base_t(shader_base_t &&_shader_base) = default;
  auto operator=(const shader_base_t &_shader_base)
      -> shader_base_t & = default;
  auto operator=(shader_base_t &&_shader_base) -> shader_base_t & = default;
  virtual ~shader_base_t() = default;
  /// @}

  /**
   * 顶点着色器
   * 处理顶点的着色器，所有顶点被载入管线后都会经由顶点着色器处理后载入管线的下一步。
   * 通过顶点着色器可以改变绘制内容的形状，包括形状大小、位置、角度、投影等等。
   * @param _shader_vertex_in 顶点着色器输入
   * @return 顶点着色器输出
   */
  [[nodiscard]] virtual auto
  vertex(const shader_vertex_in_t &_shader_vertex_in) const
      -> shader_vertex_out_t = 0;

  /**
   * 片段着色器
   * 片段着色器负责处理绘制内容的颜色，所有顶点光栅化后经由片段着色器都会被赋值颜色，并且也会被插值处理。
   * @param _shader_fragment_in 片段着色器输入
   * @return 片段着色器输出
   */
  [[nodiscard]] virtual auto
  fragment(const shader_fragment_in_t &_shader_fragment_in) const
      -> shader_fragment_out_t = 0;
};

#endif /* SIMPLERENDER_SHADER_H */
