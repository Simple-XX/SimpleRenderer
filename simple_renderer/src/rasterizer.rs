use rayon::prelude::*;

use crate::color::Color;
use crate::fragment::Fragment;
use crate::math::{Vec2, Vec3};
use crate::vertex::Vertex;

/// Triangle rasterizer.
///
/// Converts screen-space triangles into fragments via barycentric interpolation
/// with perspective-correct attribute interpolation.  The pixel loop is
/// parallelised over rows with `rayon`.
pub struct Rasterizer {
    width: usize,
    height: usize,
}

impl Rasterizer {
    /// Create a rasterizer for a framebuffer of `width × height` pixels.
    pub fn new(width: usize, height: usize) -> Self {
        Self { width, height }
    }

    /// Rasterize a single triangle defined by three screen-space vertices.
    ///
    /// Returns a `Vec<Fragment>` for every pixel that falls inside the
    /// triangle (after clamping to the screen bounds).
    pub fn rasterize(&self, v0: &Vertex, v1: &Vertex, v2: &Vertex) -> Vec<Fragment> {
        // Screen-space XY from the vertex position (already in screen coords).
        let a = Vec2::new(v0.position.x, v0.position.y);
        let b = Vec2::new(v1.position.x, v1.position.y);
        let c = Vec2::new(v2.position.x, v2.position.y);

        // Bounding box.
        let bbox_min_x = a.x.min(b.x).min(c.x);
        let bbox_min_y = a.y.min(b.y).min(c.y);
        let bbox_max_x = a.x.max(b.x).max(c.x);
        let bbox_max_y = a.y.max(b.y).max(c.y);

        // Clamp to screen.
        let x_min = bbox_min_x.max(0.0) as i32;
        let y_min = bbox_min_y.max(0.0) as i32;
        let x_max = bbox_max_x.min((self.width as f32) - 1.0) as i32;
        let y_max = bbox_max_y.min((self.height as f32) - 1.0) as i32;

        if x_min > x_max || y_min > y_max {
            return Vec::new();
        }

        // Positions as Vec3 for barycentric computation.
        let p0 = Vec3::new(v0.position.x, v0.position.y, v0.position.z);
        let p1 = Vec3::new(v1.position.x, v1.position.y, v1.position.z);
        let p2 = Vec3::new(v2.position.x, v2.position.y, v2.position.z);

        // W components for perspective correction.
        let w0 = v0.position.w;
        let w1 = v1.position.w;
        let w2 = v2.position.w;

        // Z components for depth interpolation.
        let z0 = v0.position.z;
        let z1 = v1.position.z;
        let z2 = v2.position.z;

        // Vertex attributes.
        let n0 = v0.normal;
        let n1 = v1.normal;
        let n2 = v2.normal;

        let uv0 = v0.tex_coords;
        let uv1 = v1.tex_coords;
        let uv2 = v2.tex_coords;

        let c0 = v0.color;
        let c1 = v1.color;
        let c2 = v2.color;

        // Parallel over rows.
        let rows: Vec<i32> = (y_min..=y_max).collect();
        rows.par_iter()
            .flat_map(|&y| {
                let mut row_frags = Vec::new();
                for x in x_min..=x_max {
                    let point = Vec3::new(x as f32, y as f32, 0.0);
                    let bary = match get_barycentric_coord(p0, p1, p2, point) {
                        Some(b) => b,
                        None => continue,
                    };

                    let (corrected, depth) =
                        perspective_correction(w0, w1, w2, z0, z1, z2, bary);

                    let normal = interpolate_vec3(n0, n1, n2, corrected);
                    let uv = interpolate_vec2(uv0, uv1, uv2, corrected);
                    let color = interpolate_color(c0, c1, c2, corrected);

                    row_frags.push(Fragment {
                        screen_coord: [x, y],
                        normal,
                        uv,
                        color,
                        depth,
                    });
                }
                row_frags
            })
            .collect()
    }
}

// ── Barycentric coordinates ────────────────────────────────────────────────
//
// Exact port of C++ `Rasterizer::GetBarycentricCoord` (cross-product method).

