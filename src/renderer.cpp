
/**
 * @file simple_renderer.cpp
 * @brief SimpleRenderer 实现
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

#include "renderer.h"

#include <omp.h>

#include <array>
#include <chrono>
#include <cstdint>
#include <limits>
#include <span>
#include <string_view>
#include <vector>

#include "config.h"
#include "light.h"
#include "log_system.h"
#include "model.hpp"

namespace simple_renderer {

SimpleRenderer::SimpleRenderer(size_t width, size_t height)
    : height_(height),
      width_(width),
      log_system_(LogSystem(kLogFilePath, kLogFileMaxSize, kLogFileMaxCount)),
      current_mode_(RenderingMode::TILE_BASED) {
  rasterizer_ = std::make_shared<Rasterizer>(width, height);
}

bool SimpleRenderer::DrawModel(const Model &model, const Shader &shader,
                               uint32_t *buffer) {
  SPDLOG_INFO("draw model: {}", model.GetModelPath());
  shader_ = std::make_shared<Shader>(shader);
  ExecuteDrawPipeline(model, buffer);
  return true;
}

void SimpleRenderer::SetRenderingMode(RenderingMode mode) {
  current_mode_ = mode;
  std::string mode_name;
  switch(mode) {
    case RenderingMode::TRADITIONAL:
      mode_name = "TRADITIONAL";
      break;
    case RenderingMode::TILE_BASED:
      mode_name = "TILE_BASED";
      break;
    case RenderingMode::DEFERRED:
      mode_name = "DEFERRED";
      break;
  }
  SPDLOG_INFO("rendering mode set to: {}", mode_name);
}

RenderingMode SimpleRenderer::GetRenderingMode() const {
  return current_mode_;
}

/*
Optimizes performance by performing depth testing during rasterization, keeping
only the closest fragment per pixel, and avoiding storing all
fragments—resulting in faster rendering.

通过在光栅化过程中执行深度测试，仅保留每个像素的深度值最近的片段，避免存储所有片段，从而优化性能，实现更快的渲染。
*/
void SimpleRenderer::ExecuteDrawPipeline(const Model &model, uint32_t *buffer) {
  std::string mode_name;
  switch(current_mode_) {
    case RenderingMode::TRADITIONAL:
      mode_name = "TRADITIONAL";
      break;
    case RenderingMode::TILE_BASED:
      mode_name = "TILE_BASED";
      break;
    case RenderingMode::DEFERRED:
      mode_name = "DEFERRED";
      break;
  }
  SPDLOG_INFO("execute draw pipeline for {} using {} mode", model.GetModelPath(), mode_name);
  
  if (!shader_) {
    SPDLOG_ERROR("No shader set for DrawModel, cannot render");
    return;
  }
  
  // === PERFORMANCE TIMING ===
  auto total_start_time = std::chrono::high_resolution_clock::now();

  /* * * Vertex Shader * * */
  auto vertex_shader_start_time = std::chrono::high_resolution_clock::now();
  std::vector<Vertex> processedVertices;
  std::vector<std::vector<Vertex>> processed_vertices_all_thread(kNProc);
#pragma omp parallel num_threads(kNProc) default(none) \
    shared(shader_, processed_vertices_all_thread) firstprivate(model)
  {
    int thread_id = omp_get_thread_num();
    std::vector<Vertex> &processedVertices_per_thread =
        processed_vertices_all_thread[thread_id];

#pragma omp for
    for (const auto &v : model.GetVertices()) {
      // 顶点着色器：世界坐标 -> 裁剪坐标
      auto clipSpaceVertex = shader_->VertexShader(v);
      
      // 透视除法：裁剪坐标 -> NDC坐标
      auto ndcVertex = PerspectiveDivision(clipSpaceVertex);
      
      // 视口变换：NDC坐标 -> 屏幕坐标
      auto screenSpaceVertex = ViewportTransformation(ndcVertex);
      
      processedVertices_per_thread.push_back(screenSpaceVertex);
    }
  }

  for (const auto &processedVertices_per_thread :
       processed_vertices_all_thread) {
    processedVertices.insert(processedVertices.end(),
                             processedVertices_per_thread.begin(),
                             processedVertices_per_thread.end());
  }
  auto vertex_shader_end_time = std::chrono::high_resolution_clock::now();
  auto vertex_shader_duration = std::chrono::duration_cast<std::chrono::microseconds>(
      vertex_shader_end_time - vertex_shader_start_time);

  // 根据当前设置的模式选择不同的渲染管线
  double vertex_ms = vertex_shader_duration.count() / 1000.0;
  
  switch (current_mode_) {
    case RenderingMode::TRADITIONAL: {
      auto stats = ExecuteTraditionalPipeline(model, processedVertices, buffer);
      double total_ms = vertex_ms + stats.total_ms;
      
      SPDLOG_INFO("=== TRADITIONAL RENDERING PERFORMANCE ===");
      SPDLOG_INFO("Vertex Shader:    {:8.3f} ms ({:5.1f}%)", vertex_ms, vertex_ms/total_ms*100);
      SPDLOG_INFO("Buffer Alloc:     {:8.3f} ms ({:5.1f}%)", stats.buffer_alloc_ms, stats.buffer_alloc_ms/total_ms*100);
      SPDLOG_INFO("Rasterization:    {:8.3f} ms ({:5.1f}%)", stats.rasterization_ms, stats.rasterization_ms/total_ms*100);
      SPDLOG_INFO("Merge:            {:8.3f} ms ({:5.1f}%)", stats.merge_ms, stats.merge_ms/total_ms*100);
      SPDLOG_INFO("Total:            {:8.3f} ms", total_ms);
      SPDLOG_INFO("==========================================");
      break;
    }
    
    case RenderingMode::TILE_BASED: {
      auto stats = ExecuteTileBasedPipeline(model, processedVertices, buffer);
      double total_ms = vertex_ms + stats.total_ms;
      
      SPDLOG_INFO("=== TILE-BASED RENDERING PERFORMANCE ===");
      SPDLOG_INFO("Vertex Shader:    {:8.3f} ms ({:5.1f}%)", vertex_ms, vertex_ms/total_ms*100);
      SPDLOG_INFO("Setup:            {:8.3f} ms ({:5.1f}%)", stats.setup_ms, stats.setup_ms/total_ms*100);
      SPDLOG_INFO("Binning:          {:8.3f} ms ({:5.1f}%)", stats.binning_ms, stats.binning_ms/total_ms*100);
      SPDLOG_INFO("Buffer Alloc:     {:8.3f} ms ({:5.1f}%)", stats.buffer_alloc_ms, stats.buffer_alloc_ms/total_ms*100);
      SPDLOG_INFO("Rasterization:    {:8.3f} ms ({:5.1f}%)", stats.rasterization_ms, stats.rasterization_ms/total_ms*100);
      SPDLOG_INFO("Merge:            {:8.3f} ms ({:5.1f}%)", stats.merge_ms, stats.merge_ms/total_ms*100);
      SPDLOG_INFO("Visualization:    {:8.3f} ms ({:5.1f}%)", stats.visualization_ms, stats.visualization_ms/total_ms*100);
      SPDLOG_INFO("Total:            {:8.3f} ms", total_ms);
      SPDLOG_INFO("==========================================");
      break;
    }
    
    case RenderingMode::DEFERRED: {
      auto stats = ExecuteDeferredPipeline(model, processedVertices, buffer);
      double total_ms = vertex_ms + stats.total_ms;
      
      SPDLOG_INFO("=== DEFERRED RENDERING PERFORMANCE ===");
      SPDLOG_INFO("Vertex Shader:        {:8.3f} ms ({:5.1f}%)", vertex_ms, vertex_ms/total_ms*100);
      SPDLOG_INFO("Buffer Alloc:         {:8.3f} ms ({:5.1f}%)", stats.buffer_alloc_ms, stats.buffer_alloc_ms/total_ms*100);
      SPDLOG_INFO("Rasterization:        {:8.3f} ms ({:5.1f}%)", stats.rasterization_ms, stats.rasterization_ms/total_ms*100);
      SPDLOG_INFO("Fragment Collection:  {:8.3f} ms ({:5.1f}%)", stats.fragment_collection_ms, stats.fragment_collection_ms/total_ms*100);
      SPDLOG_INFO("Fragment Merge:       {:8.3f} ms ({:5.1f}%)", stats.fragment_merge_ms, stats.fragment_merge_ms/total_ms*100);
      SPDLOG_INFO("Deferred Shading:     {:8.3f} ms ({:5.1f}%)", stats.deferred_shading_ms, stats.deferred_shading_ms/total_ms*100);
      SPDLOG_INFO("Total:                {:8.3f} ms", total_ms);
      SPDLOG_INFO("=========================================");
      break;
    }
  }
}


