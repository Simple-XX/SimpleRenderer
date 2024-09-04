
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

#include "math.hpp"
#include "model.hpp"

namespace simple_renderer {

/**
 * 顶点着色器输入数据
 */
class ShaderVertexIn {
 public:
  /// 面信息
  Face face_;

  /**
   * 构造函数
   * @param face 面信息
   */
  explicit ShaderVertexIn(const Face &face);

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
  Face face_;

  /**
   * 构造函数
   * @param face            面信息
   */
  explicit ShaderVertexOut(const Face &face);

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
  Vector3f barycentric_coord_;

  // face
  // 面信息
  Face face_;

  /// 光照方向
  Vector3f light_;

  /**
   * 构造函数
   * @param barycentric_coord 重心坐标
   * @param face 面信息
   * @param light 光照
   */
  explicit ShaderFragmentIn(const Vector3f &_barycentric_coord,
                            const Face &face, const Vector3f &light);

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
  /// 颜色
  Color color_;

  /**
   * 构造函数
   * @param _color               颜色
   */
  explicit ShaderFragmentOut(const Color &color);

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
  Matrix4f model_matrix_ = Matrix4f(1.0f);
  /// 视图变换矩阵
  Matrix4f view_matrix_ = Matrix4f(1.0f);
  /// 正交变换矩阵
  Matrix4f project_matrix_ = Matrix4f(1.0f);

  /**
   * 构造函数
   * @param model_matrix 模型变换矩阵
   * @param view_matrix 视图变换矩阵
   * @param project_matrix 正交变换矩阵
   */
  explicit ShaderData(const Matrix4f &model_matrix, const Matrix4f &view_matrix,
                      const Matrix4f &project_matrix);

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
