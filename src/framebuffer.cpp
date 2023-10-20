
/**
 * @file framebuffer.cpp
 * @brief 缓冲区抽象
 * @author Zone.N (Zone.Niuzh@hotmail.com)
 * @version 1.0
 * @date 2022-09-03
 * @copyright MIT LICENSE
 * https://github.com/Simple-XX/SimpleRenderer
 * @par change log:
 * <table>
 * <tr><th>Date<th>Author<th>Description
 * <tr><td>2022-09-03<td>Zone.N<td>创建文件
 * </table>
 */

#include <cmath>
#include <iostream>

#include "exception.hpp"
#include "framebuffer.h"

namespace {
/// 缓冲区计数，用于设置 id
size_t count = 0;
} // namespace

framebuffer_t::framebuffer_t() : id(count++) {}

framebuffer_t::framebuffer_t(const framebuffer_t &_framebuffer)
    : id(count++), width(_framebuffer.width), height(_framebuffer.height),
      color_buffer(_framebuffer.color_buffer),
      depth_buffer(_framebuffer.depth_buffer) {}

framebuffer_t::framebuffer_t(framebuffer_t &&_framebuffer) noexcept
    : id(_framebuffer.id), width(_framebuffer.width),
      height(_framebuffer.height),
      color_buffer(std::move(_framebuffer.color_buffer)),
      depth_buffer(std::move(_framebuffer.depth_buffer)) {}

framebuffer_t::framebuffer_t(size_t _width, size_t _height)
    : id(count++), width(_width), height(_height), color_buffer(width, height),
      depth_buffer(width, height) {}

auto framebuffer_t::operator=(const framebuffer_t &_framebuffer)
    -> framebuffer_t & {
  if (this == &_framebuffer) {
    return *this;
  }
  if (width != _framebuffer.get_width() ||
      height != _framebuffer.get_height()) {
    return *this;
  }
  color_buffer = _framebuffer.color_buffer;
  depth_buffer = _framebuffer.depth_buffer;
  return *this;
}

auto framebuffer_t::operator=(framebuffer_t &&_framebuffer) noexcept
    -> framebuffer_t & {
  if (this == &_framebuffer) {
    return *this;
  }
  if (width != _framebuffer.get_width() ||
      height != _framebuffer.get_height()) {
    return *this;
  }
  id = _framebuffer.id;
  color_buffer = _framebuffer.color_buffer;
  depth_buffer = _framebuffer.depth_buffer;
  return *this;
}

auto framebuffer_t::get_width() const -> size_t { return width; }

auto framebuffer_t::get_height() const -> size_t { return height; }

void framebuffer_t::clear(const color_t &_color, const depth_t &_depth) {
  if (std::isnan(_depth)) {
    throw SimpleRenderer::exception("std::isnan(_depth)");
  }
  color_buffer.clear(_color);
  depth_buffer.clear(_depth);
}

void framebuffer_t::pixel(size_t _x, size_t _y, const color_t &_color,
                          const depth_t &_depth) {
  if (_x >= width) {
    throw SimpleRenderer::exception("_x >= width");
  }
  if (_y >= height) {
    throw SimpleRenderer::exception("_y >= height");
  }
  if (std::isnan(_depth)) {
    throw SimpleRenderer::exception("std::isnan(_depth)");
  }
  /// @todo 性能瓶颈
  color_buffer(_x, _y) = _color;
  depth_buffer(_x, _y) = _depth;
}

auto framebuffer_t::get_color_buffer() -> framebuffer_t::color_buffer_t & {
  return color_buffer;
}

auto framebuffer_t::get_color_buffer() const
    -> const framebuffer_t::color_buffer_t & {
  return color_buffer;
}

auto framebuffer_t::get_depth_buffer() -> framebuffer_t::depth_buffer_t & {
  return depth_buffer;
}

auto framebuffer_t::get_depth_buffer(size_t _x, size_t _y)
    -> framebuffer_t::depth_t & {
  return depth_buffer(_x, _y);
}