/*
Organizes processing to simulate how OpenGL works with GPUs by collecting all
fragments per pixel before processing, closely mimicking the GPU pipeline but
leading to increased memory usage and slower performance.

组织处理方式模拟 OpenGL 在 GPU
上的工作原理，先收集每个像素的所有片段再并行处理屏幕上的每个像素，模仿 GPU
管线，但导致内存使用增加和渲染速度变慢。

现在作为延迟渲染管线的一部分，用于教学演示经典GPU管线概念。
*/
SimpleRenderer::DeferredRenderStats SimpleRenderer::ExecuteDeferredPipeline(
    const Model &model,
    const std::vector<Vertex> &processedVertices,
    uint32_t *buffer) {
    
  DeferredRenderStats stats;
  auto total_start_time = std::chrono::high_resolution_clock::now();
  SPDLOG_INFO("execute deferred pipeline for {}", model.GetModelPath());
  /*  *  *  *  *  *  *  */

  /* * * Buffer Allocation * * */
  auto buffer_alloc_start_time = std::chrono::high_resolution_clock::now();
  std::vector<std::vector<std::vector<Fragment>>> fragmentsBuffer_all_thread(
      kNProc, std::vector<std::vector<Fragment>>(width_ * height_));

  // 预先缓存所有Material数据，避免指针悬垂问题
  std::vector<Material> material_cache;
  material_cache.reserve(model.GetFaces().size());
  for (const auto &f : model.GetFaces()) {
    material_cache.push_back(f.GetMaterial()); // 值拷贝
  }
  auto buffer_alloc_end_time = std::chrono::high_resolution_clock::now();
  auto buffer_alloc_duration = std::chrono::duration_cast<std::chrono::microseconds>(
      buffer_alloc_end_time - buffer_alloc_start_time);
  SPDLOG_INFO("cached {} materials for deferred rendering", material_cache.size());

  /* * * Rasterization * * */
  auto rasterization_start_time = std::chrono::high_resolution_clock::now();
#pragma omp parallel num_threads(kNProc) default(none)                       \
    shared(processedVertices, fragmentsBuffer_all_thread, rasterizer_, width_, \
               height_, material_cache) firstprivate(model)
  {
    int thread_id = omp_get_thread_num();
    auto &fragmentsBuffer_per_thread = fragmentsBuffer_all_thread[thread_id];

#pragma omp for
    for (size_t face_idx = 0; face_idx < model.GetFaces().size(); ++face_idx) {
      const auto &f = model.GetFaces()[face_idx];
      auto v0 = processedVertices[f.GetIndex(0)];
      auto v1 = processedVertices[f.GetIndex(1)];
      auto v2 = processedVertices[f.GetIndex(2)];

      const Material *material = &material_cache[face_idx]; // 使用缓存的Material

      auto fragments = rasterizer_->Rasterize(v0, v1, v2);

      for (auto &fragment : fragments) {
        fragment.material = material;

        size_t x = fragment.screen_coord[0];
        size_t y = fragment.screen_coord[1];

        if (x >= width_ || y >= height_) {
          continue;
        }

        size_t index = x + y * width_;
        fragmentsBuffer_per_thread[index].push_back(fragment);
      }
    }
  }
  auto rasterization_end_time = std::chrono::high_resolution_clock::now();
  auto rasterization_duration = std::chrono::duration_cast<std::chrono::microseconds>(
      rasterization_end_time - rasterization_start_time);
  /*  *  *  *  *  *  *  */

  /* * * Fragment Collection * * */
  auto fragment_collection_start_time = std::chrono::high_resolution_clock::now();
  std::vector<std::vector<Fragment>> fragmentsBuffer(width_ * height_);
  for (const auto &fragmentsBuffer_per_thread : fragmentsBuffer_all_thread) {
    for (size_t i = 0; i < fragmentsBuffer_per_thread.size(); i++) {
      fragmentsBuffer[i].insert(fragmentsBuffer[i].end(),
                                fragmentsBuffer_per_thread[i].begin(),
                                fragmentsBuffer_per_thread[i].end());
    }
  }
  auto fragment_collection_end_time = std::chrono::high_resolution_clock::now();
  auto fragment_collection_duration = std::chrono::duration_cast<std::chrono::microseconds>(
      fragment_collection_end_time - fragment_collection_start_time);
  /*  *  *  *  *  *  *  */

  /* * * Fragment Merge & Deferred Shading * * */
  auto fragment_merge_start_time = std::chrono::high_resolution_clock::now();
  
  // Fragment Merge阶段：深度测试选择最近片段
  std::vector<const Fragment*> selected_fragments(width_ * height_, nullptr);
  #pragma omp parallel for
  for (size_t i = 0; i < fragmentsBuffer.size(); i++) {
    const auto &fragments = fragmentsBuffer[i];
    if (fragments.empty()) {
      continue;
    }

    const Fragment *renderFragment = nullptr;
    for (const auto &fragment : fragments) {
      if (!renderFragment || fragment.depth < renderFragment->depth) {
        renderFragment = &fragment;
      }
    }
    selected_fragments[i] = renderFragment;
  }
  auto fragment_merge_end_time = std::chrono::high_resolution_clock::now();
  auto fragment_merge_duration = std::chrono::duration_cast<std::chrono::microseconds>(
      fragment_merge_end_time - fragment_merge_start_time);
  
  // Deferred Shading阶段：执行片段着色器
  auto deferred_shading_start_time = std::chrono::high_resolution_clock::now();
#pragma omp parallel for
  for (size_t i = 0; i < selected_fragments.size(); i++) {
    const Fragment *renderFragment = selected_fragments[i];
    if (renderFragment) {
      // 添加Material指针有效性检查
      if (renderFragment->material == nullptr) {
        SPDLOG_ERROR("Fragment material is nullptr at pixel {}", i);
        continue;
      }
      auto color = shader_->FragmentShader(*renderFragment);
      buffer[i] = uint32_t(color);
    }
  }
  auto deferred_shading_end_time = std::chrono::high_resolution_clock::now();
  auto deferred_shading_duration = std::chrono::duration_cast<std::chrono::microseconds>(
      deferred_shading_end_time - deferred_shading_start_time);
  /*  *  *  *  *  *  *  */
  
  auto total_end_time = std::chrono::high_resolution_clock::now();
  auto total_duration = std::chrono::duration_cast<std::chrono::microseconds>(
      total_end_time - total_start_time);
  
  // 填充统计信息
  stats.buffer_alloc_ms = buffer_alloc_duration.count() / 1000.0;
  stats.rasterization_ms = rasterization_duration.count() / 1000.0;
  stats.fragment_collection_ms = fragment_collection_duration.count() / 1000.0;
  stats.fragment_merge_ms = fragment_merge_duration.count() / 1000.0;
  stats.deferred_shading_ms = deferred_shading_duration.count() / 1000.0;
  stats.total_ms = total_duration.count() / 1000.0;
  
  return stats;
}

