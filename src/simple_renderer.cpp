#include "simple_renderer.h"
#include "default_shader.h"
#include "light.h"
#include <limits>

namespace simple_renderer {

SimpleRenderer::SimpleRenderer(std::shared_ptr<Framebuffer> framebuffer, DrawPixelFunc draw_pixel_func)
    : framebuffer_(framebuffer),
      primitive_renderer_(framebuffer, draw_pixel_func) {}

bool SimpleRenderer::render(const Model &model) {
    auto shader = DefaultShader();
    auto light = Light();
    DrawModel(shader, light, model, true, true);
    return true;
}

void SimpleRenderer::DrawModel(const ShaderBase &shader, const Light &light, const Model &model, bool draw_line, bool draw_triangle) {
    if (draw_line) {
#pragma omp parallel for num_threads(kNProc) default(none) shared(shader) firstprivate(model)
        for (const auto &f : model.GetFace()) {
            auto face = shader.Vertex(ShaderVertexIn(f)).face_;
            primitive_renderer_.DrawLine(face.v0_.coord_.x(), face.v0_.coord_.y(), face.v1_.coord_.x(), face.v1_.coord_.y(), Color::kRed);
            primitive_renderer_.DrawLine(face.v1_.coord_.x(), face.v1_.coord_.y(), face.v2_.coord_.x(), face.v2_.coord_.y(), Color::kGreen);
            primitive_renderer_.DrawLine(face.v2_.coord_.x(), face.v2_.coord_.y(), face.v0_.coord_.x(), face.v0_.coord_.y(), Color::kBlue);
        }
    }
    if (draw_triangle) {
#pragma omp parallel for num_threads(kNProc) default(none) shared(shader) firstprivate(model, light)
        for (const auto &f : model.GetFace()) {
            auto face = shader.Vertex(ShaderVertexIn(f)).face_;
            primitive_renderer_.DrawTriangle(shader, light, face.normal_, face);
        }
    }
}

} // namespace simple_renderer
