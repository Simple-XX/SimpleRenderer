//! Shared vertex post-processing utilities for all rendering strategies.

use crate::math::Vec4;
use crate::vertex::Vertex;

const MIN_W_VALUE: f32 = 1e-6;

/// Perspective division: clip space → NDC.
///
/// Divides x, y, z by w.  Stores `1/w` in the w component for later
/// perspective-correct interpolation.
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
        clip_position: vertex.clip_position,
        world_position: vertex.world_position,
    }
}

/// Viewport transform: NDC `[-1, 1]` → screen coordinates `[0, width/height]`.
///
/// Y is flipped (NDC +Y is up, screen +Y is down).
pub fn viewport_transform(vertex: &Vertex, width: usize, height: usize) -> Vertex {
    let pos = vertex.position;
    let w = width as f32;
    let h = height as f32;

    Vertex {
        position: Vec4::new(
            (pos.x + 1.0) * w / 2.0, // x: [-1,1] → [0, width]
            (1.0 - pos.y) * h / 2.0, // y: [-1,1] → [height, 0] (flipped)
            pos.z,                   // z: preserved for depth testing
            pos.w,                   // w: preserved (1/w from perspective division)
        ),
        normal: vertex.normal,
        tex_coords: vertex.tex_coords,
        color: vertex.color,
        clip_position: vertex.clip_position,
        world_position: vertex.world_position,
    }
}

// ── Tests ──────────────────────────────────────────────────────────────────

#[cfg(test)]
mod tests {
    use super::*;
    use crate::color::Color;
    use crate::math::{Vec2, Vec3};

    /// Helper: create a clip-space vertex at `(x, y, z, w)` with basic
    /// attributes (normal = +Z, uv = (0,0), color = WHITE).
    fn clip_vertex(x: f32, y: f32, z: f32, w: f32) -> Vertex {
        Vertex {
            position: Vec4::new(x, y, z, w),
            normal: Vec3::Z,
            tex_coords: Vec2::ZERO,
            color: Color::WHITE,
            clip_position: Some(Vec4::new(x, y, z, w)),
            world_position: Vec3::ZERO,
        }
    }

    // ── Perspective division ──────────────────────────────────────────

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
        // w near zero should be clamped to MIN_W_VALUE
        let v = clip_vertex(1.0, 2.0, 3.0, 1e-10);
        let ndc = perspective_division(&v);
        // Should not be inf/nan
        assert!(ndc.position.x.is_finite());
        assert!(ndc.position.y.is_finite());
        assert!(ndc.position.z.is_finite());
        assert!(ndc.position.w.is_finite());
    }

    #[test]
    fn perspective_division_negative_w() {
        // Negative w near zero should also be clamped
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
            clip_position: Some(Vec4::new(1.0, 2.0, 3.0, 2.0)),
            world_position: Vec3::ZERO,
        };
        let ndc = perspective_division(&v);
        assert_eq!(ndc.normal, Vec3::Y);
        assert_eq!(ndc.tex_coords, Vec2::new(0.3, 0.7));
        assert_eq!(ndc.color, Color::RED);
        assert_eq!(ndc.clip_position, v.clip_position);
    }

    // ── Viewport transform ────────────────────────────────────────────

    #[test]
    fn viewport_transform_center() {
        // NDC (0, 0) → screen center
        let v = clip_vertex(0.0, 0.0, 0.5, 1.0);
        let screen = viewport_transform(&v, 800, 600);
        assert!((screen.position.x - 400.0).abs() < 1e-4);
        assert!((screen.position.y - 300.0).abs() < 1e-4);
        assert!((screen.position.z - 0.5).abs() < 1e-6); // z preserved
        assert!((screen.position.w - 1.0).abs() < 1e-6); // w preserved
    }

    #[test]
    fn viewport_transform_corners() {
        // NDC (-1, -1) → screen (0, height) — bottom-left in NDC → bottom of screen
        let bl = clip_vertex(-1.0, -1.0, 0.0, 1.0);
        let s = viewport_transform(&bl, 100, 100);
        assert!((s.position.x - 0.0).abs() < 1e-4);
        assert!((s.position.y - 100.0).abs() < 1e-4);

        // NDC (1, 1) → screen (width, 0) — top-right in NDC → top of screen
        let tr = clip_vertex(1.0, 1.0, 0.0, 1.0);
        let s = viewport_transform(&tr, 100, 100);
        assert!((s.position.x - 100.0).abs() < 1e-4);
        assert!((s.position.y - 0.0).abs() < 1e-4);
    }

    #[test]
    fn viewport_transform_y_flip() {
        // NDC +Y is up, screen +Y is down
        // NDC (0, 0.5) should map to screen y < center
        let v = clip_vertex(0.0, 0.5, 0.0, 1.0);
        let s = viewport_transform(&v, 100, 100);
        assert!(
            s.position.y < 50.0,
            "positive NDC y should map above center"
        );

        // NDC (0, -0.5) should map to screen y > center
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
            clip_position: Some(Vec4::new(0.0, 0.0, 0.5, 1.0)),
            world_position: Vec3::ZERO,
        };
        let s = viewport_transform(&v, 200, 200);
        assert_eq!(s.normal, Vec3::X);
        assert_eq!(s.tex_coords, Vec2::new(0.1, 0.9));
        assert_eq!(s.color, Color::BLUE);
    }

    // ── Combined pipeline ─────────────────────────────────────────────

    #[test]
    fn perspective_division_then_viewport() {
        // Clip-space vertex at (1, 1, 1, 2)
        // After perspective division: NDC (0.5, 0.5, 0.5, 0.5)
        // After viewport (100x100): screen ((0.5+1)*50, (1-0.5)*50) = (75, 25)
        let v = clip_vertex(1.0, 1.0, 1.0, 2.0);
        let ndc = perspective_division(&v);
        let screen = viewport_transform(&ndc, 100, 100);
        assert!((screen.position.x - 75.0).abs() < 1e-4);
        assert!((screen.position.y - 25.0).abs() < 1e-4);
    }
}
