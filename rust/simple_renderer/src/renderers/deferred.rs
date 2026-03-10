//! Deferred renderer (AoS layout).
//!
//! Port of C++ `DeferredRenderer` (deferred_renderer.cpp, 177 lines).
//!
//! Algorithm:
//! 1. Vertex transform (sequential — `vertex_shader` needs `&mut self`)
//! 2. Parallel rasterization: collect ALL fragments per pixel (no backface culling)
//! 3. Merge per-thread fragment buffers
//! 4. Depth resolve: find closest fragment per pixel
//! 5. Deferred shading: shade only winner fragments
//! 6. Write to output buffer

use rayon::prelude::*;

use crate::fragment::Fragment;
use crate::model::Model;
use crate::rasterizer::Rasterizer;
use crate::renderers::base;
use crate::renderers::Renderer;
use crate::shader::Shader;

/// AoS deferred renderer: collect all fragments, then shade only the winners.
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

/// Per-pixel fragment entry: stores fragment + face index for material lookup.
struct FragmentEntry {
    fragment: Fragment,
    face_index: usize,
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

        // 3. Parallel rasterization: collect ALL fragments (NO backface culling)
        let num_pixels = width * height;
        let faces = model.faces();
        let rasterizer = Rasterizer::new(width, height);
        let num_threads = rayon::current_num_threads();
        let chunk_size = std::cmp::max(faces.len() / num_threads, 1);

        // Per-thread fragment buffers: Vec<Vec<FragmentEntry>> per pixel
        let chunk_results: Vec<Vec<Vec<FragmentEntry>>> = faces
            .par_chunks(chunk_size)
            .enumerate()
            .map(|(_chunk_idx, face_chunk)| {
                let mut pixel_fragments: Vec<Vec<FragmentEntry>> =
                    (0..num_pixels).map(|_| Vec::new()).collect();

                // Compute starting face index for this chunk
                let chunk_start = face_chunk.as_ptr() as usize - faces.as_ptr() as usize;
                let chunk_start_idx = chunk_start / std::mem::size_of_val(&faces[0]);

                for (local_idx, face) in face_chunk.iter().enumerate() {
                    let face_idx = chunk_start_idx + local_idx;
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
                        pixel_fragments[idx].push(FragmentEntry {
                            fragment: frag,
                            face_index: face_idx,
                        });
                    }
                }

                pixel_fragments
            })
            .collect();

        // 4. Merge per-thread fragment buffers + depth resolve + deferred shading
        // For each pixel: collect from all threads, find min depth, shade winner
        for i in 0..num_pixels {
            let mut best_entry: Option<(&FragmentEntry, f32)> = None;

            for thread_buf in &chunk_results {
                for entry in &thread_buf[i] {
                    let depth = entry.fragment.depth;
                    match best_entry {
                        None => best_entry = Some((entry, depth)),
                        Some((_, best_depth)) if depth < best_depth => {
                            best_entry = Some((entry, depth));
                        }
                        _ => {}
                    }
                }
            }

            if let Some((winner, _)) = best_entry {
                let material = &faces[winner.face_index].material;
                let color = shader.fragment_shader(&winner.fragment, material);
                out_buffer[i] = u32::from(color);
            }
        }

        true
    }
}

// ── Tests ──────────────────────────────────────────────────────────────────

#[cfg(test)]
mod tests {
    use super::*;
    use crate::color::Color;
    use crate::light::Light;
    use crate::math::{Mat4, Vec3};

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
        let path = std::env::temp_dir()
            .join(format!("simple_renderer_deferred_test_{}.obj", id));

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
