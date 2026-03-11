//! Shared types and functions for tile-based renderers.
//!
//! Used by both `TileBasedRenderer` and `TileBasedDeferredRenderer`.

use crate::color::Color;

use crate::model::Model;
use crate::renderers::base;
use crate::shader::Shader;
use crate::vertex::VertexSoA;

// ── Constants ─────────────────────────────────────────────────────────────

pub const DEFAULT_TILE_SIZE: usize = 64;
pub const K_LANE: usize = 8;
pub const DEPTH_CLEAR: f32 = f32::INFINITY;
pub const COLOR_CLEAR: u32 = 0;

// ── Tile grid context ─────────────────────────────────────────────────────

/// Immutable context describing the tile grid and SoA vertex data.
pub struct TileGridContext {
    pub soa: VertexSoA,
    pub tiles_x: usize,
    pub tiles_y: usize,
    pub tile_size: usize,
    pub width: usize,
    pub height: usize,
}

// ── Triangle reference for tile binning ───────────────────────────────────

/// A lightweight reference to a triangle stored in a tile's triangle list.
///
/// Mirrors C++ `TileTriangleRef`.
pub struct TileTriangleRef {
    pub i0: usize,
    pub i1: usize,
    pub i2: usize,
    pub face_index: usize,
}

// ── Vertex transform to SoA ──────────────────────────────────────────────

/// Transform all vertices through the vertex shader into SoA layout.
///
/// Vertex shader is sequential (`&mut self`), perspective division and
/// viewport transform are applied, and all results are stored in `VertexSoA`.
pub fn vertex_transform_soa(
    model: &Model,
    shader: &mut Shader,
    width: usize,
    height: usize,
) -> VertexSoA {
    let vertices = model.vertices();
    let mut soa = VertexSoA::default();
    soa.resize(vertices.len());

    for (i, v) in vertices.iter().enumerate() {
        let clip = shader.vertex_shader(v);
        soa.pos_clip[i] = clip.position;
        let ndc = base::perspective_division(&clip);
        let screen = base::viewport_transform(&ndc, width, height);
        soa.pos_screen[i] = screen.position;
        soa.normal[i] = screen.normal;
        soa.uv[i] = screen.tex_coords;
        soa.color[i] = screen.color;
    }

    soa
}

// ── Triangle-tile binning ─────────────────────────────────────────────────

/// Bin triangles into tiles using a 2-pass approach (count then fill).
///
/// Includes frustum culling (clip space) and backface culling (screen space).
pub fn triangle_tile_binning(model: &Model, grid: &TileGridContext) -> Vec<Vec<TileTriangleRef>> {
    let total_tiles = grid.tiles_x * grid.tiles_y;
    let mut tile_triangles: Vec<Vec<TileTriangleRef>> =
        (0..total_tiles).map(|_| Vec::new()).collect();
    let mut tile_counts = vec![0usize; total_tiles];

    let faces = model.faces();

    // Pass 1: count triangles per tile
    for (tri_idx, face) in faces.iter().enumerate() {
        process_triangle_for_binning(
            tri_idx,
            face,
            true,
            grid,
            &mut tile_counts,
            &mut tile_triangles,
        );
    }

    // Pre-allocate
    for tile_id in 0..total_tiles {
        if tile_counts[tile_id] > 0 {
            tile_triangles[tile_id].reserve(tile_counts[tile_id]);
        }
    }

    // Pass 2: fill
    for (tri_idx, face) in faces.iter().enumerate() {
        process_triangle_for_binning(
            tri_idx,
            face,
            false,
            grid,
            &mut tile_counts,
            &mut tile_triangles,
        );
    }

    tile_triangles
}

