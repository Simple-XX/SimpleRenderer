#ifndef SIMPLERENDER_SRC_INCLUDE_RASTERIZER_HPP_
#define SIMPLERENDER_SRC_INCLUDE_RASTERIZER_HPP_

#include "shader.hpp"

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

  std::vector<Fragment> rasterize(const Vertex& v0, const Vertex& v1,
                                  const Vertex& v2);

  void setDepthBuffer(size_t index, float zvalue) {
    depth_buffer_[index] = zvalue;
  }

 private:
  size_t width_, height_;
  std::shared_ptr<float[]> depth_buffer_;

  template <typename T>
  T interpolate(const T& v0, const T& v1, const T& v2,
                const Vector3f& barycentric_coord);

  Color interpolateColor(const Color& color0, const Color& color1,
                         const Color& color2,
                         const Vector3f& barycentric_coord);

  std::pair<bool, Vector3f> GetBarycentricCoord(const Vector3f& p0,
                                                const Vector3f& p1,
                                                const Vector3f& p2,
                                                const Vector3f& pa);
};

}  // namespace simple_renderer

#endif  // SIMPLERENDER_SRC_INCLUDE_RASTERIZER_HPP_
