
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

#include "cassert"
#include "cmath"
#include "iostream"

#include "framebuffer.h"

uint32_t framebuffer_t::count = 0;

/// @todo 巨大性能开销
std::pair<bool, vector4f_t>
framebuffer_t::get_barycentric_coord(const vector4f_t& _p0,
                                     const vector4f_t& _p1,
                                     const vector4f_t& _p2,
                                     const vector4f_t& _p) {
    auto                          ab   = _p1 - _p0;
    auto                          ac   = _p2 - _p0;
    auto                          ap   = _p - _p0;

    vector_element_concept_t auto deno = (ab.x * ac.y - ab.y * ac.x);
    if (std::abs(deno) < std::numeric_limits<decltype(deno)>::epsilon()) {
        return std::pair<bool, const vector4f_t> { false, vector4f_t() };
    }

    vector_element_concept_t auto s = (ac.y * ap.x - ac.x * ap.y) / deno;
    if ((s > 1) || (s < 0)) {
        return std::pair<bool, const vector4f_t> { false, vector4f_t() };
    }

    vector_element_concept_t auto t = (ab.x * ap.y - ab.y * ap.x) / deno;
    if ((t > 1) || (t < 0)) {
        return std::pair<bool, const vector4f_t> { false, vector4f_t() };
    }

    if ((1 - s - t > 1) || (1 - s - t < 0)) {
        return std::pair<bool, const vector4f_t> { false, vector4f_t() };
    }

    return std::pair<bool, const vector4f_t> { true,
                                               vector4f_t(1 - s - t, s, t) };
}

float framebuffer_t::interpolate_depth(float _depth0, float _depth1,
                                       float             _depth2,
                                       const vector4f_t& _barycentric_coord) {
    auto z = _depth0 * _barycentric_coord.x;
    z      += _depth1 * _barycentric_coord.y;
    z      += _depth2 * _barycentric_coord.z;
    return z;
}

framebuffer_t::framebuffer_t(void)
    : width(0), height(0), color_buffer(), depth_buffer() {
    id = count++;
    return;
}

framebuffer_t::framebuffer_t(const framebuffer_t& _framebuffer)
    : width(_framebuffer.width),
      height(_framebuffer.height),
      color_buffer(_framebuffer.color_buffer),
      depth_buffer(_framebuffer.depth_buffer) {
    id = count++;
    return;
}

framebuffer_t::framebuffer_t(uint32_t _width, uint32_t _height)
    : width(_width),
      height(_height),
      color_buffer(color_buffer_t(_width, _height)),
      depth_buffer(depth_buffer_t(_width, _height)) {
    id = count++;
    return;
}

framebuffer_t::~framebuffer_t(void) {
    return;
}

framebuffer_t& framebuffer_t::operator=(const framebuffer_t& _framebuffer) {
    if (this == &_framebuffer) {
        throw std::runtime_error(log("this == &_framebuffer"));
    }
    if (width != _framebuffer.get_width()) {
        throw std::invalid_argument(log("width != _framebuffer.get_width()"));
    }
    if (height != _framebuffer.get_height()) {
        throw std::invalid_argument(log("height != _framebuffer.get_height()"));
    }
    color_buffer = _framebuffer.color_buffer;
    depth_buffer = _framebuffer.depth_buffer;
    return *this;
}

uint32_t framebuffer_t::get_width(void) const {
    return width;
}

uint32_t framebuffer_t::get_height(void) const {
    return height;
}

void framebuffer_t::clear(const color_t& _color, const depth_t& _depth) {
    if (std::isnan(_depth)) {
        throw std::invalid_argument(log("std::isnan(_depth)"));
    }
    (void)_color;
    (void)_depth;
    color_buffer.clear();
    depth_buffer.clear();
    return;
}

void framebuffer_t::pixel(uint32_t _x, uint32_t _y, const color_t& _color,
                          const depth_t& _depth) {
    if (_x >= width) {
        throw std::invalid_argument(log("_x >= width"));
    }
    if (_y >= height) {
        throw std::invalid_argument(log("_y >= height"));
    }
    if (std::isnan(_depth)) {
        throw std::invalid_argument(log("std::isnan(_depth)"));
    }
    /// @todo 性能瓶颈
    color_buffer(_x, _y) = _color;
    depth_buffer(_x, _y) = _depth;
    return;
}

color_buffer_t& framebuffer_t::get_color_buffer(void) {
    return color_buffer;
}

const color_buffer_t& framebuffer_t::get_color_buffer(void) const {
    return color_buffer;
}

depth_buffer_t& framebuffer_t::get_depth_buffer(void) {
    return depth_buffer;
}

framebuffer_t::depth_t&
framebuffer_t::get_depth_buffer(uint32_t _x, uint32_t _y) {
    return depth_buffer(_x, _y);
}

const depth_buffer_t& framebuffer_t::get_depth_buffer(void) const {
    return depth_buffer;
}

framebuffer_t::depth_t
framebuffer_t::get_depth_buffer(uint32_t _x, uint32_t _y) const {
    return depth_buffer(_x, _y);
}