auto framebuffer_t::get_depth_buffer() const
    -> const framebuffer_t::depth_buffer_t & {
  return depth_buffer;
}

auto framebuffer_t::get_depth_buffer(size_t _x, size_t _y) const
    -> framebuffer_t::depth_t {
  return depth_buffer(_x, _y);
}

void framebuffer_t::line(float _x0, float _y0, float _x1, float _y1,
                         const color_t &_color) {
  auto p0_x = static_cast<int32_t>(_x0);
  auto p0_y = static_cast<int32_t>(_y0);
  auto p1_x = static_cast<int32_t>(_x1);
  auto p1_y = static_cast<int32_t>(_y1);
  if (p0_x >= static_cast<int32_t>(WIDTH) ||
      p0_y >= static_cast<int32_t>(HEIGHT) ||
      p1_x >= static_cast<int32_t>(WIDTH) ||
      p1_y >= static_cast<int32_t>(HEIGHT)) {
    SPDLOG_LOGGER_WARN(
        SRLOG, "WIDTH: {}, HEIGHT: {}, p0_x: {}, p0_y: {}, p1_x: {}, p1_y: {}",
        WIDTH, HEIGHT, p0_x, p0_y, p1_x, p1_y);
  }

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
      if ((unsigned)y >= width || (unsigned)x >= height) {
        continue;
      }
      pixel(y, x, _color);
    } else {
      /// @todo 这里要用裁剪替换掉
      if ((unsigned)x >= width || (unsigned)y >= height) {
        continue;
      }
      pixel(x, y, _color);
    }
    de += std::abs(dy2);
    if (de >= dx2) {
      y += yi;
      de -= dx2;
    }
  }
}

void framebuffer_t::triangle(const shader_base_t &_shader,
                             const light_t &_light,
                             const model_t::normal_t &_normal,
                             const model_t::face_t &_face) {
  auto v0 = _face.v0;
  auto v1 = _face.v1;
  auto v2 = _face.v2;

  // 获取三角形的最小 box
  auto min = v0.coord;
  auto max = v1.coord;
  auto max_x = std::max(_face.v0.coord.x(),
                        std::max(_face.v1.coord.x(), _face.v2.coord.x()));
  auto max_y = std::max(_face.v0.coord.y(),
                        std::max(_face.v1.coord.y(), _face.v2.coord.y()));
  max.x() = max_x > max.x() ? max_x : max.x();
  max.y() = max_y > max.y() ? max_y : max.y();
  max.z() = 0;
  auto min_x = std::min(_face.v0.coord.x(),
                        std::min(_face.v1.coord.x(), _face.v2.coord.x()));
  auto min_y = std::min(_face.v0.coord.y(),
                        std::min(_face.v1.coord.y(), _face.v2.coord.y()));
  min.x() = min_x < min.x() ? min_x : min.x();
  min.y() = min_y < min.y() ? min_y : min.y();
  min.z() = 0;

#pragma omp parallel for num_threads(NPROC) collapse(2) default(none)          \
    shared(min, max, v0, v1, v2, _shader) firstprivate(_normal, _light)
  for (auto x = int32_t(min.x()); x <= int32_t(max.x()); x++) {
    for (auto y = int32_t(min.y()); y <= int32_t(max.y()); y++) {
      /// @todo 这里要用裁剪替换掉
      if ((unsigned)x >= width || (unsigned)y >= height) {
        continue;
      }
      auto [is_inside, barycentric_coord] = get_barycentric_coord(
          v0.coord, v1.coord, v2.coord,
          vector3f_t(static_cast<float>(x), static_cast<float>(y), 0));
      // 如果点在三角形内再进行下一步
      if (!is_inside) {
        continue;
      }
      // 计算该点的深度，通过重心坐标插值计算
      auto z = interpolate_depth(v0.coord.z(), v1.coord.z(), v2.coord.z(),
                                 barycentric_coord);
      // 深度在已有颜色之上
      if (z < depth_buffer(x, y)) {
        continue;
      }
      // 构造着色器输入
      auto shader_fragment_in = shader_fragment_in_t(
          barycentric_coord, _normal, _light.dir, v0.color, v1.color, v2.color);
      // 计算颜色，颜色为通过 shader 片段着色器计算
      auto shader_fragment_out = _shader.fragment(shader_fragment_in);
      // 如果不需要绘制则跳过
      if (!shader_fragment_out.is_need_draw) {
        continue;
      }
      // 构造颜色
      auto color = color_t(shader_fragment_out.color);
      // 填充像素
      pixel(x, y, color, z);
    }
  }
}
/// @todo
static bool draw_wireframe = false;
void framebuffer_t::model(const shader_base_t &_shader, const light_t &_light,
                          const model_t &_model) {
  SPDLOG_LOGGER_INFO(SRLOG, "draw {}", _model.obj_path);

  if (draw_wireframe) {
#pragma omp parallel for num_threads(NPROC) default(none) shared(_shader)      \
    firstprivate(_model)
    for (const auto &f : _model.get_face()) {
      /// @todo 巨大性能开销
      auto face = _shader.vertex(shader_vertex_in_t(f)).face;
      line(face.v0.coord.x(), face.v0.coord.y(), face.v1.coord.x(),
           face.v1.coord.y(), color_t::WHITE);
      line(face.v1.coord.x(), face.v1.coord.y(), face.v2.coord.x(),
           face.v2.coord.y(), color_t::WHITE);
      line(face.v2.coord.x(), face.v2.coord.y(), face.v0.coord.x(),
           face.v0.coord.y(), color_t::WHITE);
    }
  } else {
#pragma omp parallel for num_threads(NPROC) default(none) shared(_shader)      \
    firstprivate(_model, _light)
    for (const auto &f : _model.get_face()) {
      /// @todo 巨大性能开销
      auto face = _shader.vertex(shader_vertex_in_t(f)).face;
      triangle(_shader, _light, face.normal, face);
    }
  }
}

