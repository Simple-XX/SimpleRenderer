//! Per-triangle forward renderer (AoS layout).
//!
//! Port of C++ `PerTriangleRenderer` (per_triangle_renderer.cpp, 173 lines).
//!
//! Algorithm:
//! 1. Vertex transform (sequential — `vertex_shader` needs `&mut self`)
//! 2. Parallel rasterization over face chunks (rayon `par_chunks`)
//! 3. Per-chunk depth + color buffers, merged at the end

use log::debug;
use std::time::Instant;

use rayon::prelude::*;

use crate::math::Vec2;
use crate::model::Model;
use crate::rasterizer::Rasterizer;
use crate::renderers::base;
use crate::renderers::Renderer;
use crate::shader::Shader;

/// AoS per-triangle renderer with per-thread local framebuffers.
///
/// Mirrors C++ `PerTriangleRenderer`: vertex transform → parallel
/// rasterization (backface culling + depth test) → merge.
#[allow(dead_code)]
pub struct PerTriangleRenderer {
    width: usize,
    height: usize,
}

impl PerTriangleRenderer {
    /// Create a renderer for a framebuffer of `width × height` pixels.
    pub fn new(width: usize, height: usize) -> Self {
        Self { width, height }
    }
}

impl Renderer for PerTriangleRenderer {
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

        // 3. Parallel rasterization over face chunks
        let t = Instant::now();
        let num_pixels = width * height;
        let faces = model.faces();
        let rasterizer = Rasterizer::new(width, height);
        let num_threads = rayon::current_num_threads();
        let chunk_size = std::cmp::max(faces.len() / num_threads, 1);

        let chunk_results: Vec<(Vec<f32>, Vec<u32>)> = faces
            .par_chunks(chunk_size)
            .map(|face_chunk| {
                let mut depth_buf = vec![f32::INFINITY; num_pixels];
                let mut color_buf = vec![0u32; num_pixels];

                for face in face_chunk {
                    let v0 = &processed_vertices[face.indices[0]];
                    let v1 = &processed_vertices[face.indices[1]];
                    let v2 = &processed_vertices[face.indices[2]];

                    // Backface culling (screen-space cross product)
                    let s0 = Vec2::new(v0.position.x, v0.position.y);
                    let s1 = Vec2::new(v1.position.x, v1.position.y);
                    let s2 = Vec2::new(v2.position.x, v2.position.y);
                    let edge1 = s1 - s0;
                    let edge2 = s2 - s0;
                    let cross = edge1.x * edge2.y - edge1.y * edge2.x;
                    if cross > 0.0 {
                        continue; // backface
                    }

                    let fragments = rasterizer.rasterize(v0, v1, v2);

                    for frag in &fragments {
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
                        if frag.depth < depth_buf[idx] {
                            depth_buf[idx] = frag.depth;
                            let color = shader.fragment_shader(frag, &face.material);
                            color_buf[idx] = u32::from(color);
                        }
                    }
                }

                (depth_buf, color_buf)
            })
            .collect();

        let raster_ms = t.elapsed().as_secs_f64() * 1000.0;

        // 4. Merge thread results — pick minimum depth per pixel
        let t = Instant::now();
        for i in 0..num_pixels {
            let mut min_depth = f32::INFINITY;
            let mut final_color = 0u32;
            for (depth_buf, color_buf) in &chunk_results {
                if depth_buf[i] < min_depth {
                    min_depth = depth_buf[i];
                    final_color = color_buf[i];
                }
            }
            if min_depth < f32::INFINITY {
                out_buffer[i] = final_color;
            }
        }
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
    ///
    /// Writes a temp file with the given vertices (with normals) and face.
    fn create_test_model(
        positions: &[[f32; 3]],
        normal: [f32; 3],
        face_indices: &[[usize; 3]],
    ) -> Model {
        use std::io::Write;
        use std::sync::atomic::{AtomicUsize, Ordering};

        static COUNTER: AtomicUsize = AtomicUsize::new(0);
        let id = COUNTER.fetch_add(1, Ordering::SeqCst);
        let path = std::env::temp_dir().join(format!("simple_renderer_test_{}.obj", id));

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

        let model = Model::load(path.to_str().unwrap()).unwrap();
        let _ = std::fs::remove_file(&path);
        model
    }

    // ── Visible triangle produces pixels ──────────────────────────────

    #[test]
    fn visible_triangle_produces_nonzero_pixels() {
        let width = 100;
        let height = 100;
        let renderer = PerTriangleRenderer::new(width, height);
        let shader = test_shader();

        // Triangle in clip space that maps to visible screen area
        // With identity MVP: clip == model positions
        // After persp div (w=1): unchanged
        // After viewport (100x100):
        //   (0,0) → (50, 50), (0.5,0) → (75, 50), (0,0.5) → (50, 25)
        // Cross product: edge1=(25,0), edge2=(0,-25), cross=-625 → front face
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

    // ── Backface culling ──────────────────────────────────────────────

    #[test]
    fn backface_triangle_produces_no_pixels() {
        let width = 100;
        let height = 100;
        let renderer = PerTriangleRenderer::new(width, height);
        let shader = test_shader();

        // Reversed winding: [0, 2, 1] instead of [0, 1, 2]
        // After viewport:
        //   v0=(50,50), v2=(50,25), v1=(75,50)
        //   edge1=(0,-25), edge2=(25,0), cross=625 > 0 → backface
        let model = create_test_model(
            &[[0.0, 0.0, 0.0], [0.5, 0.0, 0.0], [0.0, 0.5, 0.0]],
            [0.0, 0.0, 1.0],
            &[[0, 2, 1]],
        );

        let mut buffer = vec![0u32; width * height];
        renderer.render(&model, &shader, &mut buffer, width, height);

        let nonzero_count = buffer.iter().filter(|&&p| p != 0).count();
        assert_eq!(
            nonzero_count, 0,
            "backface triangle should produce no pixels, got {}",
            nonzero_count
        );
    }

    // ── Off-screen triangle ───────────────────────────────────────────

    #[test]
    fn offscreen_triangle_produces_no_pixels() {
        let width = 100;
        let height = 100;
        let renderer = PerTriangleRenderer::new(width, height);
        let shader = test_shader();

        // Vertices far outside NDC range
        // After viewport (100x100):
        //   (5,5) → (300, -200), all way off screen
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
}