void framebuffer_t::line(float _x0, float _y0, float _x1, float _y1,
                         const color_t& _color) {
    auto p0_x  = static_cast<int32_t>(_x0);
    auto p0_y  = static_cast<int32_t>(_y0);
    auto p1_x  = static_cast<int32_t>(_x1);
    auto p1_y  = static_cast<int32_t>(_y1);

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

    auto de  = 0;
    auto dy2 = std::abs(p1_y - p0_y) << 1;
    auto dx2 = std::abs(p1_x - p0_x) << 1;
    auto y   = p0_y;
    auto yi  = 1;
    if (p1_y <= p0_y) {
        yi = -1;
    }
    for (auto x = p0_x; x <= p1_x; x++) {
        if (steep == true) {
            /// @todo 这里要用裁剪替换掉
            if ((unsigned)y >= width || (unsigned)x >= height) {
                continue;
            }
            pixel(y, x, _color);
        }
        else {
            /// @todo 这里要用裁剪替换掉
            if ((unsigned)x >= width || (unsigned)y >= height) {
                continue;
            }
            pixel(x, y, _color);
        }
        de += std::abs(dy2);
        if (de >= dx2) {
            y  += yi;
            de -= dx2;
        }
    }

    return;
}

void framebuffer_t::triangle(const config_t&          _config,
                             const shader_base_t&     _shader,
                             const light_t&           _light,
                             const model_t::normal_t& _normal,
                             const model_t::face_t&   _face) {
    auto v0  = _face.v0;
    auto v1  = _face.v1;
    auto v2  = _face.v2;
    auto min = v0.coord.min(v1.coord).min(v2.coord);
    auto max = v0.coord.max(v1.coord).max(v2.coord);

#pragma omp parallel for num_threads(_config.procs) collapse(2) default(none) \
  shared(min, max, v0, v1, v2, _shader) firstprivate(_normal, _light)
    for (auto x = int32_t(min.x); x <= int32_t(max.x); x++) {
        for (auto y = int32_t(min.y); y <= int32_t(max.y); y++) {
            /// @todo 这里要用裁剪替换掉
            if ((unsigned)x >= width || (unsigned)y >= height) {
                continue;
            }
            auto [is_inside, barycentric_coord]
              = get_barycentric_coord(v0.coord, v1.coord, v2.coord,
                                      vector4f_t(static_cast<float>(x),
                                                 static_cast<float>(y), 0));
            // 如果点在三角形内再进行下一步
            if (is_inside == false) {
                continue;
            }
            // 计算该点的深度，通过重心坐标插值计算
            auto z = interpolate_depth(v0.coord.z, v1.coord.z, v2.coord.z,
                                       barycentric_coord);
            // 深度在已有颜色之上
            if (z < depth_buffer(x, y)) {
                continue;
            }
            // 构造着色器输入
            auto shader_fragment_in
              = shader_fragment_in_t(barycentric_coord, _normal, _light.dir,
                                     v0.color, v1.color, v2.color);
            // 计算颜色，颜色为通过 shader 片段着色器计算
            auto shader_fragment_out = _shader.fragment(shader_fragment_in);
            // 如果不需要绘制则跳过
            if (shader_fragment_out.is_need_draw == false) {
                continue;
            }
            // 构造颜色
            auto color = color_t(shader_fragment_out.color);
            // 填充像素
            pixel(x, y, color, z);
        }
    }
    return;
}

void framebuffer_t::model(const config_t& _config, const shader_base_t& _shader,
                          const light_t& _light, const model_t& _model) {
    if (_config.draw_wireframe == true) {
#pragma omp parallel for num_threads(_config.procs) default(none) \
  shared(_shader) firstprivate(_model)
        for (const auto& f : _model.get_face()) {
            /// @todo 巨大性能开销
            auto face = _shader.vertex(shader_vertex_in_t(f)).face;
            line(face.v0.coord.x, face.v0.coord.y, face.v1.coord.x,
                 face.v1.coord.y, color_t::WHITE);
            line(face.v1.coord.x, face.v1.coord.y, face.v2.coord.x,
                 face.v2.coord.y, color_t::WHITE);
            line(face.v2.coord.x, face.v2.coord.y, face.v0.coord.x,
                 face.v0.coord.y, color_t::WHITE);
        }
    }
    else {
#pragma omp parallel for num_threads(_config.procs) default(none) \
  shared(_shader) firstprivate(_model, _config, _light)
        for (const auto& f : _model.get_face()) {
            /// @todo 巨大性能开销
            auto face = _shader.vertex(shader_vertex_in_t(f)).face;
            triangle(_config, _shader, _light, face.normal, face);
        }
    }
    return;
}

void framebuffer_t::scene(const shader_base_t& _shader, const scene_t& _scene) {
    auto models = _scene.get_visible_models();
    while (models.empty() == false) {
        model(_scene.get_config(), _shader, _scene.get_light(), models.front());
        models.pop();
    }
    return;
}
