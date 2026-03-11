//! Tile-based deferred renderer (TBDR) with 2-pass rasterization.
//!
//! Port of C++ `TileBasedDeferredRenderer` (tile_based_deferred_renderer.cpp, 435 lines).
//!
//! Algorithm:
//! 1. Vertex transform to SoA
//! 2. Setup tile grid
//! 3. Triangle-tile binning (2-pass: count then fill)
//! 4. Per-tile 2-pass rasterization (rayon):
//!    - Pass A (Z-prepass): edge function rasterization, update zmin + winner
//!    - Pass B (Deferred shade): shade only winner fragments
//! 5. Copy tile buffers to global framebuffer
//! 6. Copy to output

use log::debug;
use std::time::Instant;

use rayon::prelude::*;

use crate::face::Face;
use crate::fragment::Fragment;
use crate::model::Model;
use crate::renderers::tile_common::{
    self, cross2, interpolate_color_bary, TileGridContext, TileTriangleRef, COLOR_CLEAR,
    DEFAULT_TILE_SIZE, DEPTH_CLEAR, K_LANE,
};
use crate::renderers::Renderer;
use crate::shader::Shader;

/// SoA tile-based deferred renderer (TBDR).
///
/// 2-pass per tile: Z-prepass determines winners, then shade only winners.
/// This avoids shading pixels that would be overdrawn by closer triangles.
#[allow(dead_code)]
pub struct TileBasedDeferredRenderer {
    width: usize,
    height: usize,
    tile_size: usize,
}

impl TileBasedDeferredRenderer {
    /// Create a TBDR renderer with the given tile size.
    pub fn new(width: usize, height: usize, tile_size: usize) -> Self {
        Self {
            width,
            height,
            tile_size: if tile_size > 0 {
                tile_size
            } else {
                DEFAULT_TILE_SIZE
            },
        }
    }

    /// Create a TBDR renderer with custom tile size (alias for `new`).
    pub fn with_tile_size(width: usize, height: usize, tile_size: usize) -> Self {
        Self::new(width, height, tile_size)
    }
}

impl Renderer for TileBasedDeferredRenderer {
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
        // 2. Vertex transform to SoA
        let soa = tile_common::vertex_transform_soa(model, &mut shader, width, height);
        let vertex_ms = t.elapsed().as_secs_f64() * 1000.0;

        let t = Instant::now();
        // 3. Setup tile grid
        let tile_size = self.tile_size;
        let tiles_x = (width + tile_size - 1) / tile_size;
        let tiles_y = (height + tile_size - 1) / tile_size;

        let grid = TileGridContext {
            soa,
            tiles_x,
            tiles_y,
            tile_size,
            width,
            height,
        };
        let setup_ms = t.elapsed().as_secs_f64() * 1000.0;

        let t = Instant::now();
        // 4. Triangle-tile binning
        let tile_triangles = tile_common::triangle_tile_binning(model, &grid);
        let binning_ms = t.elapsed().as_secs_f64() * 1000.0;

        let t = Instant::now();
        // 6. Parallel 2-pass rasterization per tile
        let total_tiles = tiles_x * tiles_y;

        let tile_results: Vec<(Vec<u32>, usize, usize, usize, usize)> = (0..total_tiles)
            .into_par_iter()
            .map(|tile_id| {
                let tile_x = tile_id % tiles_x;
                let tile_y = tile_id / tiles_x;
                let screen_x_start = tile_x * tile_size;
                let screen_y_start = tile_y * tile_size;
                let screen_x_end = (screen_x_start + tile_size).min(width);
                let screen_y_end = (screen_y_start + tile_size).min(height);
                let tile_width = screen_x_end - screen_x_start;
                let tile_height = screen_y_end - screen_y_start;

                let mut tile_depth = vec![DEPTH_CLEAR; tile_width * tile_height];
                let mut tile_color = vec![COLOR_CLEAR; tile_width * tile_height];

                rasterize_tile_deferred(
                    &tile_triangles[tile_id],
                    &grid,
                    &mut tile_depth,
                    &mut tile_color,
                    &shader,
                    model.faces(),
                    screen_x_start,
                    screen_y_start,
                    screen_x_end,
                    screen_y_end,
                    tile_width,
                    tile_height,
                    width,
                    height,
                );

                // tile_depth is only used as z-buffer within
                // rasterize_tile_deferred — no need to return it
                (
                    tile_color,
                    screen_x_start,
                    screen_y_start,
                    tile_width,
                    tile_height,
                )
            })
            .collect();

