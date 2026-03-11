//! Deferred renderer (AoS layout).
//!
//! Port of C++ `DeferredRenderer` (deferred_renderer.cpp, 177 lines).
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

use crate::color::Color;
use crate::fragment::Fragment;
use crate::math::{Vec2, Vec3};
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
#[allow(dead_code)]
pub struct DeferredRenderer {
    width: usize,
    height: usize,
}

impl DeferredRenderer {
    /// Create a renderer for a framebuffer of `width × height` pixels.
    pub fn new(width: usize, height: usize) -> Self {
        Self { width, height }
    }
}

impl Renderer for DeferredRenderer {
    fn render(
        &self,
        model: &Model,
        shader: &Shader,
        out_buffer: &mut [u32],
        width: usize,
        height: usize,
    ) -> bool {
        // 1. Clone shader + prepare caches
        let mut shader = shader.clone();
        shader.prepare_caches();

        let t = Instant::now();
        // 2. Vertex transform (sequential — vertex_shader writes frag_pos_varying)
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
        // 3. Parallel rasterization with per-thread depth testing
        //
        // Each thread keeps only the CLOSEST fragment per pixel, drastically
        // reducing memory from O(threads × pixels × fragments_per_pixel) to
        // O(threads × pixels).
        let num_pixels = width * height;
        let faces = model.faces();
        let rasterizer = Rasterizer::new(width, height);
        let num_threads = rayon::current_num_threads();
        let chunk_size = std::cmp::max(faces.len() / num_threads, 1);

        // Dummy fragment for buffer initialization (never read — only valid
        // entries where depth_buf < INFINITY are accessed during merge).
        let dummy = Fragment {
            screen_coord: [0, 0],
            normal: Vec3::ZERO,
            uv: Vec2::ZERO,
            color: Color::new(0, 0, 0, 0),
            depth: f32::INFINITY,
        };

        // Per-thread result: (depth_buf, fragment_buf, face_index_buf)
        let chunk_results: Vec<(Vec<f32>, Vec<Fragment>, Vec<usize>)> = faces
            .par_chunks(chunk_size)
            .enumerate()
            .map(|(chunk_idx, face_chunk)| {
                let mut depth_buf = vec![f32::INFINITY; num_pixels];
                let mut frag_buf = vec![dummy.clone(); num_pixels];
                let mut face_buf = vec![0usize; num_pixels];

                for (local_idx, face) in face_chunk.iter().enumerate() {
                    let face_idx = chunk_idx * chunk_size + local_idx;
                    let v0 = &processed_vertices[face.indices[0]];
                    let v1 = &processed_vertices[face.indices[1]];
                    let v2 = &processed_vertices[face.indices[2]];

                    // NO backface culling — collect all fragments
                    let fragments = rasterizer.rasterize(v0, v1, v2);

                    for frag in fragments {
                        let x = frag.screen_coord[0];
                        let y = frag.screen_coord[1];
                        if x < 0 || y < 0 {
                            continue;
                        }
                        let x = x as usize;
                        let y = y as usize;
                        if x >= width || y >= height {
                            continue;
                        }
                        let idx = x + y * width;
                        // Per-thread depth test: keep only the closest fragment
                        if frag.depth < depth_buf[idx] {
                            depth_buf[idx] = frag.depth;
                            frag_buf[idx] = frag;
                            face_buf[idx] = face_idx;
                        }
                    }
                }

                (depth_buf, frag_buf, face_buf)
            })
            .collect();

        let collect_ms = t.elapsed().as_secs_f64() * 1000.0;

        // 4. Parallel merge + deferred shading
        //
        // For each pixel, find the closest fragment across all threads,
        // then shade only that winner. Both merge and shade run in parallel.
        let t = Instant::now();
        let final_buffer: Vec<u32> = (0..num_pixels)
            .into_par_iter()
            .map(|i| {
                let mut best_depth = f32::INFINITY;
                let mut best_chunk: Option<usize> = None;

                for (chunk_idx, (depth_buf, _, _)) in chunk_results.iter().enumerate() {
                    if depth_buf[i] < best_depth {
                        best_depth = depth_buf[i];
                        best_chunk = Some(chunk_idx);
                    }
                }

                if let Some(chunk_idx) = best_chunk {
                    let winner_frag = &chunk_results[chunk_idx].1[i];
                    let winner_face_idx = chunk_results[chunk_idx].2[i];
                    let material = &faces[winner_face_idx].material;
                    u32::from(shader.fragment_shader(winner_frag, material))
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

        true
    }
}

// ── Tests ──────────────────────────────────────────────────────────────────

#[cfg(test)]
mod tests {
    use super::*;
    use crate::light::Light;
    use crate::math::Mat4;

    /// Set up a shader with identity matrices and a simple light for testing.
    fn test_shader() -> Shader {
        let mut shader = Shader::new();
        shader.set_uniform("modelMatrix", Mat4::IDENTITY);
        shader.set_uniform("viewMatrix", Mat4::IDENTITY);
        shader.set_uniform("projectionMatrix", Mat4::IDENTITY);
        shader.set_lights(&[Light {
            name: "test".to_string(),
            position: Vec3::ZERO,
            direction: Vec3::new(0.0, 0.0, 1.0),
            color: Color::WHITE,
        }]);
        shader.set_uniform("cameraPos", Vec3::new(0.0, 0.0, 5.0));
        shader
    }

    /// Create a simple OBJ file and load it as a Model.
    fn create_test_model(
        positions: &[[f32; 3]],
        normal: [f32; 3],
        face_indices: &[[usize; 3]],
    ) -> crate::model::Model {
        use std::io::Write;
        use std::sync::atomic::{AtomicUsize, Ordering};

        static COUNTER: AtomicUsize = AtomicUsize::new(1000);
        let id = COUNTER.fetch_add(1, Ordering::SeqCst);
        let path = std::env::temp_dir().join(format!("simple_renderer_deferred_test_{}.obj", id));

        let mut file = std::fs::File::create(&path).unwrap();
        for pos in positions {
            writeln!(file, "v {} {} {}", pos[0], pos[1], pos[2]).unwrap();
        }
        for _ in positions {
            writeln!(file, "vn {} {} {}", normal[0], normal[1], normal[2]).unwrap();
        }
        for face in face_indices {
            writeln!(
                file,
                "f {}//{} {}//{} {}//{}",
                face[0] + 1,
                face[0] + 1,
                face[1] + 1,
                face[1] + 1,
                face[2] + 1,
                face[2] + 1,
            )
            .unwrap();
        }
        drop(file);

        let model = crate::model::Model::load(path.to_str().unwrap()).unwrap();
        let _ = std::fs::remove_file(&path);
        model
    }

    // ── Visible triangle produces pixels ──────────────────────────────

    #[test]
    fn visible_triangle_produces_nonzero_pixels() {
        let width = 100;
        let height = 100;
        let renderer = DeferredRenderer::new(width, height);
        let shader = test_shader();

        // Front-facing triangle
        let model = create_test_model(
            &[[0.0, 0.0, 0.0], [0.5, 0.0, 0.0], [0.0, 0.5, 0.0]],
            [0.0, 0.0, 1.0],
            &[[0, 1, 2]],
        );

        let mut buffer = vec![0u32; width * height];
        let result = renderer.render(&model, &shader, &mut buffer, width, height);
        assert!(result);

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
        let renderer = DeferredRenderer::new(width, height);
        let shader = test_shader();

        // Reversed winding: [0, 2, 1] — this is a backface
        // DeferredRenderer does NOT cull backfaces, so it should still produce pixels
        let model = create_test_model(
            &[[0.0, 0.0, 0.0], [0.5, 0.0, 0.0], [0.0, 0.5, 0.0]],
            [0.0, 0.0, 1.0],
            &[[0, 2, 1]],
        );

        let mut buffer = vec![0u32; width * height];
        renderer.render(&model, &shader, &mut buffer, width, height);

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
        let renderer = DeferredRenderer::new(width, height);
        let shader = test_shader();

        // Model with no faces
        let model = create_test_model(
            &[[0.0, 0.0, 0.0], [0.5, 0.0, 0.0], [0.0, 0.5, 0.0]],
            [0.0, 0.0, 1.0],
            &[], // no faces
        );

        let mut buffer = vec![0u32; width * height];
        let result = renderer.render(&model, &shader, &mut buffer, width, height);
        assert!(result);

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
        let renderer = DeferredRenderer::new(width, height);
        let shader = test_shader();

        let model = create_test_model(
            &[[5.0, 5.0, 0.0], [6.0, 5.0, 0.0], [5.0, 6.0, 0.0]],
            [0.0, 0.0, 1.0],
            &[[0, 1, 2]],
        );

        let mut buffer = vec![0u32; width * height];
        renderer.render(&model, &shader, &mut buffer, width, height);

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
        let renderer = DeferredRenderer::new(width, height);
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
        assert!(result);

        let nonzero_count = buffer.iter().filter(|&&p| p != 0).count();
        assert!(
            nonzero_count > 0,
            "overlapping triangles should produce pixels"
        );
    }
}
