#include "rasterizer.hpp"

#include <omp.h>

namespace simple_renderer {

Rasterizer::Rasterizer(size_t width, size_t height)
    : width_(width), height_(height) {
  SPDLOG_INFO("Rasterizer init with {}, {}", width, height);
}

std::vector<Fragment> Rasterizer::Rasterize(const Vertex& v0, const Vertex& v1,
                                            const Vertex& v2) {
  std::vector<Fragment> fragments;

  // 获取三角形的最小 box
  Vector2f a = Vector2f(v0.GetPosition().x, v0.GetPosition().y);
  Vector2f b = Vector2f(v1.GetPosition().x, v1.GetPosition().y);
  Vector2f c = Vector2f(v2.GetPosition().x, v2.GetPosition().y);

  Vector2f bboxMin =
      Vector2f{std::min({a.x, b.x, c.x}), std::min({a.y, b.y, c.y})};
  Vector2f bboxMax =
      Vector2f{std::max({a.x, b.x, c.x}), std::max({a.y, b.y, c.y})};

  // Clamp the bounding box to the screen dimensions
  bboxMin.x = std::max(0.0f, bboxMin.x);
  bboxMin.y = std::max(0.0f, bboxMin.y);
  bboxMax.x = std::min(float(width_ - 1), bboxMax.x);
  bboxMax.y = std::min(float(height_ - 1), bboxMax.y);

  std::vector<std::vector<Fragment>> fragments_per_thread(kNProc);

#pragma omp parallel num_threads(kNProc) default(none) shared( \
        bboxMin, bboxMax, v0, v1, v2, fragments, width_, fragments_per_thread)
  {
    int thread_id = omp_get_thread_num();
    std::vector<Fragment>& local_fragments = fragments_per_thread[thread_id];
#pragma omp for collapse(2)
    for (auto x = int32_t(bboxMin.x); x <= int32_t(bboxMax.x); x++) {
      for (auto y = int32_t(bboxMin.y); y <= int32_t(bboxMax.y); y++) {
        auto [is_inside, barycentric_coord] = GetBarycentricCoord(
            v0.GetPosition(), v1.GetPosition(), v2.GetPosition(),
            Vector3f(static_cast<float>(x), static_cast<float>(y), 0));
        // 如果点在三角形内再进行下一步
        if (!is_inside) {
          continue;
        }

        // 透视矫正插值
        // 1. 获取三个顶点的1/w值
        float w0_inv = v0.GetPosition().w;
        float w1_inv = v1.GetPosition().w;  
        float w2_inv = v2.GetPosition().w;
        
        // 2. 插值1/w
        float w_inv_interpolated = Interpolate(w0_inv, w1_inv, w2_inv, barycentric_coord);
        
        // 3. 计算透视矫正的重心坐标
        Vector3f corrected_bary(
          barycentric_coord.x * w0_inv / w_inv_interpolated,
          barycentric_coord.y * w1_inv / w_inv_interpolated,
          barycentric_coord.z * w2_inv / w_inv_interpolated
        );
        
        // 4. 使用矫正的重心坐标进行插值
        auto z = Interpolate(v0.GetPosition().z, v1.GetPosition().z,
                             v2.GetPosition().z, corrected_bary);


        Fragment fragment;
        fragment.screen_coord = {x, y};
        fragment.normal = Interpolate(v0.GetNormal(), v1.GetNormal(),
                                      v2.GetNormal(), corrected_bary);
        fragment.uv = Interpolate(v0.GetTexCoords(), v1.GetTexCoords(),
                                  v2.GetTexCoords(), corrected_bary);
        fragment.color = InterpolateColor(v0.GetColor(), v1.GetColor(),
                                          v2.GetColor(), corrected_bary);
        fragment.depth = z;

        local_fragments.push_back(fragment);
      }
    }
  }

  for (const auto& local_fragments : fragments_per_thread) {
    fragments.insert(fragments.end(), local_fragments.begin(),
                     local_fragments.end());
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
T Rasterizer::Interpolate(const T& v0, const T& v1, const T& v2,
                          const Vector3f& barycentric_coord) {
  return v0 * barycentric_coord.x + v1 * barycentric_coord.y +
         v2 * barycentric_coord.z;
}

Color Rasterizer::InterpolateColor(const Color& color0, const Color& color1,
                                   const Color& color2,
                                   const Vector3f& barycentric_coord) {
  auto color_r = FloatToUint8_t(
      static_cast<float>(color0[Color::kColorIndexRed]) * barycentric_coord.x +
      static_cast<float>(color1[Color::kColorIndexRed]) * barycentric_coord.y +
      static_cast<float>(color2[Color::kColorIndexRed]) * barycentric_coord.z);
  auto color_g =
      FloatToUint8_t(static_cast<float>(color0[Color::kColorIndexGreen]) *
                         barycentric_coord.x +
                     static_cast<float>(color1[Color::kColorIndexGreen]) *
                         barycentric_coord.y +
                     static_cast<float>(color2[Color::kColorIndexGreen]) *
                         barycentric_coord.z);
  auto color_b = FloatToUint8_t(
      static_cast<float>(color0[Color::kColorIndexBlue]) * barycentric_coord.x +
      static_cast<float>(color1[Color::kColorIndexBlue]) * barycentric_coord.y +
      static_cast<float>(color2[Color::kColorIndexBlue]) * barycentric_coord.z);
  return Color(color_r, color_g, color_b);
}

// Calculate the normal vector based on the vertices
// 根据顶点计算法向量
Vector3f Rasterizer::CalculateNormal(const Vector3f& v0, const Vector3f& v1,
                                     const Vector3f& v2) {
  Vector3f edge1 = v1 - v0;
  Vector3f edge2 = v2 - v0;
  return glm::normalize(
      // Normalize the cross product to get the
      // normal 归一化叉积以获得法向量
      glm::cross(edge1, edge2));
}

}  // namespace simple_renderer