fn get_barycentric_coord(p0: Vec3, p1: Vec3, p2: Vec3, pa: Vec3) -> Option<Vec3> {
    let v0 = Vec3::new(p2.x - p0.x, p1.x - p0.x, p0.x - pa.x);
    let v1 = Vec3::new(p2.y - p0.y, p1.y - p0.y, p0.y - pa.y);

    let u = v0.cross(v1);

    const EPSILON: f32 = 1e-6;
    if u.z.abs() < EPSILON {
        return None; // degenerate triangle
    }

    let x = 1.0 - (u.x + u.y) / u.z;
    let y = u.y / u.z;
    let z = u.x / u.z;

    if x < 0.0 || y < 0.0 || z < 0.0 || x > 1.0 || y > 1.0 || z > 1.0 {
        return None; // outside triangle
    }

    Some(Vec3::new(x, y, z))
}

// ── Perspective correction ─────────────────────────────────────────────────
//
// Exact port of C++ `Rasterizer::PerformPerspectiveCorrection`.

fn perspective_correction(
    w0: f32,
    w1: f32,
    w2: f32,
    z0: f32,
    z1: f32,
    z2: f32,
    original_bary: Vec3,
) -> (Vec3, f32) {
    let w0_inv = 1.0 / w0;
    let w1_inv = 1.0 / w1;
    let w2_inv = 1.0 / w2;

    let w_inv_interp = interpolate_f32(w0_inv, w1_inv, w2_inv, original_bary);

    let corrected = Vec3::new(
        original_bary.x * w0_inv / w_inv_interp,
        original_bary.y * w1_inv / w_inv_interp,
        original_bary.z * w2_inv / w_inv_interp,
    );

    let z = interpolate_f32(z0, z1, z2, corrected);
    (corrected, z)
}

// ── Interpolation helpers ──────────────────────────────────────────────────

#[inline]
fn interpolate_f32(v0: f32, v1: f32, v2: f32, bary: Vec3) -> f32 {
    v0 * bary.x + v1 * bary.y + v2 * bary.z
}

#[inline]
fn interpolate_vec2(v0: Vec2, v1: Vec2, v2: Vec2, bary: Vec3) -> Vec2 {
    v0 * bary.x + v1 * bary.y + v2 * bary.z
}

#[inline]
fn interpolate_vec3(v0: Vec3, v1: Vec3, v2: Vec3, bary: Vec3) -> Vec3 {
    v0 * bary.x + v1 * bary.y + v2 * bary.z
}

/// Per-channel float interpolation matching C++ `InterpolateColor` +
/// `FloatToUint8_t`: `val + 0.5` then truncate.
#[inline]
fn interpolate_color(c0: Color, c1: Color, c2: Color, bary: Vec3) -> Color {
    let r = c0.r() as f32 * bary.x + c1.r() as f32 * bary.y + c2.r() as f32 * bary.z;
    let g = c0.g() as f32 * bary.x + c1.g() as f32 * bary.y + c2.g() as f32 * bary.z;
    let b = c0.b() as f32 * bary.x + c1.b() as f32 * bary.y + c2.b() as f32 * bary.z;
    Color::from_f32(r, g, b, 255.0)
}

// ── Tests ──────────────────────────────────────────────────────────────────

#[cfg(test)]
mod tests {
    use super::*;
    use crate::math::Vec4;

    /// Helper: create a screen-space vertex at `(x, y, z, w)` with basic
    /// attributes (normal = +Y, uv = (0,0), color = WHITE).
    fn screen_vertex(x: f32, y: f32, z: f32, w: f32) -> Vertex {
        Vertex {
            position: Vec4::new(x, y, z, w),
            normal: Vec3::Y,
            tex_coords: Vec2::ZERO,
            color: Color::WHITE,
            clip_position: None,
        }
    }

    // ── Known triangle ─────────────────────────────────────────────────