Vertex SimpleRenderer::PerspectiveDivision(const Vertex &vertex) {
  Vector4f position = vertex.GetPosition();
  
  // 检查w分量，避免除零和负数问题
  if (position.w <= 1e-6f) {
    Vector4f farPosition(0.0f, 0.0f, 1.0f, 1.0f);
    return Vertex(farPosition, vertex.GetNormal(), vertex.GetTexCoords(), vertex.GetColor());
  }
  
  // 保存原始w分量用于透视矫正插值
  float original_w = position.w;
  
  // 执行透视除法：(x, y, z, w) -> (x/w, y/w, z/w, 1/w)
  Vector4f ndcPosition(
    position.x / position.w,  // x_ndc = x_clip / w_clip
    position.y / position.w,  // y_ndc = y_clip / w_clip  
    position.z / position.w,  // z_ndc = z_clip / w_clip
    1.0f / original_w         // 保存1/w用于透视矫正插值
  );
  
  // 只对Z坐标进行深度范围限制，X和Y允许超出以支持屏幕外三角形
  // 这些坐标在后续的视口变换和裁剪阶段会被正确处理
  ndcPosition.z = std::clamp(ndcPosition.z, -1.0f, 1.0f);
  
  // 创建新的顶点，保持其他属性和裁剪空间坐标不变
  if (vertex.HasClipPosition()) {
    return Vertex(ndcPosition, vertex.GetNormal(), vertex.GetTexCoords(), vertex.GetColor(), vertex.GetClipPosition());
  } else {
    return Vertex(ndcPosition, vertex.GetNormal(), vertex.GetTexCoords(), vertex.GetColor());
  }
}

