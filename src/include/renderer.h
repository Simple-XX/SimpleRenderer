
/**
 * @file simple_renderer.h
 * @brief simple_renderer 接口头文件
 * @author Zone.N (Zone.Niuzh@hotmail.com)
 * @version 1.0
 * @date 2023-10-23
 * @copyright MIT LICENSE
 * https://github.com/Simple-XX/SimpleRenderer
 * @par change log:
 * <table>
 * <tr><th>Date<th>Author<th>Description
 * <tr><td>2023-10-23<td>Zone.N<td>创建文件
 * </table>
 */

#ifndef SIMPLERENDER_SRC_INCLUDE_RENDERER_H_
#define SIMPLERENDER_SRC_INCLUDE_RENDERER_H_

#include <cstdint>
#include <functional>
#include <span>

#include "buffer.hpp"
#include "light.h"
#include "log_system.h"
#include "math.hpp"
#include "model.hpp"
#include "rasterizer.hpp"
#include "shader.hpp"

namespace simple_renderer {

// 渲染模式枚举
enum class RenderingMode {
  TRADITIONAL,  // 传统光栅化模式 - 立即深度测试
  TILE_BASED,   // Tile-based光栅化模式 - 移动GPU架构
  DEFERRED      // 延迟渲染模式 - 经典GPU管线教学模拟
};

// Face 只包含顶点索引，不包含实际的顶点数据;
// Vertex 包含3D坐标，但没有屏幕坐标
// Fragment 包含屏幕坐标，但它是光栅化的结果，不是输入
struct TriangleInfo {
  Vertex v0, v1, v2;
  const Material *material;
  size_t face_index;
  TriangleInfo(const Vertex& vertex0, const Vertex& vertex1, const Vertex& vertex2,
             const Material* mat, size_t face_idx = 0)
    : v0(vertex0), v1(vertex1), v2(vertex2), material(mat), face_index(face_idx) {}
    
  TriangleInfo() = default;
};

class SimpleRenderer {
 public:
  /**
   * 构造函数
   * @param width
   * @param height
   * @param buffer 要进行绘制的内存区域，大小为 width*height*sizeof(uint32_t)
   * @param
   */
  SimpleRenderer(size_t width, size_t height);

  /// @name 默认构造/析构函数
  /// @{
  SimpleRenderer(const SimpleRenderer &_simplerenderer) = default;
  SimpleRenderer(SimpleRenderer &&_simplerenderer) = default;
  auto operator=(const SimpleRenderer &_simplerenderer) -> SimpleRenderer & =
                                                               default;
  auto operator=(SimpleRenderer &&_simplerenderer) -> SimpleRenderer & =
                                                          default;
  virtual ~SimpleRenderer() = default;
  /// @}

  /**
   * 绘制单个模型
   * @param model 要绘制的模型
   * @param shader 用于渲染的着色器
   * @param buffer 输出缓冲区
   * @return 绘制是否成功
   */
  bool DrawModel(const Model &model, const Shader &shader, uint32_t *buffer);

  /**
   * 设置渲染模式
   * @param mode 渲染模式（传统或基于Tile）
   */
  void SetRenderingMode(RenderingMode mode);

  /**
   * 获取当前渲染模式
   * @return 当前渲染模式
   */
  RenderingMode GetRenderingMode() const;

 private:
  const size_t height_;
  const size_t width_;
  LogSystem log_system_;
  RenderingMode current_mode_;  // 当前渲染模式
  bool early_z_enabled_;        // Early-Z优化开关

  std::shared_ptr<Shader> shader_;
  std::shared_ptr<Rasterizer> rasterizer_;

  /**
   * 执行绘制管线
   * @param model 模型
   * @param buffer 输出缓冲区
   */
  void ExecuteDrawPipeline(const Model &model, uint32_t *buffer);
  

  /**
   * 传统光栅化渲染
   * @param model 模型
   * @param processedVertices 已处理的顶点
   * @param buffer 输出缓冲区
   * @return 渲染统计信息
   */
  struct RenderStats {
    double buffer_alloc_ms;
    double rasterization_ms;
    double merge_ms;
    double total_ms;
  };
  
  RenderStats ExecuteTraditionalPipeline(const Model &model, 
                                        const std::vector<Vertex> &processedVertices,
                                        uint32_t *buffer);

  /**
   * Tile-based光栅化渲染
   * @param model 模型
   * @param processedVertices 已处理的顶点
   * @param buffer 输出缓冲区
   * @return 渲染统计信息
   */
  struct TileRenderStats {
    double setup_ms;
    double binning_ms;
    double buffer_alloc_ms;
    double rasterization_ms;
    double merge_ms;
    double total_ms;
  };
  
  /**
   * 延迟渲染统计信息
   */
  struct DeferredRenderStats {
    double buffer_alloc_ms;
    double rasterization_ms;
    double fragment_collection_ms;
    double fragment_merge_ms;
    double deferred_shading_ms;
    double total_ms;
  };
  
  TileRenderStats ExecuteTileBasedPipeline(const Model &model,
                                          const std::vector<Vertex> &processedVertices,
                                          uint32_t *buffer);

  /**
   * 延迟渲染管线
   * @param model 模型
   * @param processedVertices 已处理的顶点
   * @param buffer 输出缓冲区
   * @return 渲染统计信息
   */
  DeferredRenderStats ExecuteDeferredPipeline(const Model &model,
                                             const std::vector<Vertex> &processedVertices,
                                             uint32_t *buffer);

  
private:

  void TriangleTileBinning(
    const Model &model, 
    const std::vector<Vertex> &screenVertices,
    std::vector<std::vector<TriangleInfo>> &tile_triangles,
    size_t tiles_x, size_t tiles_y, size_t tile_size);

  void RasterizeTile(
    size_t tile_id,
    const std::vector<TriangleInfo> &triangles,
    size_t tiles_x, size_t tiles_y, size_t tile_size,
    float* tile_depth_buffer, uint32_t* tile_color_buffer,
    std::unique_ptr<float[]> &global_depth_buffer,
    std::unique_ptr<uint32_t[]> &global_color_buffer,
    bool use_early_z = false,
    std::vector<Fragment>* scratch_fragments = nullptr);

  
  /**
   * 透视除法 - 将裁剪空间坐标转换为归一化设备坐标(NDC)
   * @param vertex 裁剪空间坐标的顶点
   * @return 转换后的顶点(NDC坐标)
   */
  Vertex PerspectiveDivision(const Vertex &vertex);

  /**
   * 视口变换 - 将NDC坐标转换为屏幕坐标
   * @param vertex NDC坐标的顶点
   * @return 转换后的顶点(屏幕坐标)
   */
  Vertex ViewportTransformation(const Vertex &vertex);
  
};
}  // namespace simple_renderer

#endif /* SIMPLERENDER_SRC_INCLUDE_SIMPLE_RENDER_H_ */