        let raster_ms = t.elapsed().as_secs_f64() * 1000.0;

        // 7. Copy tile results directly to output buffer
        let t = Instant::now();
        for (tile_color, sx, sy, tw, th) in &tile_results {
            for y in 0..*th {
                let tile_row_off = y * tw;
                let out_row_off = (sy + y) * width + sx;
                out_buffer[out_row_off..out_row_off + tw]
                    .copy_from_slice(&tile_color[tile_row_off..tile_row_off + tw]);
            }
        }
        let copy_ms = t.elapsed().as_secs_f64() * 1000.0;

        let sum_ms = vertex_ms + setup_ms + binning_ms + raster_ms + copy_ms;
        if sum_ms > 0.0 {
            debug!("=== TILE-BASED DEFERRED RENDERING PERFORMANCE ===");
            debug!(
                "Vertex Shader:    {:8.3} ms ({:5.1}%)",
                vertex_ms,
                vertex_ms / sum_ms * 100.0
            );
            debug!("Setup:            {:8.3} ms", setup_ms);
            debug!("Binning:          {:8.3} ms", binning_ms);
            debug!("Tile Raster:      {:8.3} ms", raster_ms);
            debug!("Copy:             {:8.3} ms", copy_ms);
            debug!("Total:            {:8.3} ms", sum_ms);
            debug!("=================================================");
        }

        true
    }
}

// ── Per-tile 2-pass rasterization ─────────────────────────────────────────