Vertex SimpleRenderer::ViewportTransformation(const Vertex &vertex) {
  Vector4f ndcPosition = vertex.GetPosition();
  
  // 视口变换：将NDC坐标[-1,1]转换为屏幕坐标[0,width]x[0,height]
  float screen_x = (ndcPosition.x + 1.0f) * width_ / 2.0f;
  float screen_y = (1.0f - ndcPosition.y) * height_ / 2.0f;
  
  Vector4f screenPosition(
    screen_x,                    // x: 屏幕坐标
    screen_y,                    // y: 屏幕坐标
    ndcPosition.z,               // z: NDC坐标用于深度测试
    ndcPosition.w                // w: 保持1/w用于透视矫正插值
  );
  
  return Vertex(screenPosition, vertex.GetNormal(), vertex.GetTexCoords(), vertex.GetColor());
}




// Triangle-Tile binning函数 - 修正版本
void SimpleRenderer::TriangleTileBinning(
    const Model &model, 
    const std::vector<Vertex> &screenVertices,
    std::vector<std::vector<TriangleInfo>> &tile_triangles,
    size_t tiles_x, size_t tiles_y, size_t tile_size) {
    
    size_t total_triangles = model.GetFaces().size();
    size_t processed_triangles = 0;
    size_t clipped_triangles = 0;
    size_t triangles_with_clipped_vertices = 0;
    
    // 裁剪统计
    size_t frustum_culled = 0;
    size_t backface_culled = 0;
    
    SPDLOG_INFO("Starting triangle-tile binning for {} triangles", total_triangles);
    SPDLOG_INFO("Screen dimensions: {}x{}, Tile size: {}, Tiles: {}x{}", 
                width_, height_, tile_size, tiles_x, tiles_y);
    
    for (size_t tri_idx = 0; tri_idx < model.GetFaces().size(); tri_idx++) {
        const auto &f = model.GetFaces()[tri_idx];
        auto v0 = screenVertices[f.GetIndex(0)];
        auto v1 = screenVertices[f.GetIndex(1)];
        auto v2 = screenVertices[f.GetIndex(2)];
        
        // 视锥体裁剪 (裁剪空间)
        if (v0.HasClipPosition()) {
            Vector4f c0 = v0.GetClipPosition();
            Vector4f c1 = v1.GetClipPosition(); 
            Vector4f c2 = v2.GetClipPosition();
            
            // 保守视锥体裁剪：只有当整个三角形都在视锥体外同一侧时才裁剪
            bool frustum_cull = 
                (c0.x > c0.w && c1.x > c1.w && c2.x > c2.w) ||  // 右平面外
                (c0.x < -c0.w && c1.x < -c1.w && c2.x < -c2.w) || // 左平面外  
                (c0.y > c0.w && c1.y > c1.w && c2.y > c2.w) ||  // 上平面外
                (c0.y < -c0.w && c1.y < -c1.w && c2.y < -c2.w) || // 下平面外
                (c0.z > c0.w && c1.z > c1.w && c2.z > c2.w) ||  // 远平面外
                (c0.z < -c0.w && c1.z < -c1.w && c2.z < -c2.w);  // 近平面外
                
            // if (frustum_cull) {
            //     frustum_culled++;
            //     continue;
            // }
        }
        
        // 获取屏幕空间坐标（现在已经是屏幕坐标了）
        Vector4f pos0 = v0.GetPosition();
        Vector4f pos1 = v1.GetPosition();
        Vector4f pos2 = v2.GetPosition();
        
        // 背面剔除 (屏幕空间)
        Vector2f screen0(pos0.x, pos0.y);
        Vector2f screen1(pos1.x, pos1.y);  
        Vector2f screen2(pos2.x, pos2.y);
        
        // 计算屏幕空间叉积判断朝向
        Vector2f edge1 = screen1 - screen0;
        Vector2f edge2 = screen2 - screen0;
        float cross_product = edge1.x * edge2.y - edge1.y * edge2.x;
        
        // 背面剔除：NDC空间中叉积为负表示顺时针，即背面。
        // 从NDC到屏幕空间中，会发生Y轴翻转，对应叉积应为正。
        if (cross_product > 0.0f) {
            backface_culled++;
            continue;
        }
        
        // 检查三角形是否有被裁剪的顶点（坐标为-1000的表示被裁剪）
        bool has_clipped_vertex = (pos0.x == -1000.0f || pos1.x == -1000.0f || pos2.x == -1000.0f);
        
        if (has_clipped_vertex) {
            triangles_with_clipped_vertices++;
            if (triangles_with_clipped_vertices <= 3) {
                SPDLOG_INFO("Triangle {} has clipped vertices:", tri_idx);
                SPDLOG_INFO("  V0: ({:.1f},{:.1f}) V1: ({:.1f},{:.1f}) V2: ({:.1f},{:.1f})", 
                           pos0.x, pos0.y, pos1.x, pos1.y, pos2.x, pos2.y);
            }
            continue;
        }
        
        // 直接使用屏幕空间坐标
        float screen_x0 = pos0.x;
        float screen_y0 = pos0.y;
        float screen_x1 = pos1.x;
        float screen_y1 = pos1.y;
        float screen_x2 = pos2.x;
        float screen_y2 = pos2.y;
        
        // 计算bounding box
        float min_x = std::min({screen_x0, screen_x1, screen_x2});
        float max_x = std::max({screen_x0, screen_x1, screen_x2});
        float min_y = std::min({screen_y0, screen_y1, screen_y2});
        float max_y = std::max({screen_y0, screen_y1, screen_y2});
        
        // 调试前几个有效三角形的坐标范围
        if (processed_triangles < 3) {
            SPDLOG_INFO("Triangle {} coordinates:", tri_idx);
            SPDLOG_INFO("  Screen coords: ({:.1f},{:.1f}) ({:.1f},{:.1f}) ({:.1f},{:.1f})", 
                       screen_x0, screen_y0, screen_x1, screen_y1, screen_x2, screen_y2);
            SPDLOG_INFO("  BBox: min({:.1f},{:.1f}) max({:.1f},{:.1f})", 
                       min_x, min_y, max_x, max_y);
        }
        
        // 临时：大幅放宽屏幕边界检查，让超出屏幕的三角形也能处理
        if (max_x < -5000.0f || min_x >= width_ + 5000.0f || 
            max_y < -5000.0f || min_y >= height_ + 5000.0f) {
            clipped_triangles++;
            if (processed_triangles < 3) {
                SPDLOG_INFO("  -> CLIPPED by screen bounds");
            }
            continue;
        }
        
        // 计算影响的tile范围
        int start_tile_x = std::max(0, static_cast<int>(min_x) / static_cast<int>(tile_size));
        int end_tile_x = std::min(static_cast<int>(tiles_x - 1), 
                                 static_cast<int>(max_x) / static_cast<int>(tile_size));
        int start_tile_y = std::max(0, static_cast<int>(min_y) / static_cast<int>(tile_size));
        int end_tile_y = std::min(static_cast<int>(tiles_y - 1), 
                                 static_cast<int>(max_y) / static_cast<int>(tile_size));
        
        // 添加三角形到相关tiles（多个三角形可能会映射到同一个tile当中，所以谨慎并行化）
        if (start_tile_x <= end_tile_x && start_tile_y <= end_tile_y) {
            TriangleInfo triangle_info = {v0, v1, v2, &f.GetMaterial(), processed_triangles};
            
            for (int ty = start_tile_y; ty <= end_tile_y; ty++) {
                for (int tx = start_tile_x; tx <= end_tile_x; tx++) {
                    size_t tile_id = ty * tiles_x + tx;
                    tile_triangles[tile_id].push_back(triangle_info); // 可能多个线程同时pushback的话有风险
                }
            }
            processed_triangles++;
            
            // 输出前几个成功添加的三角形信息
            if (processed_triangles <= 3) {
                SPDLOG_INFO("  -> SUCCESSFULLY ADDED to tiles x[{}..{}] y[{}..{}]", 
                           start_tile_x, end_tile_x, start_tile_y, end_tile_y);
            }
        } else {
            if (processed_triangles < 3) {
                SPDLOG_INFO("  -> FAILED tile calculation: x[{}..{}] y[{}..{}]", 
                           start_tile_x, end_tile_x, start_tile_y, end_tile_y);
            }
        }
    }
    
    // 输出统计信息
    SPDLOG_INFO("Triangle-Tile binning completed:");
    SPDLOG_INFO("  Total triangles: {}", total_triangles);
    SPDLOG_INFO("  Triangles with clipped vertices: {}", triangles_with_clipped_vertices);
    SPDLOG_INFO("  Processed triangles: {}", processed_triangles);
    SPDLOG_INFO("  Clipped by screen bounds: {}", clipped_triangles);
    SPDLOG_INFO("  Frustum culled: {}", frustum_culled);
    SPDLOG_INFO("  Backface culled: {}", backface_culled);
    
    size_t total_triangle_refs = 0;
    size_t non_empty_tiles = 0;
    for (const auto& tile : tile_triangles) {
        total_triangle_refs += tile.size();
        if (!tile.empty()) non_empty_tiles++;
    }
    
    SPDLOG_INFO("  Total triangle references: {}", total_triangle_refs);
    SPDLOG_INFO("  Non-empty tiles: {}", non_empty_tiles);
    SPDLOG_INFO("  Average triangles per tile: {:.2f}", 
                total_triangle_refs > 0 ? float(total_triangle_refs) / tile_triangles.size() : 0.0f);
}

