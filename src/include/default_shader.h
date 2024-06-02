
/**
 * @file default_shader.h
 * @brief 默认着色器
 * @author Zone.N (Zone.Niuzh@hotmail.com)
 * @version 1.0
 * @date 2022-11-01
 * @copyright MIT LICENSE
 * https://github.com/Simple-XX/SimpleRenderer
 * @par change log:
 * <table>
 * <tr><th>Date<th>Author<th>Description
 * <tr><td>2022-11-01<td>Zone.N<td>创建文件
 * </table>
 */

#ifndef SIMPLERENDER_SRC_INCLUDE_DEFAULT_SHADER_H_
#define SIMPLERENDER_SRC_INCLUDE_DEFAULT_SHADER_H_

#include "shader_base.h"

namespace simple_renderer {

/**
 * 默认着色器
 */
class DefaultShader : public ShaderBase {
 public:
  /// @name 默认构造/析构函数
  /// @{
  DefaultShader() = default;
  DefaultShader(const DefaultShader &default_shader) = default;
  DefaultShader(DefaultShader &&default_shader) = default;
  auto operator=(const DefaultShader &default_shader) -> DefaultShader & =
                                                             default;
  auto operator=(DefaultShader &&default_shader) -> DefaultShader & = default;
  ~DefaultShader() override = default;
  /// @}

  /**
   * 顶点着色器
   * @param shader_vertex_in 输入
   * @return 输出
   */
  [[nodiscard]] auto Vertex(const ShaderVertexIn &shader_vertex_in) const
      -> ShaderVertexOut final;

  /**
   * 片段着色器
   * @param _shader_fragment_in 输入
   * @return 输出
   */
  [[nodiscard]] auto Fragment(const ShaderFragmentIn &shader_fragment_in) const
      -> ShaderFragmentOut final;

 private:
  /**
   * 颜色插值，由重心坐标计算出对应点的颜色
   * @param color0 第一个点的颜色
   * @param color1 第二个点的颜色
   * @param color2 第三个点的颜色
   * @param barycentric_coord 重心坐标
   * @return 颜色值
   */
  static auto InterpolateColor(const Color &color0, const Color &color1,
                               const Color &color2,
                               const Vector3f &barycentric_coord) -> Color;
};

}  // namespace simple_renderer

#endif /* SIMPLERENDER_SRC_INCLUDE_DEFAULT_SHADER_H_ */
