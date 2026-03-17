//! Deferred renderer (AoS layout).
//!
//! Deferred renderer: collect closest fragments, then shade only depth winners.
//!
//! Algorithm:
//! 1. Vertex transform (sequential — `vertex_shader` needs `&mut self`)
//! 2. Parallel rasterization with per-thread depth testing:
//!    each thread keeps only the closest fragment per pixel (NO backface culling)
//! 3. Parallel merge + deferred shading:
//!    find closest fragment across threads and shade only winners (in parallel)
//! 4. Write to output buffer

use log::debug;
use std::time::Instant;

use rayon::prelude::*;

use crate::fragment::Fragment;
use crate::model::Model;
use crate::rasterizer::Rasterizer;
use crate::renderers::base;
use crate::renderers::Renderer;
use crate::shader::Shader;

/// AoS deferred renderer: collect closest fragments per thread, then shade only the winners.
///
/// Key difference from `PerTriangleRenderer`:
/// - NO backface culling — all fragments are collected
/// - Depth resolve happens after rasterization, not during
/// - Fragment shader is called ONCE per pixel (on the winner only)
pub struct DeferredRenderer {
    chunk_depth: Vec<Vec<f32>>,
    chunk_frags: Vec<Vec<Option<(Fragment, usize)>>>,
}

impl DeferredRenderer {
    pub fn new(_width: usize, _height: usize) -> Self {
        Self {
            chunk_depth: Vec::new(),
            chunk_frags: Vec::new(),
        }
    }
}

impl Renderer for DeferredRenderer {
    fn render(
        &mut self,
        model: &Model,
        shader: &Shader,
        out_buffer: &mut [u32],
        width: usize,
        height: usize,
    ) -> crate::error::Result<()> {
        let t = Instant::now();
        // 1. Vertex transform (sequential — Shader is not Sync yet)
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

        let t = Instant::now();
        // 2. Parallel rasterization with per-thread depth testing
        //
        // Each thread keeps only the CLOSEST fragment per pixel, drastically
        // reducing memory from O(threads × pixels × fragments_per_pixel) to
        // O(threads × pixels).
        let num_pixels = width * height;
        let faces = model.faces();
        let rasterizer = Rasterizer::new(width, height);
        let num_threads = rayon::current_num_threads();
        let chunk_size = std::cmp::max(faces.len() / num_threads, 1);
        let num_chunks = faces.chunks(chunk_size).len();

        // Reuse chunk buffers across frames
        self.chunk_depth.resize_with(num_chunks, Vec::new);
        self.chunk_frags.resize_with(num_chunks, Vec::new);
        self.chunk_depth.truncate(num_chunks);
        self.chunk_frags.truncate(num_chunks);
        for buf in &mut self.chunk_depth {
            buf.resize(num_pixels, f32::INFINITY);
            buf.fill(f32::INFINITY);
        }
        for buf in &mut self.chunk_frags {
            buf.resize(num_pixels, None);
            buf.fill(None);
        }

        let chunk_depth = &mut self.chunk_depth;
        let chunk_frags = &mut self.chunk_frags;

        faces
            .par_chunks(chunk_size)
            .enumerate()
            .zip(chunk_depth.par_iter_mut().zip(chunk_frags.par_iter_mut()))
            .for_each(|((chunk_idx, face_chunk), (depth_buf, frag_buf))| {
                for (local_idx, face) in face_chunk.iter().enumerate() {
                    let face_idx = chunk_idx * chunk_size + local_idx;
                    let v0 = &processed_vertices[face.indices[0]];
                    let v1 = &processed_vertices[face.indices[1]];
                    let v2 = &processed_vertices[face.indices[2]];

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
                            frag_buf[idx] = Some((frag, face_idx));
                        }
                    });
                }
            });

        let collect_ms = t.elapsed().as_secs_f64() * 1000.0;

        // 3. Parallel merge + deferred shading
        //
        // For each pixel, find the closest fragment across all threads,
        // then shade only that winner. Both merge and shade run in parallel.
        let t = Instant::now();
        let final_buffer: Vec<u32> = (0..num_pixels)
            .into_par_iter()
            .map(|i| {
                let mut best_depth = f32::INFINITY;
                let mut best_chunk: Option<usize> = None;

                for (ci, depth_buf) in chunk_depth.iter().enumerate() {
                    if depth_buf[i] < best_depth {
                        best_depth = depth_buf[i];
                        best_chunk = Some(ci);
                    }
                }

                if let Some(ci) = best_chunk {
                    if let Some((winner_frag, winner_face_idx)) = &chunk_frags[ci][i] {
                        let material = &faces[*winner_face_idx].material;
                        u32::from(shader.fragment_shader(winner_frag, material))
                    } else {
                        0u32
                    }
                } else {
                    0u32
                }
            })
            .collect();
        out_buffer[..num_pixels].copy_from_slice(&final_buffer);
        let shade_ms = t.elapsed().as_secs_f64() * 1000.0;

        let sum_ms = vertex_ms + collect_ms + shade_ms;
        if sum_ms > 0.0 {
            debug!("=== DEFERRED RENDERING PERFORMANCE ===");
            debug!(
                "Vertex Shader:        {:8.3} ms ({:5.1}%)",
                vertex_ms,
                vertex_ms / sum_ms * 100.0
            );
            debug!("Fragment Collection:  {:8.3} ms", collect_ms);
            debug!("Deferred Shading:     {:8.3} ms", shade_ms);
            debug!("Total:                {:8.3} ms", sum_ms);
            debug!("=======================================");
        }

        Ok(())
    }
}

