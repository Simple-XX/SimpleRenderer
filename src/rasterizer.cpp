#include "rasterizer.hpp"

#include <omp.h>
#include <algorithm>
#include <cmath>

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
        auto perspective_result = PerformPerspectiveCorrection(
            v0.GetPosition().w, v1.GetPosition().w, v2.GetPosition().w,
            v0.GetPosition().z, v1.GetPosition().z, v2.GetPosition().z,
            barycentric_coord);
        
        const Vector3f& corrected_bary = perspective_result.corrected_barycentric;
        float z = perspective_result.interpolated_z;


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

void Rasterizer::RasterizeTo(const VertexSoA& soa, size_t i0, size_t i1, size_t i2,
                             int x0, int y0, int x1, int y1,
                             std::vector<Fragment>& out) {
  // 读取三顶点的屏幕空间位置
  const Vector4f& p0 = soa.pos_screen[i0];
  const Vector4f& p1 = soa.pos_screen[i1];
  const Vector4f& p2 = soa.pos_screen[i2];

  Vector2f a = Vector2f(p0.x, p0.y);
  Vector2f b = Vector2f(p1.x, p1.y);
  Vector2f c = Vector2f(p2.x, p2.y);

  Vector2f bboxMin = Vector2f{std::min({a.x, b.x, c.x}), std::min({a.y, b.y, c.y})};
  Vector2f bboxMax = Vector2f{std::max({a.x, b.x, c.x}), std::max({a.y, b.y, c.y})};

  // Clamp 到屏幕尺寸
  float minx = std::max(0.0f, bboxMin.x);
  float miny = std::max(0.0f, bboxMin.y);
  float maxx = std::min(float(width_ - 1), bboxMax.x);
  float maxy = std::min(float(height_ - 1), bboxMax.y);

  // 与外部提供的裁剪区域相交（半开区间） -> 闭区间扫描
  int sx = std::max(x0, static_cast<int>(std::floor(minx)));
  int sy = std::max(y0, static_cast<int>(std::floor(miny)));
  int ex = std::min(x1 - 1, static_cast<int>(std::floor(maxx)));
  int ey = std::min(y1 - 1, static_cast<int>(std::floor(maxy)));
  if (sx > ex || sy > ey) return;

  for (int x = sx; x <= ex; ++x) {
    for (int y = sy; y <= ey; ++y) {
      auto [is_inside, bary] = GetBarycentricCoord(
          Vector3f(p0.x, p0.y, p0.z), Vector3f(p1.x, p1.y, p1.z), Vector3f(p2.x, p2.y, p2.z),
          Vector3f(static_cast<float>(x), static_cast<float>(y), 0));
      if (!is_inside) continue;

      // 透视矫正插值
      auto perspective_result = PerformPerspectiveCorrection(
          p0.w, p1.w, p2.w,
          p0.z, p1.z, p2.z,
          bary);
      
      const Vector3f& corrected_bary = perspective_result.corrected_barycentric;
      float z = perspective_result.interpolated_z;

      Fragment frag; // Note: material 指针由调用方填写
      frag.screen_coord = {x, y};
      frag.normal = Interpolate(soa.normal[i0], soa.normal[i1], soa.normal[i2], corrected_bary);
      frag.uv     = Interpolate(soa.uv[i0],     soa.uv[i1],     soa.uv[i2],     corrected_bary);
      frag.color  = InterpolateColor(soa.color[i0], soa.color[i1], soa.color[i2], corrected_bary);
      frag.depth  = z;

      out.push_back(frag);
    }
  }
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
                          const Vector3f& barycentric_coord) const {
  return v0 * barycentric_coord.x + v1 * barycentric_coord.y +
         v2 * barycentric_coord.z;
}

Color Rasterizer::InterpolateColor(const Color& color0, const Color& color1,
                                   const Color& color2,
                                   const Vector3f& barycentric_coord) const {
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

// 透视矫正helper函数：在透视投影下，1/w 在屏幕空间中是线性的// 因此需要先对 1/w 进行插值，再用结果矫正其他属性
Rasterizer::PerspectiveCorrectionResult Rasterizer::PerformPerspectiveCorrection(
    float w0, float w1, float w2,
    float z0, float z1, float z2,
    const Vector3f& original_barycentric) const {
    
  // 1. 插值 1/w （注意：这里传入的w0,w1,w2是原始的w值，需要先求倒数）
  float w0_inv = 1.0f / w0;
  float w1_inv = 1.0f / w1;
  float w2_inv = 1.0f / w2;
  float w_inv_interpolated = Interpolate(w0_inv, w1_inv, w2_inv, original_barycentric);
  
  // 2. 计算透视矫正的重心坐标
  Vector3f corrected_barycentric(
      original_barycentric.x * w0_inv / w_inv_interpolated,
      original_barycentric.y * w1_inv / w_inv_interpolated,
      original_barycentric.z * w2_inv / w_inv_interpolated
  );
  
  // 3. 使用矫正的重心坐标插值深度值
  float interpolated_z = Interpolate(z0, z1, z2, corrected_barycentric);
  
  return {corrected_barycentric, interpolated_z};
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
