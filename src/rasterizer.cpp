#include "rasterizer.hpp"

namespace simple_renderer {

Rasterizer::Rasterizer(size_t width, size_t height)
    : width_(width), height_(height) {
  depth_buffer_ = std::shared_ptr<float[]>(new float[width * height],
                                           std::default_delete<float[]>());

  std::fill(depth_buffer_.get(), depth_buffer_.get() + width * height,
            std::numeric_limits<float>::lowest());

  SPDLOG_INFO("Rasterizer init with {}, {}", width, height);
}

std::vector<Fragment> Rasterizer::rasterize(const Vertex& v0, const Vertex& v1,
                                            const Vertex& v2) {
  std::vector<Fragment> fragments;
  // 获取三角形的最小 box
  Vector2f a = Vector2f(v0.position().x, v0.position().y);
  Vector2f b = Vector2f(v1.position().x, v1.position().y);
  Vector2f c = Vector2f(v2.position().x, v2.position().y);

  Vector2f bboxMin =
      Vector2f{std::min({a.x, b.x, c.x}), std::min({a.y, b.y, c.y})};
  Vector2f bboxMax =
      Vector2f{std::max({a.x, b.x, c.x}), std::max({a.y, b.y, c.y})};
#pragma omp parallel for num_threads(kNProc) collapse(2) default(none) \
    shared(bboxMin, bboxMax, v0, v1, v2, shader, face) firstprivate(light)
  for (auto x = int32_t(bboxMin.x); x <= int32_t(bboxMax.x); x++) {
    for (auto y = int32_t(bboxMin.y); y <= int32_t(bboxMax.y); y++) {
      auto [is_inside, barycentric_coord] = GetBarycentricCoord(
          v0.position(), v1.position(), v2.position(),
          Vector3f(static_cast<float>(x), static_cast<float>(y), 0));
      // 如果点在三角形内再进行下一步
      if (!is_inside) {
        continue;
      }
      // 计算该点的深度，通过重心坐标插值计算
      auto z = interpolate(v0.position().z, v1.position().z, v2.position().z,
                           barycentric_coord);
      // 深度在已有颜色之上 TODO: maybe need to change the process sequance
      // here, z_buffer check maybe outside of the rasterize
      if (z < depth_buffer_[y * width_ + x]) {
        continue;
      }
      depth_buffer_[y * width_ + x] = z;
      Fragment fragment;
      fragment.screen_coord = {x, y};
      fragment.normal =
          interpolate(v0.normal(), v1.normal(), v2.normal(), barycentric_coord);
      fragment.uv = interpolate(v0.texCoords(), v1.texCoords(), v2.texCoords(),
                                barycentric_coord);
      fragment.color = interpolateColor(v0.color(), v1.color(), v2.color(),
                                        barycentric_coord);
      fragments.push_back(fragment);
    }
  }
  return fragments;
}

std::pair<bool, Vector3f> Rasterizer::GetBarycentricCoord(const Vector3f& p0,
                                                          const Vector3f& p1,
                                                          const Vector3f& p2,
                                                          const Vector3f& pa) {
  Vector3f v0 = Vector3f{p2.x - p0.x, p1.x - p0.x, p0.x - pa.x};
  Vector3f v1 = Vector3f{p2.y - p0.y, p1.y - p0.y, p0.y - pa.y};

  Vector3f u = glm::cross(v0, v1);

  // 如果 u.z == 0 说明三角形退化
  const float epsilon = 1e-6f;
  if (std::abs(u.z) < epsilon) {
    return std::pair<bool, const Vector3f>{false, Vector3f(0, 0, 0)};
  }

  auto x = 1.0f - (u.x + u.y) / u.z;
  auto y = u.y / u.z;
  auto z = u.x / u.z;

  // 如果重心坐标不在 [0, 1] 之间则说明点在三角形外
  if (x < 0 || y < 0 || z < 0 || x > 1 || y > 1 || z > 1) {
    return std::pair<bool, const Vector3f>{false, Vector3f(0, 0, 0)};
  }

  return std::pair<bool, const Vector3f>{true, Vector3f(x, y, z)};
}

template <typename T>
T Rasterizer::interpolate(const T& v0, const T& v1, const T& v2,
                          const Vector3f& barycentric_coord) {
  return v0 * barycentric_coord.x + v1 * barycentric_coord.y +
         v2 * barycentric_coord.z;
}

Color Rasterizer::interpolateColor(const Color& color0, const Color& color1,
                                   const Color& color2,
                                   const Vector3f& barycentric_coord) {
  auto color_r = float_to_uint8_t(
      static_cast<float>(color0[Color::kColorIndexRed]) * barycentric_coord.x +
      static_cast<float>(color1[Color::kColorIndexRed]) * barycentric_coord.y +
      static_cast<float>(color2[Color::kColorIndexRed]) * barycentric_coord.z);
  auto color_g =
      float_to_uint8_t(static_cast<float>(color0[Color::kColorIndexGreen]) *
                           barycentric_coord.x +
                       static_cast<float>(color1[Color::kColorIndexGreen]) *
                           barycentric_coord.y +
                       static_cast<float>(color2[Color::kColorIndexGreen]) *
                           barycentric_coord.z);
  auto color_b = float_to_uint8_t(
      static_cast<float>(color0[Color::kColorIndexBlue]) * barycentric_coord.x +
      static_cast<float>(color1[Color::kColorIndexBlue]) * barycentric_coord.y +
      static_cast<float>(color2[Color::kColorIndexBlue]) * barycentric_coord.z);
  return Color(color_r, color_g, color_b);
}

}  // namespace simple_renderer