// Copyright The SimpleGameEngine Contributors


//! 瓦片渲染器共享的类型和函数。
//!
//! 由 `TileBasedRenderer` 和 `TileBasedDeferredRenderer` 共同使用。

use crate::color::Color;

use crate::model::Model;
use crate::renderers::base;
use crate::shader::Shader;
use crate::vertex::VertexSoA;


pub const DEFAULT_TILE_SIZE: usize = 64;
pub const K_LANE: usize = 8;
pub const DEPTH_CLEAR: f32 = f32::INFINITY;
pub const COLOR_CLEAR: u32 = 0;

pub struct TileBounds {
    pub screen_x_start: usize,
    pub screen_y_start: usize,
    pub screen_x_end: usize,
    pub screen_y_end: usize,
    pub tile_width: usize,
    pub tile_height: usize,
    pub fb_width: usize,
    pub fb_height: usize,
}


/// 描述瓦片网格和 SoA 顶点数据的不可变上下文。
pub struct TileGridContext {
    pub soa: VertexSoA,
    pub tiles_x: usize,
    pub tiles_y: usize,
    pub tile_size: usize,
}


/// 存储在瓦片三角形列表中的轻量级三角形引用。
pub struct TileTriangleRef {
    pub i0: usize,
    pub i1: usize,
    pub i2: usize,
    pub face_index: usize,
}


/// 将所有顶点通过顶点着色器变换为 SoA 布局。
///
/// 顶点着色器为 `&self`，透视除法和
/// 视口变换会被应用，所有结果存储在 `VertexSoA` 中。
pub fn vertex_transform_soa(
    model: &Model,
    shader: &Shader,
    width: usize,
    height: usize,
) -> VertexSoA {
    let vertices = model.vertices();
    let mut soa = VertexSoA::default();
    soa.resize(vertices.len());

    for (i, v) in vertices.iter().enumerate() {
        let clip = shader.vertex_shader(v);
        soa.pos_clip[i] = clip.position;
        soa.world_pos[i] = clip.world_position;
        let ndc = base::perspective_division(&clip);
        let screen = base::viewport_transform(&ndc, width, height);
        soa.pos_screen[i] = screen.position;
        soa.normal[i] = screen.normal;
        soa.uv[i] = screen.tex_coords;
        soa.color[i] = screen.color;
    }

    soa
}


/// 使用两遍方法（先计数后填充）将三角形分箱到瓦片中。
///
/// 包含视锥体剔除（裁剪空间）和背面剔除（屏幕空间）。
pub fn triangle_tile_binning(model: &Model, grid: &TileGridContext) -> Vec<Vec<TileTriangleRef>> {
    let total_tiles = grid.tiles_x * grid.tiles_y;
    let mut tile_triangles: Vec<Vec<TileTriangleRef>> =
        (0..total_tiles).map(|_| Vec::new()).collect();
    let mut tile_counts = vec![0usize; total_tiles];

    let faces = model.faces();

    // 第一遍：统计每个瓦片的三角形数量
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

    // 预分配
    for tile_id in 0..total_tiles {
        if tile_counts[tile_id] > 0 {
            tile_triangles[tile_id].reserve(tile_counts[tile_id]);
        }
    }

    // 第二遍：填充
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

    // 视锥体剔除（保守的裁剪空间测试）
    let c0 = grid.soa.pos_clip[i0];
    let c1 = grid.soa.pos_clip[i1];
    let c2 = grid.soa.pos_clip[i2];

    let frustum_cull = (c0.x > c0.w && c1.x > c1.w && c2.x > c2.w)
        || (c0.x < -c0.w && c1.x < -c1.w && c2.x < -c2.w)
        || (c0.y > c0.w && c1.y > c1.w && c2.y > c2.w)
        || (c0.y < -c0.w && c1.y < -c1.w && c2.y < -c2.w)
        || (c0.z > c0.w && c1.z > c1.w && c2.z > c2.w)
        || (c0.z < -c0.w && c1.z < -c1.w && c2.z < -c2.w);
    if frustum_cull {
        return;
    }

    let pos0 = grid.soa.pos_screen[i0];
    let pos1 = grid.soa.pos_screen[i1];
    let pos2 = grid.soa.pos_screen[i2];

    // 背面剔除（屏幕空间叉积 > 0 → 背面）
    let edge1_x = pos1.x - pos0.x;
    let edge1_y = pos1.y - pos0.y;
    let edge2_x = pos2.x - pos0.x;
    let edge2_y = pos2.y - pos0.y;
    let cross_product = edge1_x * edge2_y - edge1_y * edge2_x;
    if cross_product > 0.0 {
        return;
    }

    // 计算屏幕空间 AABB
    let min_x = pos0.x.min(pos1.x).min(pos2.x);
    let max_x = pos0.x.max(pos1.x).max(pos2.x);
    let min_y = pos0.y.min(pos1.y).min(pos2.y);
    let max_y = pos0.y.max(pos1.y).max(pos2.y);

    // 查找重叠的瓦片
    let clamped_min_x = (min_x as i32).max(0);
    let clamped_min_y = (min_y as i32).max(0);
    let clamped_max_x = (max_x as i32).max(0);
    let clamped_max_y = (max_y as i32).max(0);
    debug_assert!(
        clamped_min_x >= 0,
        "clamped min_x must be non-negative: {}",
        clamped_min_x
    );
    debug_assert!(
        clamped_min_y >= 0,
        "clamped min_y must be non-negative: {}",
        clamped_min_y
    );
    let start_tile_x = clamped_min_x as usize / grid.tile_size;
    let end_tile_x = (clamped_max_x as usize / grid.tile_size).min(grid.tiles_x.saturating_sub(1));
    let start_tile_y = clamped_min_y as usize / grid.tile_size;
    let end_tile_y = (clamped_max_y as usize / grid.tile_size).min(grid.tiles_y.saturating_sub(1));

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


/// 二维叉积：`ax*by - ay*bx`。
#[inline]
pub fn cross2(ax: f32, ay: f32, bx: f32, by: f32) -> f32 {
    ax * by - ay * bx
}

/// 使用透视校正的重心坐标插值颜色。
///
/// 使用 `Color::from_f32`，其执行 `(val + 0.5) as u8` 舍入。
#[inline]
pub fn interpolate_color_bary(c0: Color, c1: Color, c2: Color, b0: f32, b1: f32, b2: f32) -> Color {
    let r = c0.r() as f32 * b0 + c1.r() as f32 * b1 + c2.r() as f32 * b2;
    let g = c0.g() as f32 * b0 + c1.g() as f32 * b1 + c2.g() as f32 * b2;
    let b = c0.b() as f32 * b0 + c1.b() as f32 * b1 + c2.b() as f32 * b2;
    Color::from_f32(r, g, b, 255.0)
}


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
