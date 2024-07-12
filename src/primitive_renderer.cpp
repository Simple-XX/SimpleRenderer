#include "primitive_renderer.hpp"
#include <algorithm>
#include <limits>
#include <tuple>

namespace simple_renderer {

PrimitiveRenderer::PrimitiveRenderer(std::shared_ptr<Framebuffer> framebuffer, DrawPixelFunc draw_pixel_func)
    : framebuffer_(framebuffer),
      depth_buffer_(std::shared_ptr<Depth[]>(new Depth[framebuffer->width() * framebuffer->height()], std::default_delete<Depth[]>())),
      draw_pixel_func_(draw_pixel_func) {
    std::fill(depth_buffer_.get(), depth_buffer_.get() + framebuffer_->width() * framebuffer_->height(), std::numeric_limits<Depth>::lowest());
}

void PrimitiveRenderer::DrawLine(float x0, float y0, float x1, float y1, const Color& color) {
    auto p0_x = static_cast<int32_t>(x0);
    auto p0_y = static_cast<int32_t>(y0);
    auto p1_x = static_cast<int32_t>(x1);
    auto p1_y = static_cast<int32_t>(y1);

    auto steep = false;
    if (std::abs(p0_x - p1_x) < std::abs(p0_y - p1_y)) {
        std::swap(p0_x, p0_y);
        std::swap(p1_x, p1_y);
        steep = true;
    }
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
            if ((unsigned)y >= framebuffer_->width() || (unsigned)x >= framebuffer_->height()) {
                continue;
            }
            draw_pixel_func_(y, x, color, framebuffer_->data());
        } else {
            if ((unsigned)x >= framebuffer_->width() || (unsigned)y >= framebuffer_->height()) {
                continue;
            }
            draw_pixel_func_(x, y, color, framebuffer_->data());
        }
        de += std::abs(dy2);
        if (de >= dx2) {
            y += yi;
            de -= dx2;
        }
    }
}

void PrimitiveRenderer::DrawTriangle(const ShaderBase &shader, const Light &light, const Model::Normal &normal, const Model::Face &face) {
    auto v0 = face.v0_;
    auto v1 = face.v1_;
    auto v2 = face.v2_;

    auto min = v0.coord_;
    auto max = v1.coord_;
    auto max_x = std::max(face.v0_.coord_.x(), std::max(face.v1_.coord_.x(), face.v2_.coord_.x()));
    auto max_y = std::max(face.v0_.coord_.y(), std::max(face.v1_.coord_.y(), face.v2_.coord_.y()));
    max.x() = max_x > max.x() ? max_x : max.x();
    max.y() = max_y > max.y() ? max_y : max.y();
    max.z() = 0;
    auto min_x = std::min(face.v0_.coord_.x(), std::min(face.v1_.coord_.x(), face.v2_.coord_.x()));
    auto min_y = std::min(face.v0_.coord_.y(), std::min(face.v1_.coord_.y(), face.v2_.coord_.y()));
    min.x() = min_x < min.x() ? min_x : min.x();
    min.y() = min_y < min.y() ? min_y : min.y();
    min.z() = 0;

#pragma omp parallel for num_threads(kNProc) collapse(2) default(none) \
    shared(min, max, v0, v1, v2, shader) firstprivate(normal, light)
    for (auto x = int32_t(min.x()); x <= int32_t(max.x()); x++) {
        for (auto y = int32_t(min.y()); y <= int32_t(max.y()); y++) {
            if ((unsigned)x >= framebuffer_->width() || (unsigned)y >= framebuffer_->height()) {
                continue;
            }
            bool is_inside;
            Vector3f barycentric_coord;
            std::tie(is_inside, barycentric_coord) = GetBarycentricCoord(v0.coord_, v1.coord_, v2.coord_, Vector3f(static_cast<float>(x), static_cast<float>(y), 0));
            if (!is_inside) {
                continue;
            }
            auto z = InterpolateDepth(v0.coord_.z(), v1.coord_.z(), v2.coord_.z(), barycentric_coord);
            if (z < depth_buffer_[y * framebuffer_->width() + x]) {
                continue;
            }
            auto shader_fragment_in = ShaderFragmentIn(barycentric_coord, normal, light.dir, v0.color_, v1.color_, v2.color_);
            auto shader_fragment_out = shader.Fragment(shader_fragment_in);
            if (!shader_fragment_out.is_need_draw_) {
                continue;
            }
            auto color = Color(shader_fragment_out.color_);
            draw_pixel_func_(x, y, color, framebuffer_->data());
            depth_buffer_[y * framebuffer_->width() + x] = z;
        }
    }
}

auto PrimitiveRenderer::GetBarycentricCoord(const Vector3f &p0, const Vector3f &p1, const Vector3f &p2, const Vector3f &pa) -> std::pair<bool, Vector3f> {
    auto p1p0 = p1 - p0;
    auto p2p0 = p2 - p0;
    auto pap0 = pa - p0;

    auto deno = (p1p0.x() * p2p0.y() - p1p0.y() * p2p0.x());
    if (std::abs(deno) < std::numeric_limits<decltype(deno)>::epsilon()) {
        return std::pair<bool, const Vector3f>{false, Vector3f()};
    }

    auto s = (p2p0.y() * pap0.x() - p2p0.x() * pap0.y()) / deno;
    if ((s > 1) || (s < 0)) {
        return std::pair<bool, const Vector3f>{false, Vector3f()};
    }

    auto t = (p1p0.x() * pap0.y() - p1p0.y() * pap0.x()) / deno;
    if ((t > 1) || (t < 0)) {
        return std::pair<bool, const Vector3f>{false, Vector3f()};
    }

    if ((1 - s - t > 1) || (1 - s - t < 0)) {
        return std::pair<bool, const Vector3f>{false, Vector3f()};
    }

    return std::pair<bool, const Vector3f>{true, Vector3f(1 - s - t, s, t)};
}

auto PrimitiveRenderer::InterpolateDepth(float depth0, float depth1, float depth2, const Vector3f &barycentric_coord) -> float {
    auto depth = depth0 * barycentric_coord.x();
    depth += depth1 * barycentric_coord.y();
    depth += depth2 * barycentric_coord.z();
    return depth;
}

} // namespace simple_renderer