#[allow(clippy::too_many_arguments)]
fn rasterize_tile_deferred(
    triangles: &[TileTriangleRef],
    grid: &TileGridContext,
    tile_depth: &mut [f32],
    tile_color: &mut [u32],
    shader: &Shader,
    faces: &[Face],
    screen_x_start: usize,
    screen_y_start: usize,
    screen_x_end: usize,
    screen_y_end: usize,
    tile_width: usize,
    tile_height: usize,
    width: usize,
    height: usize,
) {
    let tile_pixels = tile_width * tile_height;

    // Per-pixel state for 2-pass
    // tile_depth is used as zmin buffer (Pass A) and output depth (Pass B)
    let mut winner: Vec<i32> = vec![-1; tile_pixels];
    let mut b0c_buf = vec![0.0f32; tile_pixels];
    let mut b1c_buf = vec![0.0f32; tile_pixels];

    // ── Pass A: Z-prepass ─────────────────────────────────────────────

    for (tri_local_idx, tri) in triangles.iter().enumerate() {
        let i0 = tri.i0;
        let i1 = tri.i1;
        let i2 = tri.i2;

        let p0 = grid.soa.pos_screen[i0];
        let p1 = grid.soa.pos_screen[i1];
        let p2 = grid.soa.pos_screen[i2];

        // Triangle AABB clipped to tile bounds
        let tri_minx = p0.x.min(p1.x).min(p2.x);
        let tri_miny = p0.y.min(p1.y).min(p2.y);
        let tri_maxx = p0.x.max(p1.x).max(p2.x);
        let tri_maxy = p0.y.max(p1.y).max(p2.y);

        let sx = (screen_x_start as i32).max(tri_minx.max(0.0).floor() as i32);
        let sy = (screen_y_start as i32).max(tri_miny.max(0.0).floor() as i32);
        let ex = ((screen_x_end - 1) as i32).min(tri_maxx.min((width - 1) as f32).floor() as i32);
        let ey = ((screen_y_end - 1) as i32).min(tri_maxy.min((height - 1) as f32).floor() as i32);

        if sx > ex || sy > ey {
            continue;
        }

        // Edge vectors and signed area
        let e01x = p1.x - p0.x;
        let e01y = p1.y - p0.y;
        let e12x = p2.x - p1.x;
        let e12y = p2.y - p1.y;
        let e20x = p0.x - p2.x;
        let e20y = p0.y - p2.y;
        let area2 = cross2(e01x, e01y, p2.x - p0.x, p2.y - p0.y);
        if area2.abs() < 1e-6 {
            continue;
        }
        let positive = area2 > 0.0;

        let z0 = p0.z;
        let z1 = p1.z;
        let z2 = p2.z;
        let w0_inv = 1.0 / p0.w;
        let w1_inv = 1.0 / p1.w;
        let w2_inv = 1.0 / p2.w;

        let de01dx = -e01y;
        let de12dx = -e12y;
        let de20dx = -e20y;

        for y in sy..=ey {
            let yf = y as f32;
            let x0f = sx as f32;

            let e01_base = cross2(e01x, e01y, x0f - p0.x, yf - p0.y);
            let e12_base = cross2(e12x, e12y, x0f - p1.x, yf - p1.y);
            let e20_base = cross2(e20x, e20y, x0f - p2.x, yf - p2.y);

            let mut xb = sx;
            while xb <= ex {
                let lane = K_LANE.min((ex - xb + 1) as usize);

                let mut e01 = [0.0f32; K_LANE];
                let mut e12 = [0.0f32; K_LANE];
                let mut e20 = [0.0f32; K_LANE];
                for j in 0..lane {
                    let step = (xb - sx + j as i32) as f32;
                    e01[j] = e01_base + de01dx * step;
                    e12[j] = e12_base + de12dx * step;
                    e20[j] = e20_base + de20dx * step;
                }

                // Coverage mask
                let mut mask_cover = 0u32;
                for j in 0..lane {
                    let inside = if positive {
                        e01[j] >= 0.0 && e12[j] >= 0.0 && e20[j] >= 0.0
                    } else {
                        e01[j] <= 0.0 && e12[j] <= 0.0 && e20[j] <= 0.0
                    };
                    if inside {
                        mask_cover |= 1 << j;
                    }
                }

                if mask_cover == 0 {
                    xb += K_LANE as i32;
                    continue;
                }

                // Z-test: update zmin and winner
                for j in 0..lane {
                    if (mask_cover >> j) & 1 == 0 {
                        continue;
                    }
                    let b0 = e12[j] / area2;
                    let b1 = e20[j] / area2;
                    let b2 = e01[j] / area2;
                    let w_inv = b0 * w0_inv + b1 * w1_inv + b2 * w2_inv;
                    let b0c = (b0 * w0_inv) / w_inv;
                    let b1c = (b1 * w1_inv) / w_inv;
                    let b2c = (b2 * w2_inv) / w_inv;
                    let z = z0 * b0c + z1 * b1c + z2 * b2c;

                    let sx_pix = xb + j as i32;
                    let local_x = (sx_pix - screen_x_start as i32) as usize;
                    let local_y = (y - screen_y_start as i32) as usize;
                    let idx = local_x + local_y * tile_width;

                    if z < tile_depth[idx] - 1e-8 {
                        tile_depth[idx] = z;
                        winner[idx] = tri_local_idx as i32;
                        b0c_buf[idx] = b0c;
                        b1c_buf[idx] = b1c;
                    }
                }

                xb += K_LANE as i32;
            }
        }
    }

    // ── Pass B: Deferred shade (winners only) ─────────────────────────

    for y in 0..tile_height {
        for x in 0..tile_width {
            let idx = x + y * tile_width;
            let win = winner[idx];
            if win < 0 {
                continue;
            }

            let tri = &triangles[win as usize];
            let i0 = tri.i0;
            let i1 = tri.i1;
            let i2 = tri.i2;
            let b0c = b0c_buf[idx];
            let b1c = b1c_buf[idx];
            let b2c = 1.0 - b0c - b1c;

            // Interpolate attributes
            let n0 = grid.soa.normal[i0];
            let n1 = grid.soa.normal[i1];
            let n2 = grid.soa.normal[i2];
            let normal = n0 * b0c + n1 * b1c + n2 * b2c;

            let uv0 = grid.soa.uv[i0];
            let uv1 = grid.soa.uv[i1];
            let uv2 = grid.soa.uv[i2];
            let uv = uv0 * b0c + uv1 * b1c + uv2 * b2c;

            let c0 = grid.soa.color[i0];
            let c1 = grid.soa.color[i1];
            let c2 = grid.soa.color[i2];
            let color = interpolate_color_bary(c0, c1, c2, b0c, b1c, b2c);

            let frag = Fragment {
                screen_coord: [(screen_x_start + x) as i32, (screen_y_start + y) as i32],
                normal,
                uv,
                color,
                depth: tile_depth[idx],
            };

            let out_color = shader.fragment_shader(&frag, &faces[tri.face_index].material);
            // tile_depth[idx] already set in Pass A
            tile_color[idx] = u32::from(out_color);
        }
    }
}

