//! Tile-based renderer (SoA layout) with edge function rasterization.
//!
//! Port of C++ `TileBasedRenderer` (tile_based_renderer.cpp, 530 lines).
//!
//! Algorithm:
//! 1. Vertex transform to SoA
//! 2. Setup tile grid
//! 3. Triangle-tile binning (2-pass: count then fill)
//! 4. Parallel rasterization per tile (rayon) with edge functions
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

/// Result of rasterizing a single tile.
struct TileResult {
    depth: Vec<f32>,
    color: Vec<u32>,
    screen_x: usize,
    screen_y: usize,
    width: usize,
    height: usize,
}

/// SoA tile-based renderer with edge function rasterization and optional Early-Z.
///
/// Mirrors C++ `TileBasedRenderer`: vertex transform → tile binning →
/// parallel per-tile rasterization → copy to output.
#[allow(dead_code)]
pub struct TileBasedRenderer {
    width: usize,
    height: usize,
    tile_size: usize,
    early_z: bool,
}

impl TileBasedRenderer {
    /// Create a tile-based renderer with the given Early-Z flag and tile size.
    pub fn new(width: usize, height: usize, early_z: bool, tile_size: usize) -> Self {
        Self {
            width,
            height,
            tile_size: if tile_size > 0 {
                tile_size
            } else {
                DEFAULT_TILE_SIZE
            },
            early_z,
        }
    }

    /// Create a tile-based renderer with default tile size (64) and Early-Z enabled.
    pub fn with_options(width: usize, height: usize, tile_size: usize, early_z: bool) -> Self {
        Self::new(width, height, early_z, tile_size)
    }
}

impl Renderer for TileBasedRenderer {
    fn render(
        &self,
        model: &Model,
        shader: &Shader,
        out_buffer: &mut [u32],
        width: usize,
        height: usize,
    ) -> crate::error::Result<()> {
        // 1. Clone shader + prepare caches
        let mut shader = shader.clone();
        shader.prepare_caches();

        let t = Instant::now();
        // 2. Vertex transform to SoA
        let soa = tile_common::vertex_transform_soa(model, &shader, width, height);
        let vertex_ms = t.elapsed().as_secs_f64() * 1000.0;

        let t = Instant::now();
        // 3. Setup tile grid
        let tile_size = self.tile_size;
        let tiles_x = width.div_ceil(tile_size);
        let tiles_y = height.div_ceil(tile_size);

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

        // 5. Global framebuffer
        let num_pixels = width * height;
        let mut global_color = vec![COLOR_CLEAR; num_pixels];
        let mut global_depth = vec![DEPTH_CLEAR; num_pixels];

        let t = Instant::now();
        // 6. Parallel rasterization per tile
        let total_tiles = tiles_x * tiles_y;
        let early_z = self.early_z;

        let tile_results: Vec<TileResult> = (0..total_tiles)
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

                rasterize_tile(
                    &tile_triangles[tile_id],
                    &grid,
                    &mut tile_depth,
                    &mut tile_color,
                    &shader,
                    model.faces(),
                    early_z,
                    screen_x_start,
                    screen_y_start,
                    screen_x_end,
                    screen_y_end,
                    tile_width,
                    width,
                    height,
                );

                TileResult {
                    depth: tile_depth,
                    color: tile_color,
                    screen_x: screen_x_start,
                    screen_y: screen_y_start,
                    width: tile_width,
                    height: tile_height,
                }
            })
            .collect();

        let raster_ms = t.elapsed().as_secs_f64() * 1000.0;

        // 7. Copy tile results to global framebuffer
        let t = Instant::now();
        for tile in &tile_results {
            for y in 0..tile.height {
                let tile_row_off = y * tile.width;
                let global_row_off = (tile.screen_y + y) * width + tile.screen_x;
                global_color[global_row_off..global_row_off + tile.width]
                    .copy_from_slice(&tile.color[tile_row_off..tile_row_off + tile.width]);
                global_depth[global_row_off..global_row_off + tile.width]
                    .copy_from_slice(&tile.depth[tile_row_off..tile_row_off + tile.width]);
            }
        }
        let copy_ms = t.elapsed().as_secs_f64() * 1000.0;

        // 8. Copy to output
        out_buffer[..num_pixels].copy_from_slice(&global_color);

        let sum_ms = vertex_ms + setup_ms + binning_ms + raster_ms + copy_ms;
        if sum_ms > 0.0 {
            debug!("=== TILE-BASED RENDERING PERFORMANCE ===");
            debug!(
                "Vertex Shader:    {:8.3} ms ({:5.1}%)",
                vertex_ms,
                vertex_ms / sum_ms * 100.0
            );
            debug!("Setup:            {:8.3} ms", setup_ms);
            debug!("Binning:          {:8.3} ms", binning_ms);
            debug!("Rasterization:    {:8.3} ms", raster_ms);
            debug!("Copy:             {:8.3} ms", copy_ms);
            debug!("Total:            {:8.3} ms", sum_ms);
            debug!("=========================================");
        }

        Ok(())
    }
}

