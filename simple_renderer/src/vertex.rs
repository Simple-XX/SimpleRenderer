use crate::color::Color;
use crate::math::{Mat3, Mat4, Vec2, Vec3, Vec4};

/// A single vertex in Array-of-Structures (AoS) layout.
#[derive(Debug, Clone, Copy)]
pub struct Vertex {
    pub position: Vec4,
    pub normal: Vec3,
    pub tex_coords: Vec2,
    pub color: Color,
    pub clip_position: Option<Vec4>,
    pub world_position: Vec3,
}

impl Default for Vertex {
    fn default() -> Self {
        Self {
            position: Vec4::ZERO,
            normal: Vec3::ZERO,
            tex_coords: Vec2::ZERO,
            color: Color::default(),
            clip_position: None,
            world_position: Vec3::ZERO,
        }
    }
}

impl Vertex {
    /// Create a new vertex with the given attributes. `clip_position` is `None`.
    #[inline]
    pub fn new(position: Vec4, normal: Vec3, tex_coords: Vec2, color: Color) -> Self {
        Self {
            position,
            normal,
            tex_coords,
            color,
            clip_position: None,
            world_position: Vec3::ZERO,
        }
    }

    /// Return a copy of this vertex with the given clip-space position set.
    #[inline]
    pub fn with_clip_position(mut self, clip: Vec4) -> Self {
        self.clip_position = Some(clip);
        self
    }

    /// Return a copy of this vertex with the given world-space position set.
    #[inline]
    pub fn with_world_position(mut self, wp: Vec3) -> Self {
        self.world_position = wp;
        self
    }

    /// Transform this vertex by a 4×4 matrix.
    ///
    /// Applies `mat` to `position` and the upper-left 3×3 of `mat` to `normal`.
    /// Also transforms `world_position` as `(mat * position).truncate()`.
    pub fn transform(&self, mat: &Mat4) -> Vertex {
        let normal_mat = Mat3::from_cols(
            mat.col(0).truncate(),
            mat.col(1).truncate(),
            mat.col(2).truncate(),
        );
        let transformed_pos = *mat * self.position;
        Vertex {
            position: transformed_pos,
            normal: (normal_mat * self.normal).normalize_or_zero(),
            tex_coords: self.tex_coords,
            color: self.color,
            clip_position: self.clip_position,
            world_position: transformed_pos.truncate(),
        }
    }
}

/// Vertex data in Structure-of-Arrays (SoA) layout for cache-friendly
/// tile-based rendering.
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
    /// Number of vertices stored.
    #[inline]
    pub fn len(&self) -> usize {
        self.pos_screen.len()
    }

    /// Whether the SoA is empty.
    #[inline]
    pub fn is_empty(&self) -> bool {
        self.pos_screen.is_empty()
    }

    /// Resize all arrays to hold `n` elements, using default values.
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
        assert!(v.clip_position.is_none());
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
        assert!(v.clip_position.is_none());
        assert_eq!(v.world_position, Vec3::ZERO);
    }

    #[test]
    fn vertex_with_clip_position() {
        let v = Vertex::default().with_clip_position(Vec4::new(1.0, 2.0, 3.0, 4.0));
        assert_eq!(v.clip_position, Some(Vec4::new(1.0, 2.0, 3.0, 4.0)));
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