    #[test]
    fn known_triangle_produces_fragments() {
        let rast = Rasterizer::new(400, 400);
        let v0 = screen_vertex(100.0, 100.0, 0.5, 1.0);
        let v1 = screen_vertex(200.0, 100.0, 0.5, 1.0);
        let v2 = screen_vertex(150.0, 200.0, 0.5, 1.0);

        let frags = rast.rasterize(&v0, &v1, &v2);
        assert!(!frags.is_empty(), "should produce fragments for a valid triangle");

        // All fragments must lie within the triangle bounding box.
        for f in &frags {
            assert!(f.screen_coord[0] >= 100 && f.screen_coord[0] <= 200);
            assert!(f.screen_coord[1] >= 100 && f.screen_coord[1] <= 200);
        }
    }

    #[test]
    fn known_triangle_fragment_depth() {
        let rast = Rasterizer::new(400, 400);
        let v0 = screen_vertex(100.0, 100.0, 0.5, 1.0);
        let v1 = screen_vertex(200.0, 100.0, 0.5, 1.0);
        let v2 = screen_vertex(150.0, 200.0, 0.5, 1.0);

        let frags = rast.rasterize(&v0, &v1, &v2);
        // With uniform z=0.5 and w=1, all fragments should have depth ≈ 0.5.
        for f in &frags {
            assert!(
                (f.depth - 0.5).abs() < 1e-4,
                "depth {} should be ~0.5",
                f.depth
            );
        }
    }

    // ── Degenerate triangle ────────────────────────────────────────────

    #[test]
    fn degenerate_triangle_returns_empty() {
        let rast = Rasterizer::new(400, 400);
        // Collinear points.
        let v0 = screen_vertex(100.0, 100.0, 0.5, 1.0);
        let v1 = screen_vertex(200.0, 100.0, 0.5, 1.0);
        let v2 = screen_vertex(300.0, 100.0, 0.5, 1.0);

        let frags = rast.rasterize(&v0, &v1, &v2);
        assert!(frags.is_empty(), "collinear points should yield no fragments");
    }

    // ── Barycentric coords ─────────────────────────────────────────────

    #[test]
    fn barycentric_coords_sum_to_one() {
        // Centroid of a triangle is guaranteed to be inside.
        let p0 = Vec3::new(100.0, 100.0, 0.0);
        let p1 = Vec3::new(200.0, 100.0, 0.0);
        let p2 = Vec3::new(150.0, 200.0, 0.0);
        let centroid = Vec3::new(
            (p0.x + p1.x + p2.x) / 3.0,
            (p0.y + p1.y + p2.y) / 3.0,
            0.0,
        );

        let bary = get_barycentric_coord(p0, p1, p2, centroid)
            .expect("centroid should be inside triangle");
        let sum = bary.x + bary.y + bary.z;
        assert!(
            (sum - 1.0).abs() < 1e-4,
            "barycentric sum {} should be ~1.0",
            sum
        );
    }

    #[test]
    fn barycentric_outside_returns_none() {
        let p0 = Vec3::new(100.0, 100.0, 0.0);
        let p1 = Vec3::new(200.0, 100.0, 0.0);
        let p2 = Vec3::new(150.0, 200.0, 0.0);
        // Point clearly outside the triangle.
        let outside = Vec3::new(0.0, 0.0, 0.0);
        assert!(get_barycentric_coord(p0, p1, p2, outside).is_none());
    }

    #[test]
    fn barycentric_degenerate_returns_none() {
        // Collinear points → degenerate.
        let p0 = Vec3::new(0.0, 0.0, 0.0);
        let p1 = Vec3::new(1.0, 0.0, 0.0);
        let p2 = Vec3::new(2.0, 0.0, 0.0);
        let pa = Vec3::new(1.0, 0.0, 0.0);
        assert!(get_barycentric_coord(p0, p1, p2, pa).is_none());
    }

    // ── Off-screen triangle ────────────────────────────────────────────