// ── Tests ──────────────────────────────────────────────────────────────────

#[cfg(test)]
mod tests {
    use super::*;
    use crate::renderers::test_utils::{create_test_model, test_shader};

    // ── Visible triangle produces pixels ──────────────────────────────

    #[test]
    fn visible_triangle_produces_nonzero_pixels() {
        let width = 100;
        let height = 100;
        let mut renderer = DeferredRenderer::new(width, height);
        let shader = test_shader();

        // Front-facing triangle
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

    // ── No backface culling (key difference from PerTriangle) ─────────

    #[test]
    fn backface_triangle_still_produces_pixels() {
        let width = 100;
        let height = 100;
        let mut renderer = DeferredRenderer::new(width, height);
        let shader = test_shader();

        // Reversed winding: [0, 2, 1] — this is a backface
        // DeferredRenderer does NOT cull backfaces, so it should still produce pixels
        let model = create_test_model(
            &[[0.0, 0.0, 0.0], [0.5, 0.0, 0.0], [0.0, 0.5, 0.0]],
            [0.0, 0.0, 1.0],
            &[[0, 2, 1]],
        );

        let mut buffer = vec![0u32; width * height];
        let _ = renderer.render(&model, &shader, &mut buffer, width, height);

        let nonzero_count = buffer.iter().filter(|&&p| p != 0).count();
        assert!(
            nonzero_count > 0,
            "deferred renderer should NOT cull backfaces, got {} pixels",
            nonzero_count
        );
    }

    // ── Empty model ──────────────────────────────────────────────────

    #[test]
    fn empty_model_produces_no_pixels() {
        let width = 100;
        let height = 100;
        let mut renderer = DeferredRenderer::new(width, height);
        let shader = test_shader();

        // Model with no faces
        let model = create_test_model(
            &[[0.0, 0.0, 0.0], [0.5, 0.0, 0.0], [0.0, 0.5, 0.0]],
            [0.0, 0.0, 1.0],
            &[], // no faces
        );

        let mut buffer = vec![0u32; width * height];
        let result = renderer.render(&model, &shader, &mut buffer, width, height);
        assert!(result.is_ok());

        let nonzero_count = buffer.iter().filter(|&&p| p != 0).count();
        assert_eq!(
            nonzero_count, 0,
            "empty model should produce no pixels, got {}",
            nonzero_count
        );
    }

    // ── Off-screen triangle ──────────────────────────────────────────

    #[test]
    fn offscreen_triangle_produces_no_pixels() {
        let width = 100;
        let height = 100;
        let mut renderer = DeferredRenderer::new(width, height);
        let shader = test_shader();

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

    // ── Depth resolve: closer triangle wins ──────────────────────────

    #[test]
    fn closer_triangle_wins_depth_resolve() {
        let width = 100;
        let height = 100;
        let mut renderer = DeferredRenderer::new(width, height);
        let shader = test_shader();

        // Two overlapping triangles at different depths
        // Triangle 1: closer (z=0.0)
        // Triangle 2: farther (z=0.5)
        let model = create_test_model(
            &[
                [0.0, 0.0, 0.0],
                [0.5, 0.0, 0.0],
                [0.0, 0.5, 0.0],
                [0.0, 0.0, 0.5],
                [0.5, 0.0, 0.5],
                [0.0, 0.5, 0.5],
            ],
            [0.0, 0.0, 1.0],
            &[[0, 1, 2], [3, 4, 5]],
        );

        let mut buffer = vec![0u32; width * height];
        let result = renderer.render(&model, &shader, &mut buffer, width, height);
        assert!(result.is_ok());

        let nonzero_count = buffer.iter().filter(|&&p| p != 0).count();
        assert!(
            nonzero_count > 0,
            "overlapping triangles should produce pixels"
        );
    }
}
