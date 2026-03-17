// Copyright (c) Simple-XX/SimpleRenderer
// SPDX-License-Identifier: MIT

//! 所有渲染策略共享的顶点后处理工具函数。

use crate::math::Vec4;
use crate::vertex::Vertex;

const MIN_W_VALUE: f32 = 1e-6;

/// 透视除法：裁剪空间 → NDC。
///
/// 将 x、y、z 除以 w。将 `1/w` 存储在 w 分量中，
/// 用于后续的透视校正插值。
pub fn perspective_division(vertex: &Vertex) -> Vertex {
    let pos = vertex.position;
    let w = if pos.w.abs() < MIN_W_VALUE {
        MIN_W_VALUE
    } else {
        pos.w
    };
    let inv_w = 1.0 / w;

    Vertex {
        position: Vec4::new(pos.x * inv_w, pos.y * inv_w, pos.z * inv_w, inv_w),
        normal: vertex.normal,
        tex_coords: vertex.tex_coords,
        color: vertex.color,
        world_position: vertex.world_position,
    }
}

/// 视口变换：NDC `[-1, 1]` → 屏幕坐标 `[0, width/height]`。
///
/// Y 轴翻转（NDC 中 +Y 朝上，屏幕中 +Y 朝下）。
pub fn viewport_transform(vertex: &Vertex, width: usize, height: usize) -> Vertex {
    let pos = vertex.position;
    let w = width as f32;
    let h = height as f32;

    Vertex {
        position: Vec4::new(
            (pos.x + 1.0) * w / 2.0, // x: [-1,1] → [0, width]
            (1.0 - pos.y) * h / 2.0, // y: [-1,1] → [height, 0]（翻转）
            pos.z,                   // z: 保留用于深度测试
            pos.w,                   // w: 保留（来自透视除法的 1/w）
        ),
        normal: vertex.normal,
        tex_coords: vertex.tex_coords,
        color: vertex.color,
        world_position: vertex.world_position,
    }
}


#[cfg(test)]
mod tests {
    use super::*;
    use crate::color::Color;
    use crate::math::{Vec2, Vec3};

    /// 辅助函数：创建位于 `(x, y, z, w)` 的裁剪空间顶点，
    /// 附带基本属性（法线 = +Z，uv = (0,0)，颜色 = WHITE）。
    fn clip_vertex(x: f32, y: f32, z: f32, w: f32) -> Vertex {
        Vertex {
            position: Vec4::new(x, y, z, w),
            normal: Vec3::Z,
            tex_coords: Vec2::ZERO,
            color: Color::WHITE,
            world_position: Vec3::ZERO,
        }
    }


    #[test]
    fn perspective_division_basic() {
        let v = clip_vertex(2.0, 4.0, 6.0, 2.0);
        let ndc = perspective_division(&v);
        assert!((ndc.position.x - 1.0).abs() < 1e-6);
        assert!((ndc.position.y - 2.0).abs() < 1e-6);
        assert!((ndc.position.z - 3.0).abs() < 1e-6);
        assert!((ndc.position.w - 0.5).abs() < 1e-6); // 1/w = 0.5
    }

    #[test]
    fn perspective_division_w_one() {
        let v = clip_vertex(0.5, -0.3, 0.8, 1.0);
        let ndc = perspective_division(&v);
        assert!((ndc.position.x - 0.5).abs() < 1e-6);
        assert!((ndc.position.y - (-0.3)).abs() < 1e-6);
        assert!((ndc.position.z - 0.8).abs() < 1e-6);
        assert!((ndc.position.w - 1.0).abs() < 1e-6);
    }

    #[test]
    fn perspective_division_near_zero_w() {
        // 接近零的 w 应被钳制为 MIN_W_VALUE
        let v = clip_vertex(1.0, 2.0, 3.0, 1e-10);
        let ndc = perspective_division(&v);
        // 不应为 inf/nan
        assert!(ndc.position.x.is_finite());
        assert!(ndc.position.y.is_finite());
        assert!(ndc.position.z.is_finite());
        assert!(ndc.position.w.is_finite());
    }

