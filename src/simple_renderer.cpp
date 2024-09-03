
/**
 * @file simple_renderer.cpp
 * @brief SimpleRenderer 实现
 * @author Zone.N (Zone.Niuzh@hotmail.com)
 * @version 1.0
 * @date 2023-10-23
 * @copyright MIT LICENSE
 * https://github.com/Simple-XX/SimpleRenderer
 * @par change log:
 * <table>
 * <tr><th>Date<th>Author<th>Description
 * <tr><td>2023-10-23<td>Zone.N<td>创建文件
 * </table>
 */

#include "simple_renderer.h"

#include <array>
#include <cstdint>
#include <limits>
#include <span>
#include <string_view>
#include <vector>

#include "config.h"
#include "default_shader.h"
#include "light.h"
#include "log_system.h"
#include "model.hpp"
#include "shader_base.h"

namespace simple_renderer {

SimpleRenderer::SimpleRenderer(size_t width, size_t height, uint32_t *buffer,
                               DrawPixelFunc draw_pixel_func)
    : height_(height),
      width_(width),
      buffer_(buffer),
      depth_buffer_(std::shared_ptr<Depth[]>(new Depth[width_ * height_],
                                             std::default_delete<Depth[]>())),
      draw_pixel_func_(draw_pixel_func),
      log_system_(LogSystem(kLogFilePath, kLogFileMaxSize, kLogFileMaxCount)) {
  if (buffer_ == nullptr) {
    SPDLOG_ERROR("buffer_ == nullptr");
    throw std::invalid_argument("buffer_ == nullptr");
  }

  if (depth_buffer_ == nullptr) {
    SPDLOG_ERROR("depth_buffer_ == nullptr");
    throw std::invalid_argument("depth_buffer_ == nullptr");
  }

  std::fill(depth_buffer_.get(), depth_buffer_.get() + width_ * height_,
            std::numeric_limits<Depth>::lowest());

  SPDLOG_INFO("SimpleRenderer init with {}, {}", width_, height_);
}

bool SimpleRenderer::render(const Model &model) {
  SPDLOG_INFO("render model: {}", model.modelPath());
  auto shader = DefaultShader();
  auto light = Light();
  DrawModel(shader, light, model, 0, 1);
  return true;
}

void SimpleRenderer::DrawLine(float x0, float y0, float x1, float y1,
                              const Color &_color) {
  auto p0_x = static_cast<int32_t>(x0);
  auto p0_y = static_cast<int32_t>(y0);
  auto p1_x = static_cast<int32_t>(x1);
  auto p1_y = static_cast<int32_t>(y1);

  // SPDLOG_DEBUG("x0({}, {}), y0({}, {}), p0({}, {}), p1({}, {})", x0, y0, x1,
  // y1,
  //              p0_x, p0_y, p1_x, p1_y);

  auto steep = false;
  if (std::abs(p0_x - p1_x) < std::abs(p0_y - p1_y)) {
    std::swap(p0_x, p0_y);
    std::swap(p1_x, p1_y);
    steep = true;
  }
  // 终点 x 坐标在起点 左边
  if (p0_x > p1_x) {
    std::swap(p0_x, p1_x);
    std::swap(p0_y, p1_y);
  }

  auto de = 0;
  auto dy2 = std::abs(p1_y - p0_y) << 1;
  auto dx2 = std::abs(p1_x - p0_x) << 1;
  auto y = p0_y;
  auto yi = 1;
  if (p1_y <= p0_y) {
    yi = -1;
  }
  for (auto x = p0_x; x <= p1_x; x++) {
    if (steep) {
      /// @todo 这里要用裁剪替换掉
      if ((unsigned)y >= width_ || (unsigned)x >= height_) {
        // SPDLOG_WARN(
        // "width: {}, height: {}, p0_x: {}, p0_y: {}, p1_x: "
        // "{}, p1_y: {}, x: {}, y: {}",
        // width_, height_, p0_x, p0_y, p1_x, p1_y, x, y);
        continue;
      }
      draw_pixel_func_(y, x, _color, buffer_);
    } else {
      /// @todo 这里要用裁剪替换掉
      if ((unsigned)x >= width_ || (unsigned)y >= height_) {
        // SPDLOG_WARN(
        //     "width: {}, height: {}, p0_x: {}, p0_y: {}, p1_x: "
        //     "{}, p1_y: {}, x: {}, y: {}",
        //     width_, height_, p0_x, p0_y, p1_x, p1_y, x, y);
        continue;
      }
      draw_pixel_func_(x, y, _color, buffer_);
    }
    de += std::abs(dy2);
    if (de >= dx2) {
      y += yi;
      de -= dx2;
    }
  }
}

void SimpleRenderer::DrawTriangle(const ShaderBase &shader, const Light &light,
                                  const Vector3f &normal, const Face &face) {
  auto v0 = face.vertex(0);
  auto v1 = face.vertex(1);
  auto v2 = face.vertex(2);

  // 获取三角形的最小 box
  Vector2f a = Vector2f(v0.position().x, v0.position().y);
  Vector2f b = Vector2f(v1.position().x, v1.position().y);
  Vector2f c = Vector2f(v2.position().x, v2.position().y);

  Vector2f bboxMin =
      Vector2f{std::min({a.x, b.x, c.x}), std::min({a.y, b.y, c.y})};
  Vector2f bboxMax =
      Vector2f{std::max({a.x, b.x, c.x}), std::max({a.y, b.y, c.y})};

#pragma omp parallel for num_threads(kNProc) collapse(2) default(none) \
    shared(bboxMin, bboxMax, v0, v1, v2, shader) firstprivate(normal, light)
  for (auto x = int32_t(bboxMin.x); x <= int32_t(bboxMax.x); x++) {
    for (auto y = int32_t(bboxMin.y); y <= int32_t(bboxMax.y); y++) {
      /// @todo 这里要用裁剪替换掉
      if ((unsigned)x >= width_ || (unsigned)y >= height_) {
        continue;
      }
      auto [is_inside, barycentric_coord] = GetBarycentricCoord(
          v0.position(), v1.position(), v2.position(),
          Vector3f(static_cast<float>(x), static_cast<float>(y), 0));
      // 如果点在三角形内再进行下一步
      if (!is_inside) {
        continue;
      }
      // 计算该点的深度，通过重心坐标插值计算
      auto z = InterpolateDepth(v0.position().z, v1.position().z,
                                v2.position().z, barycentric_coord);
      // 深度在已有颜色之上
      if (z < depth_buffer_[y * width_ + x]) {
        continue;
      }
      // 构造着色器输入
      auto shader_fragment_in =
          ShaderFragmentIn(barycentric_coord, normal, light.dir, v0.color(),
                           v1.color(), v2.color());
      // 计算颜色，颜色为通过 shader 片段着色器计算
      auto shader_fragment_out = shader.Fragment(shader_fragment_in);
      // 如果不需要绘制则跳过
      if (!shader_fragment_out.is_need_draw_) {
        continue;
      }
      // 构造颜色
      auto color = Color(shader_fragment_out.color_);
      // 填充像素
      draw_pixel_func_(x, y, color, buffer_);
      depth_buffer_[y * width_ + x] = z;
    }
  }
}

void SimpleRenderer::DrawModel(const ShaderBase &shader, const Light &light,
                               const Model &model, bool draw_line,
                               bool draw_triangle) {
  SPDLOG_INFO("draw {}", model.modelPath());

  if (draw_line) {
#pragma omp parallel for num_threads(kNProc) default(none) shared(shader) \
    firstprivate(model)
    for (const auto &f : model.faces()) {
      /// @todo 巨大性能开销
      auto face = shader.Vertex(ShaderVertexIn(f)).face_;
      auto a = face.vertex(0).position();
      auto b = face.vertex(1).position();
      auto c = face.vertex(2).position();
      DrawLine(a.x, a.y, b.x, b.y, Color::kRed);
      DrawLine(b.x, b.y, c.x, c.y, Color::kGreen);
      DrawLine(c.x, c.y, a.x, a.y, Color::kBlue);
    }
  }
  if (draw_triangle) {
#pragma omp parallel for num_threads(kNProc) default(none) shared(shader) \
    firstprivate(model, light)
    for (const auto &f : model.faces()) {
      /// @todo 巨大性能开销
      auto face = shader.Vertex(ShaderVertexIn(f)).face_;
      DrawTriangle(shader, light, face.normal(), face);
    }
  }
}

/// @todo 巨大性能开销
auto SimpleRenderer::GetBarycentricCoord(const Vector3f &p0, const Vector3f &p1,
                                         const Vector3f &p2, const Vector3f &pa)
    -> std::pair<bool, Vector3f> {
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

auto SimpleRenderer::InterpolateDepth(float depth0, float depth1, float depth2,
                                      const Vector3f &_barycentric_coord)
    -> float {
  auto depth = depth0 * _barycentric_coord.x;
  depth += depth1 * _barycentric_coord.y;
  depth += depth2 * _barycentric_coord.z;
  return depth;
}

}  // namespace simple_renderer