// 单个tile光栅化函数
void SimpleRenderer::RasterizeTile(
    size_t tile_id,
    const std::vector<TriangleInfo> &triangles,
    size_t tiles_x, size_t tiles_y, size_t tile_size,
    float* tile_depth_buffer, uint32_t* tile_color_buffer,
    std::unique_ptr<float[]> &global_depth_buffer,
    std::unique_ptr<uint32_t[]> &global_color_buffer) {
  // 计算tile在屏幕空间的范围
  size_t tile_x = tile_id % tiles_x;
  size_t tile_y = tile_id / tiles_x;
  size_t screen_x_start = tile_x * tile_size;
  size_t screen_y_start = tile_y * tile_size;
  size_t screen_x_end = std::min(screen_x_start + tile_size, width_);
  size_t screen_y_end = std::min(screen_y_start + tile_size, height_);
    
  // 初始化tile缓冲区
  size_t tile_width = screen_x_end - screen_x_start;
  size_t tile_height = screen_y_end - screen_y_start;
  std::fill_n(tile_depth_buffer, tile_width * tile_height,
              std::numeric_limits<float>::infinity());
  std::fill_n(tile_color_buffer, tile_width * tile_height, 0);
    
  // 在tile内光栅化所有三角形
  for (const auto &triangle : triangles) {
    auto fragments = rasterizer_->Rasterize(triangle.v0, triangle.v1, triangle.v2);
        
    for (auto &fragment : fragments) {
      fragment.material = triangle.material;
            
      size_t screen_x = fragment.screen_coord[0];
      size_t screen_y = fragment.screen_coord[1];
            
      // 检查fragment是否在当前tile内
      if (screen_x >= screen_x_start && screen_x < screen_x_end &&
          screen_y >= screen_y_start && screen_y < screen_y_end) {
                
        size_t tile_local_x = screen_x - screen_x_start;
        size_t tile_local_y = screen_y - screen_y_start;
        size_t tile_index = tile_local_x + tile_local_y * tile_width;
                
        // tile内深度测试
        if (fragment.depth < tile_depth_buffer[tile_index]) {
          tile_depth_buffer[tile_index] = fragment.depth;
                    
          auto color = shader_->FragmentShader(fragment);
          tile_color_buffer[tile_index] = uint32_t(color);
        }
          }
    }
  }
    
  // 将tile结果写入全局缓冲区
  for (size_t y = 0; y < tile_height; y++) {
    for (size_t x = 0; x < tile_width; x++) {
      size_t tile_index = x + y * tile_width;
      size_t global_index = (screen_x_start + x) + (screen_y_start + y) * width_;
            
      if (tile_depth_buffer[tile_index] < global_depth_buffer[global_index]) {
        global_depth_buffer[global_index] = tile_depth_buffer[tile_index];
        global_color_buffer[global_index] = tile_color_buffer[tile_index];
      }
    }
  }
}

