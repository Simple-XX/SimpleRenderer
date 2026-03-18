// Copyright The SimpleGameEngine Contributors


use crate::color::Color;
use crate::math::{Mat3, Mat4, Vec2, Vec3, Vec4};

/// 结构体数组（AoS）布局的单个顶点。
#[derive(Debug, Clone, Copy)]
pub struct Vertex {
    pub position: Vec4,
    pub normal: Vec3,
    pub tex_coords: Vec2,
    pub color: Color,
    pub world_position: Vec3,
}

impl Default for Vertex {
    fn default() -> Self {
        Self {
            position: Vec4::ZERO,
            normal: Vec3::ZERO,
            tex_coords: Vec2::ZERO,
            color: Color::default(),
            world_position: Vec3::ZERO,
        }
    }
}

impl Vertex {
    /// 使用给定的属性创建一个新顶点。
    #[inline]
    pub fn new(position: Vec4, normal: Vec3, tex_coords: Vec2, color: Color) -> Self {
        Self {
            position,
            normal,
            tex_coords,
            color,
            world_position: Vec3::ZERO,
        }
    }

    /// 返回此顶点的副本，并设置给定的世界空间位置。
    #[inline]
    pub fn with_world_position(mut self, wp: Vec3) -> Self {
        self.world_position = wp;
        self
    }

    /// 使用 4×4 矩阵变换此顶点。
    ///
    /// 将 `mat` 应用于 `position`，并将 `mat` 左上角 3×3 子矩阵的逆转置应用于 `normal`。
    /// 这能正确处理非均匀缩放。
    /// 同时将 `world_position` 变换为 `(mat * position).truncate()`。
    pub fn transform(&self, mat: &Mat4) -> Vertex {
        let normal_mat = Mat3::from_mat4(*mat).inverse().transpose();
        let transformed_pos = *mat * self.position;
        Vertex {
            position: transformed_pos,
            normal: (normal_mat * self.normal).normalize_or_zero(),
            tex_coords: self.tex_coords,
            color: self.color,
            world_position: transformed_pos.truncate(),
        }
    }
}

/// 数组结构体（SoA）布局的顶点数据，用于缓存友好的
/// 基于图块的渲染。
#[derive(Debug, Clone, Default)]
pub struct VertexSoA {
    pub pos_screen: Vec<Vec4>,
    pub pos_clip: Vec<Vec4>,
    pub normal: Vec<Vec3>,
    pub uv: Vec<Vec2>,
    pub color: Vec<Color>,
    pub world_pos: Vec<Vec3>,
}

impl VertexSoA {
    /// 存储的顶点数量。
    #[inline]
    pub fn len(&self) -> usize {
        self.pos_screen.len()
    }

    /// SoA 是否为空。
    #[inline]
    pub fn is_empty(&self) -> bool {
        self.pos_screen.is_empty()
    }

    /// 将所有数组调整为 `n` 个元素的大小，使用默认值填充。
    pub fn resize(&mut self, n: usize) {
        self.pos_screen.resize(n, Vec4::ZERO);
        self.pos_clip.resize(n, Vec4::ZERO);
        self.normal.resize(n, Vec3::ZERO);
        self.uv.resize(n, Vec2::ZERO);
        self.color.resize(n, Color::default());
        self.world_pos.resize(n, Vec3::ZERO);
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn vertex_default() {
        let v = Vertex::default();
        assert_eq!(v.position, Vec4::ZERO);
        assert_eq!(v.normal, Vec3::ZERO);
        assert_eq!(v.world_position, Vec3::ZERO);
    }

    #[test]
    fn vertex_new() {
        let v = Vertex::new(
            Vec4::new(1.0, 2.0, 3.0, 1.0),
            Vec3::Y,
            Vec2::new(0.5, 0.5),
            Color::RED,
        );
        assert_eq!(v.position, Vec4::new(1.0, 2.0, 3.0, 1.0));
        assert_eq!(v.normal, Vec3::Y);
        assert_eq!(v.color, Color::RED);
        assert_eq!(v.world_position, Vec3::ZERO);
    }

    #[test]
    fn vertex_with_world_position() {
        let v = Vertex::default().with_world_position(Vec3::new(10.0, 20.0, 30.0));
        assert_eq!(v.world_position, Vec3::new(10.0, 20.0, 30.0));
    }

    #[test]
    fn vertex_transform_identity() {
        let v = Vertex::new(
            Vec4::new(1.0, 2.0, 3.0, 1.0),
            Vec3::Y,
            Vec2::ZERO,
            Color::WHITE,
        );
        let transformed = v.transform(&Mat4::IDENTITY);
        assert!((transformed.position - v.position).length() < 1e-6);
        assert!((transformed.normal - v.normal).length() < 1e-6);
        assert!((transformed.world_position - Vec3::new(1.0, 2.0, 3.0)).length() < 1e-6);
    }

    #[test]
    fn vertex_transform_translation() {
        let v = Vertex::new(
            Vec4::new(0.0, 0.0, 0.0, 1.0),
            Vec3::Y,
            Vec2::ZERO,
            Color::WHITE,
        );
        let mat = Mat4::from_translation(Vec3::new(10.0, 20.0, 30.0));
        let t = v.transform(&mat);
        assert!((t.position.x - 10.0).abs() < 1e-6);
        assert!((t.position.y - 20.0).abs() < 1e-6);
        assert!((t.position.z - 30.0).abs() < 1e-6);
        assert!((t.world_position - Vec3::new(10.0, 20.0, 30.0)).length() < 1e-6);
    }

    #[test]
    fn vertex_transform_nonuniform_scale_normal() {
        let v = Vertex::new(
            Vec4::new(0.0, 0.0, 0.0, 1.0),
            Vec3::new(0.0, 1.0, 0.0), // +Y 法线
            Vec2::ZERO,
            Color::WHITE,
        );
        // 非均匀缩放：X 轴拉伸 2 倍，Y 和 Z 保持不变
        let mat = Mat4::from_scale(Vec3::new(2.0, 1.0, 1.0));
        let t = v.transform(&mat);
        // 使用正确的逆转置，法线应保持归一化的 (0, 1, 0)
        // （拉伸 X 不影响 Y 法线）
        assert!(
            (t.normal.y - 1.0).abs() < 1e-5,
            "normal Y should be ~1.0, got {}",
            t.normal.y
        );
        assert!(
            t.normal.x.abs() < 1e-5,
            "normal X should be ~0.0, got {}",
            t.normal.x
        );
    }

    #[test]
    fn vertex_copy() {
        let v = Vertex::default();
        let v2 = v;
        assert_eq!(v.position, v2.position);
    }

    #[test]
    fn soa_default_is_empty() {
        let soa = VertexSoA::default();
        assert_eq!(soa.len(), 0);
        assert!(soa.is_empty());
    }

    #[test]
    fn soa_resize() {
        let mut soa = VertexSoA::default();
        soa.resize(10);
        assert_eq!(soa.len(), 10);
        assert!(!soa.is_empty());
        assert_eq!(soa.pos_screen.len(), 10);
        assert_eq!(soa.pos_clip.len(), 10);
        assert_eq!(soa.normal.len(), 10);
        assert_eq!(soa.uv.len(), 10);
        assert_eq!(soa.color.len(), 10);
        assert_eq!(soa.world_pos.len(), 10);
    }

    #[test]
    fn soa_resize_down() {
        let mut soa = VertexSoA::default();
        soa.resize(10);
        soa.resize(3);
        assert_eq!(soa.len(), 3);
    }
}
