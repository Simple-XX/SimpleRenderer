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

void Rasterizer::RasterizeTo(const Vertex& v0, const Vertex& v1, const Vertex& v2,
                             int x0, int y0, int x1, int y1,
                             std::vector<Fragment>& out) {
  // 获取三角形的最小 box（屏幕空间）
  const Vector4f p0 = v0.GetPosition();
  const Vector4f p1 = v1.GetPosition();
  const Vector4f p2 = v2.GetPosition();

  Vector2f a(p0.x, p0.y);
  Vector2f b(p1.x, p1.y);
  Vector2f c(p2.x, p2.y);

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

      Fragment frag; // material 指针由调用方填写
      frag.screen_coord = {x, y};
      frag.normal = Interpolate(v0.GetNormal(), v1.GetNormal(), v2.GetNormal(), corrected_bary);
      frag.uv     = Interpolate(v0.GetTexCoords(), v1.GetTexCoords(), v2.GetTexCoords(), corrected_bary);
      frag.color  = InterpolateColor(v0.GetColor(), v1.GetColor(), v2.GetColor(), corrected_bary);
      frag.depth  = z;

      out.push_back(frag);
    }
  }
}

void Rasterizer::RasterizeTo(const VertexSoA& soa, size_t i0, size_t i1, size_t i2,
                             int x0, int y0, int x1, int y1,
                             std::vector<Fragment>& out) {
  // 读取三顶点的屏幕空间位置
  const Vector4f& p0 = soa.pos_screen[i0];
  const Vector4f& p1 = soa.pos_screen[i1];
  const Vector4f& p2 = soa.pos_screen[i2];

  // 为BarycentricCoord预构造Vec3f，避免循环内重复构造
  const Vector3f sp0(p0.x, p0.y, p0.z);
  const Vector3f sp1(p1.x, p1.y, p1.z);
  const Vector3f sp2(p2.x, p2.y, p2.z);

  // 计算屏幕空间AABB包围盒
  const float minx_f = std::max(0.0f, std::min({p0.x, p1.x, p2.x}));
  const float miny_f = std::max(0.0f, std::min({p0.y, p1.y, p2.y}));
  const float maxx_f = std::min(float(width_  - 1), std::max({p0.x, p1.x, p2.x}));
  const float maxy_f = std::min(float(height_ - 1), std::max({p0.y, p1.y, p2.y}));

  // 与外部提供的裁剪区域相交（半开区间） -> 闭区间扫描
  int sx = std::max(x0, static_cast<int>(std::floor(minx_f)));
  int sy = std::max(y0, static_cast<int>(std::floor(miny_f)));
  int ex = std::min(x1 - 1, static_cast<int>(std::floor(maxx_f)));
  int ey = std::min(y1 - 1, static_cast<int>(std::floor(maxy_f)));
  if (sx > ex || sy > ey) return;

  // 预计算边函数系数：E(x,y) = A*x + B*y + C
  // 使用相对坐标的边函数定义，避免大常数项导致的数值不稳定
  // 如使用绝对形式Ax+By+C会由于常数C的量级过大，造成浮点抵消，有效位丢失不稳定
  auto cross2 = [](float ax, float ay, float bx, float by) {
    return ax * by - ay * bx;
  };
  // 边向量
  const float e01x = p1.x - p0.x, e01y = p1.y - p0.y; // (p0->p1)
  const float e12x = p2.x - p1.x, e12y = p2.y - p1.y; // (p1->p2)
  const float e20x = p0.x - p2.x, e20y = p0.y - p2.y; // (p2->p0)

  // 有向面积（两倍），用相对面积定义：area2 = cross(p1 - p0, p2 - p0)
  float area2 = cross2(e01x, e01y, p2.x - p0.x, p2.y - p0.y);
  if (std::abs(area2) < 1e-6f) return; // 退化三角形
  const float inv_area2 = 1.0f / area2;
  const bool positive = (area2 > 0.0f);

  // 行优先遍历：有利于 cache 与向量化
  #pragma omp simd
  for (int y = sy; y <= ey; ++y) {
    const float yf = static_cast<float>(y);

    // 注意：此处存在对 out.push_back 的写入，属于有副作用操作，不适合使用
    // omp simd 进行强制向量化，否则可能导致不符合预期的行为（如周期性伪影）。
    // 先保持标量内层，后续如切换为“直写像素回调”再考虑安全的 SIMD 化。
    for (int x = sx; x <= ex; ++x) {
      const float xf = static_cast<float>(x);

      // 相对坐标边函数：
      // E01(p) = cross(p1 - p0, p - p0)
      // E12(p) = cross(p2 - p1, p - p1)
      // E20(p) = cross(p0 - p2, p - p2)
      const float E01 = cross2(e01x, e01y, xf - p0.x, yf - p0.y);
      const float E12 = cross2(e12x, e12y, xf - p1.x, yf - p1.y);
      const float E20 = cross2(e20x, e20y, xf - p2.x, yf - p2.y);

      // 半空间测试（根据朝向选择符号）
      const bool inside = positive ? (E01 >= 0.0f && E12 >= 0.0f && E20 >= 0.0f)
                                   : (E01 <= 0.0f && E12 <= 0.0f && E20 <= 0.0f);
      if (!inside) continue;

      // 重心权重映射：
      // b0 对应 v0，取与对边 (v1,v2) 的子面积 → E12
      // b1 对应 v1 → E20
      // b2 对应 v2 → E01
      const float b0 = E12 * inv_area2;
      const float b1 = E20 * inv_area2;
      const float b2 = E01 * inv_area2;
      const Vector3f bary(b0, b1, b2);

      // 透视矫正插值
      auto perspective_result = PerformPerspectiveCorrection(
          p0.w, p1.w, p2.w,
          p0.z, p1.z, p2.z,
          bary);

      const Vector3f& corrected_bary = perspective_result.corrected_barycentric;
      const float z = perspective_result.interpolated_z;

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