// ── Tests ──────────────────────────────────────────────────────────────────

#[cfg(test)]
mod tests {
    use super::*;
    use crate::color::Color;
    use crate::light::Light;
    use crate::math::{Mat4, Vec3};

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

    fn create_test_model(
        positions: &[[f32; 3]],
        normal: [f32; 3],
        face_indices: &[[usize; 3]],
    ) -> crate::model::Model {
        use std::io::Write;
        use std::sync::atomic::{AtomicUsize, Ordering};

        static COUNTER: AtomicUsize = AtomicUsize::new(3000);
        let id = COUNTER.fetch_add(1, Ordering::SeqCst);
        let path = std::env::temp_dir().join(format!("simple_renderer_tbdr_test_{}.obj", id));

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
        let renderer = TileBasedDeferredRenderer::new(width, height, DEFAULT_TILE_SIZE);
        let shader = test_shader();

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
        let renderer = TileBasedDeferredRenderer::new(width, height, DEFAULT_TILE_SIZE);
        let shader = test_shader();

        let model = create_test_model(
            &[[0.0, 0.0, 0.0], [0.5, 0.0, 0.0], [0.0, 0.5, 0.0]],
            [0.0, 0.0, 1.0],
            &[[0, 2, 1]], // reversed winding
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

    // ── Empty model ──────────────────────────────────────────────────

    #[test]
    fn empty_model_produces_no_pixels() {
        let width = 100;
        let height = 100;
        let renderer = TileBasedDeferredRenderer::new(width, height, DEFAULT_TILE_SIZE);
        let shader = test_shader();

        let model = create_test_model(
            &[[0.0, 0.0, 0.0], [0.5, 0.0, 0.0], [0.0, 0.5, 0.0]],
            [0.0, 0.0, 1.0],
            &[],
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
        let renderer = TileBasedDeferredRenderer::new(width, height, DEFAULT_TILE_SIZE);
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

    // ── Custom tile size ─────────────────────────────────────────────

    #[test]
    fn custom_tile_size_works() {
        let width = 100;
        let height = 100;
        let renderer = TileBasedDeferredRenderer::with_tile_size(width, height, 16);
        let shader = test_shader();

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
            "small tile size should still render, got {}",
            nonzero_count
        );
    }

    // ── 2-pass advantage: only winners get shaded ────────────────────

    #[test]
    fn two_pass_renders_overlapping_triangles() {
        let width = 100;
        let height = 100;
        let renderer = TileBasedDeferredRenderer::new(width, height, DEFAULT_TILE_SIZE);
        let shader = test_shader();

        // Two overlapping triangles at different depths
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
