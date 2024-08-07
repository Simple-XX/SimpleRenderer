
/**
 * @file shader_base.h
 * @brief 着色器基类头文件
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

#ifndef SIMPLERENDER_SRC_INCLUDE_SHADER_BASE_H_
#define SIMPLERENDER_SRC_INCLUDE_SHADER_BASE_H_

#include "matrix.hpp"
#include "model.hpp"
#include "vector.hpp"

namespace simple_renderer {

/**
 * 顶点着色器输入数据
 */
class ShaderVertexIn {
 public:
  /// 面信息
  Model::Face face_;

  /**
   * 构造函数
   * @param face 面信息
   */
  explicit ShaderVertexIn(const Model::Face &face);

  /// @name 默认构造/析构函数
  /// @{
  ShaderVertexIn() = default;
  ShaderVertexIn(const ShaderVertexIn &shader_vertex_in) = default;
  ShaderVertexIn(ShaderVertexIn &&shader_vertex_in) = default;
  auto operator=(const ShaderVertexIn &shader_vertex_in) -> ShaderVertexIn & =
                                                                default;
  auto operator=(ShaderVertexIn &&shader_vertex_in) -> ShaderVertexIn & =
                                                           default;
  ~ShaderVertexIn() = default;
  /// @}
};

/**
 * 顶点着色器输出数据
 */
class ShaderVertexOut {
 public:
  /// 面信息
  Model::Face face_;

  /**
   * 构造函数
   * @param face            面信息
   */
  explicit ShaderVertexOut(const Model::Face &face);

  /// @name 默认构造/析构函数
  /// @{
  ShaderVertexOut() = default;
  ShaderVertexOut(const ShaderVertexOut &shader_vertex_out) = default;
  ShaderVertexOut(ShaderVertexOut &&shader_vertex_out) = default;
  auto operator=(const ShaderVertexOut &shader_vertex_out)
      -> ShaderVertexOut & = default;
  auto operator=(ShaderVertexOut &&shader_vertex_out) -> ShaderVertexOut & =
                                                             default;
  ~ShaderVertexOut() = default;
  /// @}
};

/**
 * 片段着色器输入
 */
class ShaderFragmentIn {
 public:
  /// 重心坐标
  glm::vec3 barycentric_coord_;
  /// 法线方向
  glm::vec3 normal_;
  /// 光照方向
  glm::vec3 light_;

  /// @name 三个顶点的颜色
  /// @{
  Color color0_;
  Color color1_;
  Color color2_;
  /// @}

  /**
   * 构造函数
   * @param barycentric_coord 重心坐标
   * @param normal 法线
   * @param light 光照
   * @param color0 顶点 0 颜色
   * @param color1 顶点 1 颜色
   * @param color2 顶点 2 颜色
   */
  explicit ShaderFragmentIn(const glm::vec3 &_barycentric_coord,
                            const glm::vec3 &_normal, const glm::vec3 &_light,
                            const Color &_color0, const Color &_color1,
                            const Color &_color2);

  /// @name 默认构造/析构函数
  /// @{
  ShaderFragmentIn() = default;
  ShaderFragmentIn(const ShaderFragmentIn &shader_fragment_in) = default;
  ShaderFragmentIn(ShaderFragmentIn &&shader_fragment_in) = default;
  auto operator=(const ShaderFragmentIn &shader_fragment_in)
      -> ShaderFragmentIn & = default;
  auto operator=(ShaderFragmentIn &&shader_fragment_in) -> ShaderFragmentIn & =
                                                               default;
  ~ShaderFragmentIn() = default;
  /// @}
};

/**
 * 片段着色器输出
 */
class ShaderFragmentOut {
 public:
  /// 是否需要绘制
  bool is_need_draw_ = false;

  /// 颜色
  Color color_;

  /**
   * 构造函数
   * @param _is_need_draw        是否需要绘制
   * @param _color               颜色
   */
  explicit ShaderFragmentOut(const bool &is_need_draw, const Color &color);

  /// @name 默认构造/析构函数
  /// @{
  ShaderFragmentOut() = default;
  ShaderFragmentOut(const ShaderFragmentOut &shader_fragment_out) = default;
  ShaderFragmentOut(ShaderFragmentOut &&shader_fragment_out) = default;
  auto operator=(const ShaderFragmentOut &shader_fragment_out)
      -> ShaderFragmentOut & = default;
  auto operator=(ShaderFragmentOut &&shader_fragment_out)
      -> ShaderFragmentOut & = default;
  ~ShaderFragmentOut() = default;
  /// @}
};

/**
 * 着色器数据
 */
class ShaderData {
 public:
  /// 模型变换矩阵
  glm::mat4 model_matrix_ = glm::mat4(1.0f);
  /// 视图变换矩阵
  glm::mat4 view_matrix_ = glm::mat4(1.0f);
  /// 正交变换矩阵
  glm::mat4 project_matrix_ = glm::mat4(1.0f);

  /**
   * 构造函数
   * @param model_matrix 模型变换矩阵
   * @param view_matrix 视图变换矩阵
   * @param project_matrix 正交变换矩阵
   */
  explicit ShaderData(const glm::mat4 &model_matrix, const glm::mat4 &view_matrix,
                      const glm::mat4 &project_matrix);

  /// @name 默认构造/析构函数
  /// @{
  ShaderData() = default;
  ShaderData(const ShaderData &shader_data) = default;
  ShaderData(ShaderData &&shader_data) = default;
  auto operator=(const ShaderData &shader_data) -> ShaderData & = default;
  auto operator=(ShaderData &&shader_data) -> ShaderData & = default;
  ~ShaderData() = default;
  /// @}
};

/**
 * 着色器基类
 */
class ShaderBase {
 public:
  /// 着色器数据
  ShaderData shader_data_;

  /// @name 默认构造/析构函数
  /// @{
  ShaderBase() = default;
  ShaderBase(const ShaderBase &shader_base) = default;
  ShaderBase(ShaderBase &&shader_base) = default;
  auto operator=(const ShaderBase &shader_base) -> ShaderBase & = default;
  auto operator=(ShaderBase &&shader_base) -> ShaderBase & = default;
  virtual ~ShaderBase() = default;
  /// @}

  /**
   * 顶点着色器
   * 处理顶点的着色器，所有顶点被载入管线后都会经由顶点着色器处理后载入管线的下一步。
   * 通过顶点着色器可以改变绘制内容的形状，包括形状大小、位置、角度、投影等等。
   * @param shader_vertex_in 顶点着色器输入
   * @return 顶点着色器输出
   */
  [[nodiscard]] virtual auto Vertex(
      const ShaderVertexIn &shader_vertex_in) const -> ShaderVertexOut = 0;

  /**
   * 片段着色器
   * 片段着色器负责处理绘制内容的颜色，所有顶点光栅化后经由片段着色器都会被赋值颜色，并且也会被插值处理。
   * @param shader_fragment_in 片段着色器输入
   * @return 片段着色器输出
   */
  [[nodiscard]] virtual auto Fragment(
      const ShaderFragmentIn &shader_fragment_in) const
      -> ShaderFragmentOut = 0;
};

}  // namespace simple_renderer

#endif /* SIMPLERENDER_SRC_INCLUDE_SHADER_BASE_H_ */
