#ifndef SIMPLE_RENDERER_H
#define SIMPLE_RENDERER_H

#include <memory>
#include "primitive_renderer.hpp"
#include "model.hpp"

namespace simple_renderer {

class SimpleRenderer {
public:
    using DrawPixelFunc = PrimitiveRenderer::DrawPixelFunc;

    SimpleRenderer(std::shared_ptr<Framebuffer> framebuffer, DrawPixelFunc draw_pixel_func);

    bool render(const Model &model);

private:
    std::shared_ptr<Framebuffer> framebuffer_;
    PrimitiveRenderer primitive_renderer_;

    void DrawModel(const ShaderBase &shader, const Light &light, const Model &model, bool draw_line, bool draw_triangle);
};

} // namespace simple_renderer

#endif // SIMPLE_RENDERER_H
