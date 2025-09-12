#ifndef SIMPLERENDER_SRC_INCLUDE_RENDERERS_TILE_BASED_RENDERER_HPP_
#define SIMPLERENDER_SRC_INCLUDE_RENDERERS_TILE_BASED_RENDERER_HPP_

#include "renderers/renderer_base.hpp"

namespace simple_renderer {

/**
 * @brief Tile 中的三角形轻量引用（SoA 索引 + 材质指针）
 */
struct TileTriangleRef {
  size_t i0, i1, i2;
  const Material* material = nullptr;
  size_t face_index = 0;
};

/**
 * @brief 基于 Tile 的渲染器（Tile‑Major）
 *
 * 特点：
 * - SoA 顶点布局；
 * - 三角形按 tile 分箱（binning），每 tile 内局部 Early‑Z；
 * - 单份全局 framebuffer，按 tile 覆盖范围直接拷贝回写；
 * - 通过构造参数 early_z 与 tile_size 控制行为。
 */
class TileBasedRenderer final : public RendererBase {
 public:
  /**
   * @brief 构造函数
   * @param width 画布宽度
   * @param height 画布高度
   * @param early_z 是否启用 Early‑Z（默认启用）
   * @param tile_size Tile 像素尺寸（默认 64）
   */
  TileBasedRenderer(size_t width, size_t height, bool early_z = true, size_t tile_size = 64)
      : RendererBase(width, height), early_z_(early_z), tile_size_(tile_size) {}
  /**
   * @copydoc RendererBase::Render
   */
  bool Render(const Model& model, const Shader& shader, uint32_t* out_color) override;

 private:
  /**
   * @brief 将三角形按屏幕空间包围盒映射到 tile 网格
   * @param model 模型（提供面/材质）
   * @param soa 经过变换后的 SoA 顶点数据
   * @param tile_triangles 输出：每个 tile 的三角形引用列表
   * @param tiles_x 水平 tile 数
   * @param tiles_y 垂直 tile 数
   * @param tile_size tile 像素尺寸
   */
  void TriangleTileBinning(const Model &model,
                           const VertexSoA &soa,
                           std::vector<std::vector<TileTriangleRef>> &tile_triangles,
                           size_t tiles_x, size_t tiles_y, size_t tile_size);

  /**
   * @brief 处理单个三角形的 tile binning 逻辑
   * @param tri_idx 三角形索引
   * @param count_only 是否仅进行计数（true=计数模式，false=填充模式）
   * @param model 模型数据
   * @param soa 经过变换后的 SoA 顶点数据
   * @param tiles_x 水平 tile 数
   * @param tiles_y 垂直 tile 数
   * @param tile_size tile 像素尺寸
   * @param tile_counts tile 计数数组的引用（计数模式时使用）
   * @param tile_triangles tile 三角形引用列表（填充模式时使用）
   */
  void ProcessTriangleForTileBinning(
      size_t tri_idx, bool count_only,
      const Model& model, const VertexSoA& soa,
      size_t tiles_x, size_t tiles_y, size_t tile_size,
      std::vector<size_t>& tile_counts,
      std::vector<std::vector<TileTriangleRef>>& tile_triangles);

  /**
   * @brief 光栅化单个 tile，并将结果写回全局 framebuffer
   * @param tile_id tile 序号
   * @param triangles 该 tile 覆盖的三角形引用
   * @param tiles_x 水平 tile 数
   * @param tiles_y 垂直 tile 数
   * @param tile_size tile 像素尺寸
   * @param tile_depth_buffer tile 局部深度缓冲（由调用方提供/复用）
   * @param tile_color_buffer tile 局部颜色缓冲（由调用方提供/复用）
   * @param global_depth_buffer 全局深度缓冲（单份）
   * @param global_color_buffer 全局颜色缓冲（单份）
   * @param soa 经过变换后的 SoA 顶点数据
   * @param shader 着色器
   * @param use_early_z 是否启用 Early‑Z
   * @param scratch_fragments 可复用片段临时容器
   */
  void RasterizeTile(size_t tile_id,
                      const std::vector<TileTriangleRef> &triangles,
                      size_t tiles_x, size_t tiles_y, size_t tile_size,
                      float* tile_depth_buffer, uint32_t* tile_color_buffer,
                      std::unique_ptr<float[]> &global_depth_buffer,
                      std::unique_ptr<uint32_t[]> &global_color_buffer,
                      const VertexSoA &soa,
                      const Shader& shader,
                      bool use_early_z,
                      std::vector<Fragment>* scratch_fragments);

 private:
  const bool early_z_;
  const size_t tile_size_;
};

}  // namespace simple_renderer

#endif  // SIMPLERENDER_SRC_INCLUDE_RENDERERS_TILE_BASED_RENDERER_HPP_
