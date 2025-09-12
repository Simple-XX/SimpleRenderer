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

  Rasterizer(size_t width, size_t height);

  std::vector<Fragment> Rasterize(const Vertex& v0, const Vertex& v1,
                                  const Vertex& v2);

  // 非分配版本：将片段直接写入调用方提供的容器
  // 可选的裁剪区域为半开区间 [x0, x1) × [y0, y1)
  // 用于 TBR：将光栅化限制在 tile 边界内，便于复用外部 scratch 容器
  void RasterizeTo(const Vertex& v0, const Vertex& v1, const Vertex& v2,
                   int x0, int y0, int x1, int y1,
                   std::vector<Fragment>& out);

  // SoA 版本：按顶点索引从 SoA 读取三角形三顶点
  void RasterizeTo(const VertexSoA& soa, size_t i0, size_t i1, size_t i2,
                   int x0, int y0, int x1, int y1,
                   std::vector<Fragment>& out);

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