// Tile可视化调试函数，这里用于固定大小的tiles
void SimpleRenderer::DrawTileVisualization(uint32_t* buffer, 
    const std::vector<std::vector<TriangleInfo>>& tile_triangles, 
    size_t tiles_x, size_t tiles_y, size_t tile_size) {
    
    SPDLOG_INFO("=== TILE VISUALIZATION DEBUG ===");
    SPDLOG_INFO("Drawing tile grid overlay for debugging");
    
    // 颜色定义 (ABGR格式)
    const uint32_t GRID_COLOR = 0xFF00FF00;      // 绿色网格线
    const uint32_t NONEMPTY_COLOR = 0x4000FFFF;  // 半透明黄色背景 (非空tile)
    const uint32_t EMPTY_COLOR = 0x20FF0000;     // 半透明蓝色背景 (空tile)
    
    // 1. 为非空tiles添加背景色
    for (size_t tile_y = 0; tile_y < tiles_y; tile_y++) {
        for (size_t tile_x = 0; tile_x < tiles_x; tile_x++) {
            size_t tile_id = tile_y * tiles_x + tile_x;
            bool is_empty = tile_triangles[tile_id].empty();
            
            // 计算tile在屏幕上的像素范围
            size_t pixel_start_x = tile_x * tile_size;
            size_t pixel_end_x = std::min(pixel_start_x + tile_size, static_cast<size_t>(width_));
            size_t pixel_start_y = tile_y * tile_size;
            size_t pixel_end_y = std::min(pixel_start_y + tile_size, static_cast<size_t>(height_));
            
            uint32_t bg_color = is_empty ? EMPTY_COLOR : NONEMPTY_COLOR;
            
            // 给tile添加半透明背景
            for (size_t y = pixel_start_y; y < pixel_end_y; y++) {
                for (size_t x = pixel_start_x; x < pixel_end_x; x++) {
                    size_t pixel_idx = y * static_cast<size_t>(width_) + x;
                    // 简单的alpha混合：将背景色与原色混合
                    uint32_t original = buffer[pixel_idx];
                    buffer[pixel_idx] = BlendColors(original, bg_color);
                }
            }
            
            // 记录非空tile的信息
            if (!is_empty) {
                SPDLOG_INFO("Non-empty Tile[{},{}] (ID:{}): {} triangles", 
                           tile_x, tile_y, tile_id, tile_triangles[tile_id].size());
            }
        }
    }
    
    // 2. 绘制网格线
    // 垂直线
    for (size_t tile_x = 0; tile_x <= tiles_x; tile_x++) {
        size_t pixel_x = tile_x * tile_size;
        if (pixel_x < static_cast<size_t>(width_)) {
            for (size_t y = 0; y < static_cast<size_t>(height_); y++) {
                buffer[y * static_cast<size_t>(width_) + pixel_x] = GRID_COLOR;
            }
        }
    }
    
    // 水平线
    for (size_t tile_y = 0; tile_y <= tiles_y; tile_y++) {
        size_t pixel_y = tile_y * tile_size;
        if (pixel_y < static_cast<size_t>(height_)) {
            for (size_t x = 0; x < static_cast<size_t>(width_); x++) {
                buffer[pixel_y * static_cast<size_t>(width_) + x] = GRID_COLOR;
            }
        }
    }
    
    SPDLOG_INFO("Tile visualization completed - Green:Grid, Yellow:NonEmpty, Blue:Empty");
    SPDLOG_INFO("=====================================");
}

// 简单的颜色混合函数 (alpha blending)
uint32_t SimpleRenderer::BlendColors(uint32_t base, uint32_t overlay) {
    // 提取RGBA通道 (假设是ABGR格式)
    uint8_t base_r = (base >> 16) & 0xFF;
    uint8_t base_g = (base >> 8) & 0xFF;
    uint8_t base_b = base & 0xFF;
    
    uint8_t overlay_r = (overlay >> 16) & 0xFF;
    uint8_t overlay_g = (overlay >> 8) & 0xFF;
    uint8_t overlay_b = overlay & 0xFF;
    uint8_t overlay_a = (overlay >> 24) & 0xFF;
    
    // 简单的alpha混合
    float alpha = overlay_a / 255.0f;
    uint8_t result_r = (uint8_t)(base_r * (1.0f - alpha) + overlay_r * alpha);
    uint8_t result_g = (uint8_t)(base_g * (1.0f - alpha) + overlay_g * alpha);
    uint8_t result_b = (uint8_t)(base_b * (1.0f - alpha) + overlay_b * alpha);
    
    return 0xFF000000 | (result_r << 16) | (result_g << 8) | result_b;
}