fn process_triangle_for_binning(
    tri_idx: usize,
    face: &crate::face::Face,
    count_only: bool,
    grid: &TileGridContext,
    tile_counts: &mut [usize],
    tile_triangles: &mut [Vec<TileTriangleRef>],
) {
    let i0 = face.indices[0];
    let i1 = face.indices[1];
    let i2 = face.indices[2];

    // Frustum culling (conservative clip-space test)
    let c0 = grid.soa.pos_clip[i0];
    let c1 = grid.soa.pos_clip[i1];
    let c2 = grid.soa.pos_clip[i2];

    let frustum_cull = (c0.x > c0.w && c1.x > c1.w && c2.x > c2.w)
        || (c0.x < -c0.w && c1.x < -c0.w && c2.x < -c0.w)
        || (c0.y > c0.w && c1.y > c1.w && c2.y > c2.w)
        || (c0.y < -c0.w && c1.y < -c0.w && c2.y < -c0.w)
        || (c0.z > c0.w && c1.z > c1.w && c2.z > c2.w)
        || (c0.z < -c0.w && c1.z < -c0.w && c2.z < -c0.w);
    if frustum_cull {
        return;
    }

    let pos0 = grid.soa.pos_screen[i0];
    let pos1 = grid.soa.pos_screen[i1];
    let pos2 = grid.soa.pos_screen[i2];

    // Backface culling (screen-space cross product > 0 → backface)
    let edge1_x = pos1.x - pos0.x;
    let edge1_y = pos1.y - pos0.y;
    let edge2_x = pos2.x - pos0.x;
    let edge2_y = pos2.y - pos0.y;
    let cross_product = edge1_x * edge2_y - edge1_y * edge2_x;
    if cross_product > 0.0 {
        return;
    }

    // Compute screen-space AABB
    let min_x = pos0.x.min(pos1.x).min(pos2.x);
    let max_x = pos0.x.max(pos1.x).max(pos2.x);
    let min_y = pos0.y.min(pos1.y).min(pos2.y);
    let max_y = pos0.y.max(pos1.y).max(pos2.y);

    // Find overlapping tiles
    let start_tile_x = (min_x as i32).max(0) as usize / grid.tile_size;
    let end_tile_x =
        ((max_x as i32).max(0) as usize / grid.tile_size).min(grid.tiles_x.saturating_sub(1));
    let start_tile_y = (min_y as i32).max(0) as usize / grid.tile_size;
    let end_tile_y =
        ((max_y as i32).max(0) as usize / grid.tile_size).min(grid.tiles_y.saturating_sub(1));

    if start_tile_x > end_tile_x || start_tile_y > end_tile_y {
        return;
    }

    if count_only {
        for ty in start_tile_y..=end_tile_y {
            for tx in start_tile_x..=end_tile_x {
                let tile_id = ty * grid.tiles_x + tx;
                tile_counts[tile_id] += 1;
            }
        }
    } else {
        for ty in start_tile_y..=end_tile_y {
            for tx in start_tile_x..=end_tile_x {
                let tile_id = ty * grid.tiles_x + tx;
                tile_triangles[tile_id].push(TileTriangleRef {
                    i0,
                    i1,
                    i2,
                    face_index: tri_idx,
                });
            }
        }
    }
}

// ── Edge function helpers ─────────────────────────────────────────────────

/// 2D cross product: `ax*by - ay*bx`.
#[inline]
pub fn cross2(ax: f32, ay: f32, bx: f32, by: f32) -> f32 {
    ax * by - ay * bx
}

/// Interpolate color using perspective-corrected barycentric coordinates.
///
/// Uses `Color::from_f32` which does `(val + 0.5) as u8` rounding,
/// matching C++ `FloatToUint8_t`.
#[inline]
pub fn interpolate_color_bary(c0: Color, c1: Color, c2: Color, b0: f32, b1: f32, b2: f32) -> Color {
    let r = c0.r() as f32 * b0 + c1.r() as f32 * b1 + c2.r() as f32 * b2;
    let g = c0.g() as f32 * b0 + c1.g() as f32 * b1 + c2.g() as f32 * b2;
    let b = c0.b() as f32 * b0 + c1.b() as f32 * b1 + c2.b() as f32 * b2;
    Color::from_f32(r, g, b, 255.0)
}

// ── Tests ─────────────────────────────────────────────────────────────────

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn cross2_basic() {
        assert!((cross2(1.0, 0.0, 0.0, 1.0) - 1.0).abs() < 1e-6);
        assert!((cross2(0.0, 1.0, 1.0, 0.0) - (-1.0)).abs() < 1e-6);
    }

    #[test]
    fn interpolate_color_uniform() {
        let c = Color::new(100, 150, 200, 255);
        let result = interpolate_color_bary(c, c, c, 0.5, 0.3, 0.2);
        assert_eq!(result.r(), 100);
        assert_eq!(result.g(), 150);
        assert_eq!(result.b(), 200);
    }

    #[test]
    fn interpolate_color_at_vertex() {
        let c0 = Color::new(255, 0, 0, 255);
        let c1 = Color::new(0, 255, 0, 255);
        let c2 = Color::new(0, 0, 255, 255);
        let result = interpolate_color_bary(c0, c1, c2, 1.0, 0.0, 0.0);
        assert_eq!(result.r(), 255);
        assert_eq!(result.g(), 0);
        assert_eq!(result.b(), 0);
    }
}
