#ifndef SIMPLERENDER_SRC_INCLUDE_RASTERIZER_HPP_
#define SIMPLERENDER_SRC_INCLUDE_RASTERIZER_HPP_

#include "config.h"
#include "shader.hpp"
#include "vertex.hpp"

namespace simple_renderer {

class Rasterizer {
 public:
  Rasterizer() = default;
  Rasterizer(const Rasterizer& rasterizer) = default;
  Rasterizer(Rasterizer&& rasterizer) = default;
  auto operator=(const Rasterizer& rasterizer) -> Rasterizer& = default;
  auto operator=(Rasterizer&& rasterizer) -> Rasterizer& = default;
  ~Rasterizer() = default;

  /**
   * @brief 构造具有指定尺寸的光栅化器
   * @param width 光栅化器宽度
   * @param height 光栅化器高度
   */
  Rasterizer(size_t width, size_t height);

  /**
   * @brief 光栅化三角形，生成片段列表
   * @param v0 三角形第一个顶点
   * @param v1 三角形第二个顶点
   * @param v2 三角形第三个顶点
   * @return 生成的片段向量
   */
  std::vector<Fragment> Rasterize(const Vertex& v0, const Vertex& v1,
                                  const Vertex& v2);

 private:
  size_t width_, height_;

  // 透视矫正结果
  struct PerspectiveCorrectionResult {
    Vector3f corrected_barycentric;
    float interpolated_z;
  };

  // 透视矫正helper函数
  PerspectiveCorrectionResult PerformPerspectiveCorrection(
      float w0, float w1, float w2,
      float z0, float z1, float z2,
      const Vector3f& original_barycentric) const;

  template <typename T>
  T Interpolate(const T& v0, const T& v1, const T& v2,
                const Vector3f& barycentric_coord) const;

  Color InterpolateColor(const Color& color0, const Color& color1,
                         const Color& color2,
                         const Vector3f& barycentric_coord) const;

  std::pair<bool, Vector3f> GetBarycentricCoord(const Vector3f& p0,
                                                const Vector3f& p1,
                                                const Vector3f& p2,
                                                const Vector3f& pa);

  // Calculate the normal vector based on the vertices
  // 根据顶点计算法向量
  Vector3f CalculateNormal(const Vector3f& v0, const Vector3f& v1,
                           const Vector3f& v2);
};

}  // namespace simple_renderer

#endif  // SIMPLERENDER_SRC_INCLUDE_RASTERIZER_HPP_
