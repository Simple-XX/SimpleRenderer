// Copyright The SimpleGameEngine Contributors


//! 逐三角形前向渲染器（AoS 布局）。
//!
//! 基于分块并行光栅化的逐三角形前向渲染器。
//!
//! 算法：
//! 1. 顶点变换（串行 — `vertex_shader` 需要 `&mut self`）
//! 2. 对面片分块进行并行光栅化（rayon `par_chunks`）
//! 3. 每个分块拥有独立的深度和颜色缓冲区，最后合并结果

use log::debug;
use std::time::Instant;

use rayon::prelude::*;

use crate::math::Vec2;
use crate::model::Model;
use crate::rasterizer::Rasterizer;
use crate::renderers::base;
use crate::renderers::Renderer;
use crate::shader::Shader;

/// AoS 逐三角形渲染器，每线程拥有独立的帧缓冲区。
///
/// 顶点变换 → 并行光栅化（背面剔除 + 深度测试）→ 合并。
pub struct PerTriangleRenderer {
    chunk_depth: Vec<Vec<f32>>,
    chunk_color: Vec<Vec<u32>>,
}

impl PerTriangleRenderer {
    pub fn new(_width: usize, _height: usize) -> Self {
        Self {
            chunk_depth: Vec::new(),
            chunk_color: Vec::new(),
        }
    }
}

impl Renderer for PerTriangleRenderer {
    fn render(
        &mut self,
        model: &Model,
        shader: &Shader,
        out_buffer: &mut [u32],
        width: usize,
        height: usize,
    ) -> crate::error::Result<()> {
        let t = Instant::now();
        // 1. 顶点变换（串行 — Shader 尚未实现 Sync）
        let vertices = model.vertices();
        let processed_vertices: Vec<_> = vertices
            .iter()
            .map(|v| {
                let clip = shader.vertex_shader(v);
                let ndc = base::perspective_division(&clip);
                base::viewport_transform(&ndc, width, height)
            })
            .collect();
        let vertex_ms = t.elapsed().as_secs_f64() * 1000.0;

        // 2. 对面片分块进行并行光栅化
        let t = Instant::now();
        let num_pixels = width * height;
        let faces = model.faces();
        let rasterizer = Rasterizer::new(width, height);
        let num_threads = rayon::current_num_threads();
        let chunk_size = std::cmp::max(faces.len() / num_threads, 1);
        let num_chunks = faces.chunks(chunk_size).len();

        // 跨帧复用分块缓冲区
        self.chunk_depth.resize_with(num_chunks, Vec::new);
        self.chunk_color.resize_with(num_chunks, Vec::new);
        self.chunk_depth.truncate(num_chunks);
        self.chunk_color.truncate(num_chunks);
        for buf in &mut self.chunk_depth {
            buf.resize(num_pixels, f32::INFINITY);
            buf.fill(f32::INFINITY);
        }
        for buf in &mut self.chunk_color {
            buf.resize(num_pixels, 0u32);
            buf.fill(0u32);
        }

        let chunk_depth = &mut self.chunk_depth;
        let chunk_color = &mut self.chunk_color;

        faces
            .par_chunks(chunk_size)
            .zip(chunk_depth.par_iter_mut().zip(chunk_color.par_iter_mut()))
            .for_each(|(face_chunk, (depth_buf, color_buf))| {
                for face in face_chunk {
                    let v0 = &processed_vertices[face.indices[0]];
                    let v1 = &processed_vertices[face.indices[1]];
                    let v2 = &processed_vertices[face.indices[2]];

                    // 背面剔除（屏幕空间叉积）
                    let s0 = Vec2::new(v0.position.x, v0.position.y);
                    let s1 = Vec2::new(v1.position.x, v1.position.y);
                    let s2 = Vec2::new(v2.position.x, v2.position.y);
                    let edge1 = s1 - s0;
                    let edge2 = s2 - s0;
                    let cross = edge1.x * edge2.y - edge1.y * edge2.x;
                    if cross > 0.0 {
                        continue; // 背面
                    }

                    rasterizer.rasterize_each(v0, v1, v2, |frag| {
                        let x = frag.screen_coord[0];
                        let y = frag.screen_coord[1];
                        if x < 0 || y < 0 {
                            return;
                        }
                        let (x, y) = (x as usize, y as usize);
                        if x >= width || y >= height {
                            return;
                        }
                        let idx = x + y * width;
                        if frag.depth < depth_buf[idx] {
                            depth_buf[idx] = frag.depth;
                            let color = shader.fragment_shader(&frag, &face.material);
                            color_buf[idx] = u32::from(color);
                        }
                    });
                }
            });

        let raster_ms = t.elapsed().as_secs_f64() * 1000.0;

        // 3. 合并线程结果 — 逐像素取最小深度（并行）
        let t = Instant::now();
        out_buffer[..num_pixels]
            .par_iter_mut()
            .enumerate()
            .for_each(|(i, pixel)| {
                let mut min_depth = f32::INFINITY;
                let mut final_color = 0u32;
                for chunk_idx in 0..num_chunks {
                    if chunk_depth[chunk_idx][i] < min_depth {
                        min_depth = chunk_depth[chunk_idx][i];
                        final_color = chunk_color[chunk_idx][i];
                    }
                }
                if min_depth < f32::INFINITY {
                    *pixel = final_color;
                }
            });
        let merge_ms = t.elapsed().as_secs_f64() * 1000.0;

        let sum_ms = vertex_ms + raster_ms + merge_ms;
        if sum_ms > 0.0 {
            debug!("=== PER-TRIANGLE RENDERING PERFORMANCE ===");
            debug!(
                "Vertex Shader:    {:8.3} ms ({:5.1}%)",
                vertex_ms,
                vertex_ms / sum_ms * 100.0
            );
            debug!(
                "Rasterization:    {:8.3} ms ({:5.1}%)",
                raster_ms,
                raster_ms / sum_ms * 100.0
            );
            debug!(
                "Merge:            {:8.3} ms ({:5.1}%)",
                merge_ms,
                merge_ms / sum_ms * 100.0
            );
            debug!("Total:            {:8.3} ms", sum_ms);
            debug!("==========================================");
        }
        Ok(())
    }
}