    #[test]
    fn perspective_division_negative_w() {
        // 接近零的负 w 也应被钳制
        let v = clip_vertex(1.0, 2.0, 3.0, -1e-10);
        let ndc = perspective_division(&v);
        assert!(ndc.position.x.is_finite());
        assert!(ndc.position.w.is_finite());
    }

    #[test]
    fn perspective_division_preserves_attributes() {
        let v = Vertex {
            position: Vec4::new(1.0, 2.0, 3.0, 2.0),
            normal: Vec3::Y,
            tex_coords: Vec2::new(0.3, 0.7),
            color: Color::RED,
            world_position: Vec3::ZERO,
        };
        let ndc = perspective_division(&v);
        assert_eq!(ndc.normal, Vec3::Y);
        assert_eq!(ndc.tex_coords, Vec2::new(0.3, 0.7));
        assert_eq!(ndc.color, Color::RED);
    }


    #[test]
    fn viewport_transform_center() {
        // NDC (0, 0) → 屏幕中心
        let v = clip_vertex(0.0, 0.0, 0.5, 1.0);
        let screen = viewport_transform(&v, 800, 600);
        assert!((screen.position.x - 400.0).abs() < 1e-4);
        assert!((screen.position.y - 300.0).abs() < 1e-4);
        assert!((screen.position.z - 0.5).abs() < 1e-6); // z 保留
        assert!((screen.position.w - 1.0).abs() < 1e-6); // w 保留
    }

    #[test]
    fn viewport_transform_corners() {
        // NDC (-1, -1) → 屏幕 (0, height) — NDC 左下角 → 屏幕底部
        let bl = clip_vertex(-1.0, -1.0, 0.0, 1.0);
        let s = viewport_transform(&bl, 100, 100);
        assert!((s.position.x - 0.0).abs() < 1e-4);
        assert!((s.position.y - 100.0).abs() < 1e-4);

        // NDC (1, 1) → 屏幕 (width, 0) — NDC 右上角 → 屏幕顶部
        let tr = clip_vertex(1.0, 1.0, 0.0, 1.0);
        let s = viewport_transform(&tr, 100, 100);
        assert!((s.position.x - 100.0).abs() < 1e-4);
        assert!((s.position.y - 0.0).abs() < 1e-4);
    }

    #[test]
    fn viewport_transform_y_flip() {
        // NDC 中 +Y 朝上，屏幕中 +Y 朝下
        // NDC (0, 0.5) 应映射到屏幕 y < 中心
        let v = clip_vertex(0.0, 0.5, 0.0, 1.0);
        let s = viewport_transform(&v, 100, 100);
        assert!(
            s.position.y < 50.0,
            "positive NDC y should map above center"
        );

        // NDC (0, -0.5) 应映射到屏幕 y > 中心
        let v = clip_vertex(0.0, -0.5, 0.0, 1.0);
        let s = viewport_transform(&v, 100, 100);
        assert!(
            s.position.y > 50.0,
            "negative NDC y should map below center"
        );
    }

    #[test]
    fn viewport_transform_preserves_attributes() {
        let v = Vertex {
            position: Vec4::new(0.0, 0.0, 0.5, 1.0),
            normal: Vec3::X,
            tex_coords: Vec2::new(0.1, 0.9),
            color: Color::BLUE,
            world_position: Vec3::ZERO,
        };
        let s = viewport_transform(&v, 200, 200);
        assert_eq!(s.normal, Vec3::X);
        assert_eq!(s.tex_coords, Vec2::new(0.1, 0.9));
        assert_eq!(s.color, Color::BLUE);
    }


    #[test]
    fn perspective_division_then_viewport() {
        // 裁剪空间顶点 (1, 1, 1, 2)
        // 透视除法后：NDC (0.5, 0.5, 0.5, 0.5)
        // 视口变换后 (100x100)：屏幕 ((0.5+1)*50, (1-0.5)*50) = (75, 25)
        let v = clip_vertex(1.0, 1.0, 1.0, 2.0);
        let ndc = perspective_division(&v);
        let screen = viewport_transform(&ndc, 100, 100);
        assert!((screen.position.x - 75.0).abs() < 1e-4);
        assert!((screen.position.y - 25.0).abs() < 1e-4);
    }
}
