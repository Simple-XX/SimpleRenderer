
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
#include <memory>
#include <string>

#include "log_system.h"
#include "model.hpp"
#include "shader.hpp"
#include "renderers/renderer_base.hpp"

namespace simple_renderer {

// 渲染模式枚举
/**
 * @brief 渲染模式
 * - PER_TRIANGLE: 逐三角形（triangle-major）前向渲染
 * - TILE_BASED: 基于 tile（tile-major）前向渲染
 * - DEFERRED: 延迟渲染（片段收集后再着色）
 */
enum class RenderingMode {
  PER_TRIANGLE,  //!< 逐三角形（triangle-major）
  TILE_BASED,    //!< 基于 tile（tile-major）
  DEFERRED       //!< 延迟渲染
};

/**
 * @brief 将渲染模式枚举转为可读字符串
 * @param mode 渲染模式
 * @return 可读字符串（PER_TRIANGLE/TILE_BASED/DEFERRED）
 */
std::string RenderingModeToString(RenderingMode mode);

/**
 * @brief 渲染门面（Facade）
 *
 * 职责：
 * - 仅作为模式选择与调用入口；
 * - 根据 `RenderingMode` 构造并持有具体渲染器；
 * - 对外暴露统一的 `DrawModel` 接口。
 */
class SimpleRenderer {
 public:
  /**
   * @brief 构造渲染器门面
   * @param width 画布宽度（像素）
   * @param height 画布高度（像素）
   */
  SimpleRenderer(size_t width, size_t height);
  ~SimpleRenderer() = default;

  /**
   * @brief 绘制单个模型
   * @param model 模型
   * @param shader 着色器（含 uniform）
   * @param buffer 输出颜色缓冲（width*height）
   * @return 是否成功
   */
  bool DrawModel(const Model &model, const Shader &shader, uint32_t *buffer);

  /**
   * @brief 设置渲染模式
   */
  void SetRenderingMode(RenderingMode mode);
  /**
   * @brief 获取当前渲染模式
   */
  RenderingMode GetRenderingMode() const;

  // 可选：配置参数（仅对 TileBasedRenderer 生效；运行中修改将重建 TBR 实例）
  /**
   * @brief 启用或禁用 Early‑Z（仅 TBR 有效）
   */
  void SetEarlyZEnabled(bool enabled);
  /**
   * @brief 设置 Tile 大小（仅 TBR 有效）
   */
  void SetTileSize(size_t tile_size);

 private:
  void EnsureRenderer();

 private:
  const size_t height_;
  const size_t width_;
  LogSystem log_system_;
  RenderingMode current_mode_;
  std::unique_ptr<RendererBase> renderer_;

  // TBR 配置缓存：在创建 TileBasedRenderer 时下发
  bool tbr_early_z_ = true;
  size_t tbr_tile_size_ = 64;
};
}  // namespace simple_renderer

#endif /* SIMPLERENDER_SRC_INCLUDE_SIMPLE_RENDER_H_ */
