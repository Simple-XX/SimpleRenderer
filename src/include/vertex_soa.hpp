// Minimal SoA layout for TBR pipeline (Phase 1)
#ifndef SIMPLERENDER_SRC_INCLUDE_VERTEX_SOA_HPP_
#define SIMPLERENDER_SRC_INCLUDE_VERTEX_SOA_HPP_

#include <vector>

#include "math.hpp"
#include "color.h"

namespace simple_renderer {

struct VertexSoA {
  // 屏幕空间坐标（视口变换后）
  std::vector<Vector4f> pos_screen;  // screen space position (x,y,z,w)
  // 裁剪空间坐标（用于视锥体剔除）：clip = MVP * pos
  std::vector<Vector4f> pos_clip;
  std::vector<Vector3f> normal;
  std::vector<Vector2f> uv;
  std::vector<Color>    color;

  inline size_t size() const { return pos_screen.size(); }
  inline void resize(size_t n) {
    pos_screen.resize(n);
    pos_clip.resize(n);
    normal.resize(n);
    uv.resize(n);
    color.resize(n);
  }
};

}  // namespace simple_renderer

#endif  // SIMPLERENDER_SRC_INCLUDE_VERTEX_SOA_HPP_