    #[test]
    fn offscreen_triangle_returns_empty() {
        let rast = Rasterizer::new(400, 400);
        // Entirely to the left of the screen.
        let v0 = screen_vertex(-300.0, 100.0, 0.5, 1.0);
        let v1 = screen_vertex(-200.0, 100.0, 0.5, 1.0);
        let v2 = screen_vertex(-250.0, 200.0, 0.5, 1.0);

        let frags = rast.rasterize(&v0, &v1, &v2);
        assert!(frags.is_empty(), "off-screen triangle should yield no fragments");
    }

    #[test]
    fn offscreen_triangle_below_returns_empty() {
        let rast = Rasterizer::new(400, 400);
        // Entirely below the screen.
        let v0 = screen_vertex(100.0, 500.0, 0.5, 1.0);
        let v1 = screen_vertex(200.0, 500.0, 0.5, 1.0);
        let v2 = screen_vertex(150.0, 600.0, 0.5, 1.0);

        let frags = rast.rasterize(&v0, &v1, &v2);
        assert!(frags.is_empty(), "off-screen triangle should yield no fragments");
    }

    // ── Perspective correction ─────────────────────────────────────────

    #[test]
    fn perspective_correction_uniform_w() {
        // With uniform w, corrected bary should equal original bary.
        let bary = Vec3::new(0.3, 0.4, 0.3);
        let (corrected, _depth) = perspective_correction(1.0, 1.0, 1.0, 0.5, 0.5, 0.5, bary);
        assert!((corrected.x - bary.x).abs() < 1e-5);
        assert!((corrected.y - bary.y).abs() < 1e-5);
        assert!((corrected.z - bary.z).abs() < 1e-5);
    }

    #[test]
    fn perspective_correction_depth_interpolation() {
        let bary = Vec3::new(1.0, 0.0, 0.0);
        let (_corrected, depth) = perspective_correction(1.0, 1.0, 1.0, 0.1, 0.5, 0.9, bary);
        assert!((depth - 0.1).abs() < 1e-5, "depth at v0 should be z0");
    }

    // ── Color interpolation ────────────────────────────────────────────

    #[test]
    fn color_interpolation_uniform() {
        let c = Color::new(100, 150, 200, 255);
        let bary = Vec3::new(0.5, 0.3, 0.2);
        let result = interpolate_color(c, c, c, bary);
        assert_eq!(result.r(), 100);
        assert_eq!(result.g(), 150);
        assert_eq!(result.b(), 200);
        assert_eq!(result.a(), 255);
    }

    #[test]
    fn color_interpolation_at_vertex() {
        let c0 = Color::new(255, 0, 0, 255);
        let c1 = Color::new(0, 255, 0, 255);
        let c2 = Color::new(0, 0, 255, 255);
        // Full weight on v0.
        let bary = Vec3::new(1.0, 0.0, 0.0);
        let result = interpolate_color(c0, c1, c2, bary);
        assert_eq!(result.r(), 255);
        assert_eq!(result.g(), 0);
        assert_eq!(result.b(), 0);
    }

    // ── Interpolation helpers ──────────────────────────────────────────

    #[test]
    fn interpolate_f32_basic() {
        let bary = Vec3::new(0.5, 0.3, 0.2);
        let result = interpolate_f32(10.0, 20.0, 30.0, bary);
        // 10*0.5 + 20*0.3 + 30*0.2 = 5 + 6 + 6 = 17
        assert!((result - 17.0).abs() < 1e-5);
    }

    #[test]
    fn interpolate_vec3_basic() {
        let v0 = Vec3::new(1.0, 0.0, 0.0);
        let v1 = Vec3::new(0.0, 1.0, 0.0);
        let v2 = Vec3::new(0.0, 0.0, 1.0);
        let bary = Vec3::new(1.0 / 3.0, 1.0 / 3.0, 1.0 / 3.0);
        let result = interpolate_vec3(v0, v1, v2, bary);
        assert!((result.x - 1.0 / 3.0).abs() < 1e-5);
        assert!((result.y - 1.0 / 3.0).abs() < 1e-5);
        assert!((result.z - 1.0 / 3.0).abs() < 1e-5);
    }
}
