#include "renderers/renderer_base.hpp"

#include <algorithm>

namespace simple_renderer {

Vertex RendererBase::PerspectiveDivision(const Vertex &vertex) {
  Vector4f position = vertex.GetPosition();

  if (position.w <= kMinWValue) {
    Vector4f farPosition(0.0f, 0.0f, 1.0f, 1.0f);
    return Vertex(farPosition, vertex.GetNormal(), vertex.GetTexCoords(), vertex.GetColor());
  }

  float original_w = position.w;
  Vector4f ndcPosition(
      position.x / position.w,  // x_ndc = x_clip / w_clip
      position.y / position.w,  // y_ndc = y_clip / w_clip
      position.z / position.w,  // z_ndc = z_clip / w_clip
      1.0f / original_w         // 保存1/w用于透视矫正插值
  );

  ndcPosition.z = std::clamp(ndcPosition.z, -1.0f, 1.0f);
  return Vertex(ndcPosition, vertex.GetNormal(), vertex.GetTexCoords(), vertex.GetColor(), vertex.GetClipPosition());
}

Vertex RendererBase::ViewportTransformation(const Vertex &vertex) {
  Vector4f ndcPosition = vertex.GetPosition();

  // 视口变换：将NDC坐标[-1,1]转换为屏幕坐标[0,width]x[0,height]
  float screen_x = (ndcPosition.x + 1.0f) * width_ / 2.0f;
  float screen_y = (1.0f - ndcPosition.y) * height_ / 2.0f;

  Vector4f screenPosition(
      screen_x,
      screen_y,
      ndcPosition.z,
      ndcPosition.w);

  return Vertex(screenPosition, vertex.GetNormal(), vertex.GetTexCoords(), vertex.GetColor());
}

}  // namespace simple_renderer