void framebuffer_t::scene(const shader_base_t &_shader, const scene_t &_scene) {
  const auto &light = _scene.get_light();
  for (const auto &i : _scene.get_models()) {
    model(_shader, light, i);
  }
}

/// @todo 巨大性能开销
auto framebuffer_t::get_barycentric_coord(const vector3f_t &_p0,
                                          const vector3f_t &_p1,
                                          const vector3f_t &_p2,
                                          const vector3f_t &_pa)
    -> std::pair<bool, vector3f_t> {
  auto p1p0 = _p1 - _p0;
  auto p2p0 = _p2 - _p0;
  auto pap0 = _pa - _p0;

  auto deno = (p1p0.x() * p2p0.y() - p1p0.y() * p2p0.x());
  if (std::abs(deno) < std::numeric_limits<decltype(deno)>::epsilon()) {
    return std::pair<bool, const vector3f_t>{false, vector3f_t()};
  }

  auto s = (p2p0.y() * pap0.x() - p2p0.x() * pap0.y()) / deno;
  if ((s > 1) || (s < 0)) {
    return std::pair<bool, const vector3f_t>{false, vector3f_t()};
  }

  auto t = (p1p0.x() * pap0.y() - p1p0.y() * pap0.x()) / deno;
  if ((t > 1) || (t < 0)) {
    return std::pair<bool, const vector3f_t>{false, vector3f_t()};
  }

  if ((1 - s - t > 1) || (1 - s - t < 0)) {
    return std::pair<bool, const vector3f_t>{false, vector3f_t()};
  }

  return std::pair<bool, const vector3f_t>{true, vector3f_t(1 - s - t, s, t)};
}

auto framebuffer_t::interpolate_depth(float _depth0, float _depth1,
                                      float _depth2,
                                      const vector3f_t &_barycentric_coord)
    -> float {
  auto depth = _depth0 * _barycentric_coord.x();
  depth += _depth1 * _barycentric_coord.y();
  depth += _depth2 * _barycentric_coord.z();
  return depth;
}