// 传统光栅化管线实现
SimpleRenderer::RenderStats SimpleRenderer::ExecuteTraditionalPipeline(
    const Model &model, 
    const std::vector<Vertex> &processedVertices,
    uint32_t *buffer) {
    
    RenderStats stats;
    auto total_start_time = std::chrono::high_resolution_clock::now();
    
    // 1. 为每个线程创建framebuffer
    auto buffer_alloc_start_time = std::chrono::high_resolution_clock::now();
    std::vector<std::unique_ptr<float[]>> depthBuffer_all_thread(kNProc);
    std::vector<std::unique_ptr<uint32_t[]>> colorBuffer_all_thread(kNProc);
    
    for (size_t thread_id = 0; thread_id < kNProc; thread_id++) {
        depthBuffer_all_thread[thread_id] = 
            std::make_unique<float[]>(width_ * height_);
        colorBuffer_all_thread[thread_id] = 
            std::make_unique<uint32_t[]>(width_ * height_);
        
        std::fill_n(depthBuffer_all_thread[thread_id].get(), width_ * height_,
                    std::numeric_limits<float>::infinity());
        std::fill_n(colorBuffer_all_thread[thread_id].get(), width_ * height_, 0);
    }
    auto buffer_alloc_end_time = std::chrono::high_resolution_clock::now();
    auto buffer_alloc_duration = std::chrono::duration_cast<std::chrono::microseconds>(
        buffer_alloc_end_time - buffer_alloc_start_time);
    
    // 2. 并行光栅化
    auto raster_start_time = std::chrono::high_resolution_clock::now();
#pragma omp parallel num_threads(kNProc) default(none) \
    shared(processedVertices, rasterizer_, shader_, width_, height_, \
           depthBuffer_all_thread, colorBuffer_all_thread) \
    firstprivate(model)
    {
        int thread_id = omp_get_thread_num();
        auto &depthBuffer_per_thread = depthBuffer_all_thread[thread_id];
        auto &colorBuffer_per_thread = colorBuffer_all_thread[thread_id];
        
#pragma omp for
        for (const auto &f : model.GetFaces()) {
            auto v0 = processedVertices[f.GetIndex(0)];
            auto v1 = processedVertices[f.GetIndex(1)];
            auto v2 = processedVertices[f.GetIndex(2)];

            const Material *material = &f.GetMaterial();
            auto fragments = rasterizer_->Rasterize(v0, v1, v2);

            for (auto &fragment : fragments) {
                fragment.material = material;
                size_t x = fragment.screen_coord[0];
                size_t y = fragment.screen_coord[1];

                if (x >= width_ || y >= height_) {
                    continue;
                }

                size_t index = x + y * width_;
                if (fragment.depth < depthBuffer_per_thread[index]) {
                    depthBuffer_per_thread[index] = fragment.depth;
                    auto color = shader_->FragmentShader(fragment);
                    colorBuffer_per_thread[index] = uint32_t(color);
                }
            }
        }
    }
    auto raster_end_time = std::chrono::high_resolution_clock::now();
    auto raster_duration = std::chrono::duration_cast<std::chrono::microseconds>(
        raster_end_time - raster_start_time);
    
    // 3. 合并结果
    auto merge_start_time = std::chrono::high_resolution_clock::now();
    std::unique_ptr<float[]> depthBuffer = 
        std::make_unique<float[]>(width_ * height_);
    std::unique_ptr<uint32_t[]> colorBuffer = 
        std::make_unique<uint32_t[]>(width_ * height_);

    std::fill_n(depthBuffer.get(), width_ * height_,
                std::numeric_limits<float>::infinity());
    std::fill_n(colorBuffer.get(), width_ * height_, 0);

#pragma omp parallel for
    for (size_t i = 0; i < width_ * height_; i++) {
        float min_depth = std::numeric_limits<float>::infinity();
        uint32_t color = 0;

        for (size_t thread_id = 0; thread_id < kNProc; thread_id++) {
            float depth = depthBuffer_all_thread[thread_id][i];
            if (depth < min_depth) {
                min_depth = depth;
                color = colorBuffer_all_thread[thread_id][i];
            }
        }
        depthBuffer[i] = min_depth;
        colorBuffer[i] = color;
    }

    std::memcpy(buffer, colorBuffer.get(), width_ * height_ * sizeof(uint32_t));
    auto merge_end_time = std::chrono::high_resolution_clock::now();
    auto merge_duration = std::chrono::duration_cast<std::chrono::microseconds>(
        merge_end_time - merge_start_time);
    
    auto total_end_time = std::chrono::high_resolution_clock::now();
    auto total_duration = std::chrono::duration_cast<std::chrono::microseconds>(
        total_end_time - total_start_time);
    
    // 填充统计信息
    stats.buffer_alloc_ms = buffer_alloc_duration.count() / 1000.0;
    stats.rasterization_ms = raster_duration.count() / 1000.0;
    stats.merge_ms = merge_duration.count() / 1000.0;
    stats.total_ms = total_duration.count() / 1000.0;
    
    return stats;
}