// ── Per-tile rasterization with edge functions ────────────────────────────

#[allow(clippy::too_many_arguments)]
fn rasterize_tile(
    triangles: &[TileTriangleRef],
    grid: &TileGridContext,
    tile_depth: &mut [f32],
    tile_color: &mut [u32],
    shader: &Shader,
    faces: &[Face],
    use_early_z: bool,
    screen_x_start: usize,
    screen_y_start: usize,
    screen_x_end: usize,
    screen_y_end: usize,
    tile_width: usize,
    width: usize,
    height: usize,
) {
    for tri in triangles {
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
            continue; // degenerate
        }
        let positive = area2 > 0.0;

        // Z and 1/w for perspective correction
        let z0 = p0.z;
        let z1 = p1.z;
        let z2 = p2.z;
        let w0_inv = 1.0 / p0.w;
        let w1_inv = 1.0 / p1.w;
        let w2_inv = 1.0 / p2.w;

        // dE/dx for stepping
        let de01dx = -e01y;
        let de12dx = -e12y;
        let de20dx = -e20y;

        // Row-major scanline
        for y in sy..=ey {
            let yf = y as f32;
            let x0f = sx as f32;

            // Edge function values at scanline start
            let e01_base = cross2(e01x, e01y, x0f - p0.x, yf - p0.y);
            let e12_base = cross2(e12x, e12y, x0f - p1.x, yf - p1.y);
            let e20_base = cross2(e20x, e20y, x0f - p2.x, yf - p2.y);

            let mut xb = sx;
            while xb <= ex {
                let lane = K_LANE.min((ex - xb + 1) as usize);

                // Compute edge functions for this lane
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

                // Compute z and perspective-corrected barycentric coords
                let mut mask_zpass = 0u32;
                let mut zvals = [0.0f32; K_LANE];
                let mut b0c_arr = [0.0f32; K_LANE];
                let mut b1c_arr = [0.0f32; K_LANE];
                let mut b2c_arr = [0.0f32; K_LANE];

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
                    b0c_arr[j] = b0c;
                    b1c_arr[j] = b1c;
                    b2c_arr[j] = b2c;
                    let z = z0 * b0c + z1 * b1c + z2 * b2c;
                    zvals[j] = z;

                    let sx_pix = xb + j as i32;
                    let local_x = (sx_pix - screen_x_start as i32) as usize;
                    let local_y = (y - screen_y_start as i32) as usize;
                    let idx = local_x + local_y * tile_width;
                    if z < tile_depth[idx] {
                        mask_zpass |= 1 << j;
                    }
                }

                // Final mask
                let mask_final = if use_early_z {
                    mask_cover & mask_zpass
                } else {
                    mask_cover
                };
                if mask_final == 0 && use_early_z {
                    xb += K_LANE as i32;
                    continue;
                }

                // Shade and write
                for j in 0..lane {
                    if use_early_z && (mask_final >> j) & 1 == 0 {
                        continue;
                    }
                    if !use_early_z && (mask_cover >> j) & 1 == 0 {
                        continue;
                    }

                    let sx_pix = xb + j as i32;
                    let local_x = (sx_pix - screen_x_start as i32) as usize;
                    let local_y = (y - screen_y_start as i32) as usize;
                    let idx = local_x + local_y * tile_width;

                    let b0c = b0c_arr[j];
                    let b1c = b1c_arr[j];
                    let b2c = b2c_arr[j];

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
                        screen_coord: [sx_pix, y],
                        normal,
                        uv,
                        color,
                        depth: zvals[j],
                        world_position: {
                            let wp0 = grid.soa.world_pos[i0];
                            let wp1 = grid.soa.world_pos[i1];
                            let wp2 = grid.soa.world_pos[i2];
                            wp0 * b0c + wp1 * b1c + wp2 * b2c
                        },
                    };

                    if use_early_z {
                        let out_color =
                            shader.fragment_shader(&frag, &faces[tri.face_index].material);
                        tile_depth[idx] = frag.depth;
                        tile_color[idx] = u32::from(out_color);
                    } else {
                        let out_color =
                            shader.fragment_shader(&frag, &faces[tri.face_index].material);
                        if frag.depth < tile_depth[idx] {
                            tile_depth[idx] = frag.depth;
                            tile_color[idx] = u32::from(out_color);
                        }
                    }
                }

                xb += K_LANE as i32;
            }
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

        static COUNTER: AtomicUsize = AtomicUsize::new(2000);
        let id = COUNTER.fetch_add(1, Ordering::SeqCst);
        let path = std::env::temp_dir().join(format!("simple_renderer_tilebased_test_{}.obj", id));

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
        let renderer = TileBasedRenderer::new(width, height, true, DEFAULT_TILE_SIZE);
        let shader = test_shader();

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

    // ── Backface culling ──────────────────────────────────────────────

    #[test]
    fn backface_triangle_produces_no_pixels() {
        let width = 100;
        let height = 100;
        let renderer = TileBasedRenderer::new(width, height, true, DEFAULT_TILE_SIZE);
        let shader = test_shader();

        let model = create_test_model(
            &[[0.0, 0.0, 0.0], [0.5, 0.0, 0.0], [0.0, 0.5, 0.0]],
            [0.0, 0.0, 1.0],
            &[[0, 2, 1]], // reversed winding
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

    // ── Empty model ──────────────────────────────────────────────────

    #[test]
    fn empty_model_produces_no_pixels() {
        let width = 100;
        let height = 100;
        let renderer = TileBasedRenderer::new(width, height, true, DEFAULT_TILE_SIZE);
        let shader = test_shader();

        let model = create_test_model(
            &[[0.0, 0.0, 0.0], [0.5, 0.0, 0.0], [0.0, 0.5, 0.0]],
            [0.0, 0.0, 1.0],
            &[],
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

    // ── Early-Z flag ─────────────────────────────────────────────────

    #[test]
    fn early_z_disabled_still_renders() {
        let width = 100;
        let height = 100;
        let renderer = TileBasedRenderer::with_options(width, height, 32, false);
        let shader = test_shader();

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
            "early-z disabled should still render, got {}",
            nonzero_count
        );
    }

    // ── Off-screen triangle ──────────────────────────────────────────

    #[test]
    fn offscreen_triangle_produces_no_pixels() {
        let width = 100;
        let height = 100;
        let renderer = TileBasedRenderer::new(width, height, true, DEFAULT_TILE_SIZE);
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

    // ── Custom tile size ─────────────────────────────────────────────

    #[test]
    fn custom_tile_size_works() {
        let width = 100;
        let height = 100;
        let renderer = TileBasedRenderer::with_options(width, height, 16, true);
        let shader = test_shader();

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
            "small tile size should still render, got {}",
            nonzero_count
        );
    }
}
