#ifndef SIMPLERENDER_SRC_INCLUDE_RENDERERS_TILE_BASED_DEFERRED_RENDERER_HPP_
#define SIMPLERENDER_SRC_INCLUDE_RENDERERS_TILE_BASED_DEFERRED_RENDERER_HPP_

#include "renderers/renderer_base.hpp"
#include "renderers/tile_based_renderer.hpp"  // 复用 TileTriangleRef / TileGridContext 定义

namespace simple_renderer {

/**
 * @brief 基于 Tile 的延迟渲染器（Tile‑Based Deferred Renderer, TBDR）
 *
 * 设计要点：
 * - SoA 顶点布局 + 三角形分箱（binning）→ 与 TBR 一致；
 * - 以 Tile 为并行单元，避免跨 Tile 写冲突；
 * - 2‑Pass（同现代 TBDR 思路）：
 *   1) Z 预通过（深度决胜）：仅更新每像素最小深度与胜出三角形索引，并缓存透视矫正重心；
 *   2) 延迟着色：仅对胜者像素执行一次片元着色，写入 tile 局部缓冲，最后整 Tile 拷贝到全局。
 *
 * 优势：显著减少overdraw场景中的无效着色（FragmentShader 调用次数近似等于胜出像素数）。
 */
class TileBasedDeferredRenderer final : public RendererBase {
 public:
  TileBasedDeferredRenderer(size_t width, size_t height, size_t tile_size = 64)
      : RendererBase(width, height), tile_size_(tile_size) {}

  bool Render(const Model& model, const Shader& shader, uint32_t* out_color) override;

 private:
  void TriangleTileBinning(const Model& model,
                           const TileGridContext& grid,
                           std::vector<std::vector<TileTriangleRef>>& tile_triangles);

  void ProcessTriangleForTileBinning(size_t tri_idx, bool count_only,
                                     const Model& model,
                                     const TileGridContext& grid,
                                     std::vector<size_t>& tile_counts,
                                     std::vector<std::vector<TileTriangleRef>>& tile_triangles);

  void RasterizeTileDeferred(size_t tile_id,
                             const std::vector<TileTriangleRef>& triangles,
                             const TileGridContext& grid,
                             float* tile_depth_buffer, uint32_t* tile_color_buffer,
                             std::unique_ptr<float[]>& global_depth_buffer,
                             std::unique_ptr<uint32_t[]>& global_color_buffer,
                             const Shader& shader,
                             uint64_t* out_tested, uint64_t* out_covered,
                             uint64_t* out_winners, uint64_t* out_shaded);

 private:
  // 深度与颜色清除默认值（与 TBR 保持一致）
  static constexpr float kDepthClear = 1.0f;
  static constexpr uint32_t kColorClear = 0u;

  const size_t tile_size_;
};

}  // namespace simple_renderer

#endif  // SIMPLERENDER_SRC_INCLUDE_RENDERERS_TILE_BASED_DEFERRED_RENDERER_HPP_