// Tile-based光栅化管线实现
SimpleRenderer::TileRenderStats SimpleRenderer::ExecuteTileBasedPipeline(
    const Model &model,
    const std::vector<Vertex> &processedVertices,
    uint32_t *buffer) {
    
    TileRenderStats stats;
    auto total_start_time = std::chrono::high_resolution_clock::now();
    
    // 1. Setup阶段
    auto setup_start_time = std::chrono::high_resolution_clock::now();
    const size_t TILE_SIZE = 64; // 64x64 pixels per tile
    const size_t tiles_x = (width_ + TILE_SIZE - 1) / TILE_SIZE;
    const size_t tiles_y = (height_ + TILE_SIZE - 1) / TILE_SIZE;
    const size_t total_tiles = tiles_x * tiles_y;
    
    // 为每个tile创建三角形列表
    std::vector<std::vector<TriangleInfo>> tile_triangles(total_tiles);
    auto setup_end_time = std::chrono::high_resolution_clock::now();
    auto setup_duration = std::chrono::duration_cast<std::chrono::microseconds>(
        setup_end_time - setup_start_time);
    
    // 2. Triangle-Tile binning阶段
    auto binning_start_time = std::chrono::high_resolution_clock::now();
    TriangleTileBinning(model, processedVertices, tile_triangles, tiles_x, tiles_y, TILE_SIZE);
    auto binning_end_time = std::chrono::high_resolution_clock::now();
    auto binning_duration = std::chrono::duration_cast<std::chrono::microseconds>(
        binning_end_time - binning_start_time);
    
    // 3. 为每个线程创建framebuffer
    auto buffer_alloc_start_time = std::chrono::high_resolution_clock::now();
    std::vector<std::unique_ptr<float[]>> depthBuffer_all_thread(kNProc);
    std::vector<std::unique_ptr<uint32_t[]>> colorBuffer_all_thread(kNProc);
    
    for (size_t thread_id = 0; thread_id < kNProc; thread_id++) {
        depthBuffer_all_thread[thread_id] = 
            std::make_unique<float[]>(width_ * height_);
        colorBuffer_all_thread[thread_id] = 
            std::make_unique<uint32_t[]>(width_ * height_);
            
        std::fill_n(depthBuffer_all_thread[thread_id].get(), width_ * height_,
                    std::numeric_limits<float>::infinity());
        std::fill_n(colorBuffer_all_thread[thread_id].get(), width_ * height_, 0);
    }
    auto buffer_alloc_end_time = std::chrono::high_resolution_clock::now();
    auto buffer_alloc_duration = std::chrono::duration_cast<std::chrono::microseconds>(
        buffer_alloc_end_time - buffer_alloc_start_time);
    
    // 4. 并行处理每个tile
    auto rasterization_start_time = std::chrono::high_resolution_clock::now();
#pragma omp parallel num_threads(kNProc) default(none) \
    shared(tile_triangles, rasterizer_, shader_, width_, height_, \
           depthBuffer_all_thread, colorBuffer_all_thread, tiles_x, tiles_y, total_tiles)
    {
        int thread_id = omp_get_thread_num();
        auto &depthBuffer_per_thread = depthBuffer_all_thread[thread_id];
        auto &colorBuffer_per_thread = colorBuffer_all_thread[thread_id];

        // 为当前线程创建tile局部缓冲区
        std::unique_ptr<float[]> tile_depth_buffer = 
            std::make_unique<float[]>(TILE_SIZE * TILE_SIZE);
        std::unique_ptr<uint32_t[]> tile_color_buffer = 
            std::make_unique<uint32_t[]>(TILE_SIZE * TILE_SIZE);

#pragma omp for
        for (size_t tile_id = 0; tile_id < total_tiles; tile_id++) {
            // 按照tile进行光栅化
            RasterizeTile(tile_id, tile_triangles[tile_id], 
                         tiles_x, tiles_y, TILE_SIZE,
                         tile_depth_buffer.get(), tile_color_buffer.get(),
                         depthBuffer_per_thread, colorBuffer_per_thread);
        }
    }
    auto rasterization_end_time = std::chrono::high_resolution_clock::now();
    auto rasterization_duration = std::chrono::duration_cast<std::chrono::microseconds>(
        rasterization_end_time - rasterization_start_time);
    
    // 5. 合并所有线程结果
    auto merge_start_time = std::chrono::high_resolution_clock::now();
    std::unique_ptr<float[]> depthBuffer = 
        std::make_unique<float[]>(width_ * height_);
    std::unique_ptr<uint32_t[]> colorBuffer = 
        std::make_unique<uint32_t[]>(width_ * height_);

    std::fill_n(depthBuffer.get(), width_ * height_,
                std::numeric_limits<float>::infinity());
    std::fill_n(colorBuffer.get(), width_ * height_, 0);

#pragma omp parallel for
    for (size_t i = 0; i < width_ * height_; i++) {
        float min_depth = std::numeric_limits<float>::infinity();
        uint32_t color = 0;

        for (size_t thread_id = 0; thread_id < kNProc; thread_id++) {
            float depth = depthBuffer_all_thread[thread_id][i];
            if (depth < min_depth) {
                min_depth = depth;
                color = colorBuffer_all_thread[thread_id][i];
            }
        }
        depthBuffer[i] = min_depth;
        colorBuffer[i] = color;
    }

    std::memcpy(buffer, colorBuffer.get(), width_ * height_ * sizeof(uint32_t));
    auto merge_end_time = std::chrono::high_resolution_clock::now();
    auto merge_duration = std::chrono::duration_cast<std::chrono::microseconds>(
        merge_end_time - merge_start_time);
    
    // 6. Tile可视化调试
    auto visualization_start_time = std::chrono::high_resolution_clock::now();
    DrawTileVisualization(buffer, tile_triangles, tiles_x, tiles_y, TILE_SIZE);
    auto visualization_end_time = std::chrono::high_resolution_clock::now();
    auto visualization_duration = std::chrono::duration_cast<std::chrono::microseconds>(
        visualization_end_time - visualization_start_time);
    
    auto total_end_time = std::chrono::high_resolution_clock::now();
    auto total_duration = std::chrono::duration_cast<std::chrono::microseconds>(
        total_end_time - total_start_time);
    
    // 填充统计信息
    stats.setup_ms = setup_duration.count() / 1000.0;
    stats.binning_ms = binning_duration.count() / 1000.0;
    stats.buffer_alloc_ms = buffer_alloc_duration.count() / 1000.0;
    stats.rasterization_ms = rasterization_duration.count() / 1000.0;
    stats.merge_ms = merge_duration.count() / 1000.0;
    stats.visualization_ms = visualization_duration.count() / 1000.0;
    stats.total_ms = total_duration.count() / 1000.0;
    
    return stats;
}

}  // namespace simple_renderer