#[cfg(test)]
mod tests {
    use super::*;
    use crate::renderers::test_utils::{create_test_model, test_shader};


    #[test]
    fn visible_triangle_produces_nonzero_pixels() {
        let width = 100;
        let height = 100;
        let mut renderer = PerTriangleRenderer::new(width, height);
        let shader = test_shader();

        // 裁剪空间中映射到可见屏幕区域的三角形
        // 使用单位 MVP：裁剪坐标 == 模型坐标
        // 透视除法后（w=1）：不变
        // 视口变换后（100x100）：
        //   (0,0) → (50, 50), (0.5,0) → (75, 50), (0,0.5) → (50, 25)
        // 叉积：edge1=(25,0), edge2=(0,-25), cross=-625 → 正面
        let model = create_test_model(
            &[[0.0, 0.0, 0.0], [0.5, 0.0, 0.0], [0.0, 0.5, 0.0]],
            [0.0, 0.0, 1.0],
            &[[0, 1, 2]],
        );

        let mut buffer = vec![0u32; width * height];
        let result = renderer.render(&model, &shader, &mut buffer, width, height);
        assert!(result.is_ok());

        let nonzero_count = buffer.iter().filter(|&&p| p != 0).count();
        assert!(
            nonzero_count > 0,
            "visible triangle should produce non-zero pixels, got {}",
            nonzero_count
        );
    }


    #[test]
    fn backface_triangle_produces_no_pixels() {
        let width = 100;
        let height = 100;
        let mut renderer = PerTriangleRenderer::new(width, height);
        let shader = test_shader();

        // 反转绕序：[0, 2, 1] 而非 [0, 1, 2]
        // 视口变换后：
        //   v0=(50,50), v2=(50,25), v1=(75,50)
        //   edge1=(0,-25), edge2=(25,0), cross=625 > 0 → 背面
        let model = create_test_model(
            &[[0.0, 0.0, 0.0], [0.5, 0.0, 0.0], [0.0, 0.5, 0.0]],
            [0.0, 0.0, 1.0],
            &[[0, 2, 1]],
        );

        let mut buffer = vec![0u32; width * height];
        let _ = renderer.render(&model, &shader, &mut buffer, width, height);

        let nonzero_count = buffer.iter().filter(|&&p| p != 0).count();
        assert_eq!(
            nonzero_count, 0,
            "backface triangle should produce no pixels, got {}",
            nonzero_count
        );
    }


    #[test]
    fn offscreen_triangle_produces_no_pixels() {
        let width = 100;
        let height = 100;
        let mut renderer = PerTriangleRenderer::new(width, height);
        let shader = test_shader();

        // 顶点远超 NDC 范围
        // 视口变换后（100x100）：
        //   (5,5) → (300, -200)，全部在屏幕外
        let model = create_test_model(
            &[[5.0, 5.0, 0.0], [6.0, 5.0, 0.0], [5.0, 6.0, 0.0]],
            [0.0, 0.0, 1.0],
            &[[0, 1, 2]],
        );

        let mut buffer = vec![0u32; width * height];
        let _ = renderer.render(&model, &shader, &mut buffer, width, height);

        let nonzero_count = buffer.iter().filter(|&&p| p != 0).count();
        assert_eq!(
            nonzero_count, 0,
            "off-screen triangle should produce no pixels, got {}",
            nonzero_count
        );
    }
}
