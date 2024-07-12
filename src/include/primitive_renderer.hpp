
#ifndef PRIMITIVE_RENDERER_H
#define PRIMITIVE_RENDERER_H

#include <memory>
#include <functional>
#include "framebuffer.hpp"
#include "color.h"
#include "shader_base.h"
#include "light.h"
#include "model.hpp"

namespace simple_renderer {

class PrimitiveRenderer {
public:
    using DrawPixelFunc = std::function<void(size_t, size_t, uint32_t, uint32_t*)>;

    PrimitiveRenderer(std::shared_ptr<Framebuffer> framebuffer, DrawPixelFunc draw_pixel_func);

    void DrawLine(float x0, float y0, float x1, float y1, const Color& color);
    void DrawTriangle(const ShaderBase &shader, const Light &light, const Model::Normal &normal, const Model::Face &face);

private:
    using Depth = float;

    std::shared_ptr<Framebuffer> framebuffer_;
    std::shared_ptr<Depth[]> depth_buffer_;
    DrawPixelFunc draw_pixel_func_;

    static auto GetBarycentricCoord(const Vector3f &p0, const Vector3f &p1, const Vector3f &p2, const Vector3f &pa) -> std::pair<bool, Vector3f>;
    static auto InterpolateDepth(float depth0, float depth1, float depth2, const Vector3f &barycentric_coord) -> float;
};

} // namespace simple_renderer

#endif // PRIMITIVE_RENDERER_H
