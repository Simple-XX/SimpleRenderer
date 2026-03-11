//! Math helpers — re-exports glam types and provides convenience functions.

pub use glam::{Mat3, Mat4, Vec2, Vec3, Vec4};

/// Create a right-handed perspective projection matrix (OpenGL convention).
///
/// Matches `glm::perspective` with RH and `[-1, 1]` depth range.
#[inline]
pub fn perspective(fov_y_radians: f32, aspect_ratio: f32, near: f32, far: f32) -> Mat4 {
    Mat4::perspective_rh_gl(fov_y_radians, aspect_ratio, near, far)
}

/// Create a right-handed look-at view matrix.
///
/// Matches `glm::lookAt` with RH convention.
#[inline]
pub fn look_at(eye: Vec3, center: Vec3, up: Vec3) -> Mat4 {
    Mat4::look_at_rh(eye, center, up)
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn perspective_produces_finite_matrix() {
        let m = perspective(std::f32::consts::FRAC_PI_4, 16.0 / 9.0, 0.1, 100.0);
        for col in 0..4 {
            for row in 0..4 {
                assert!(m.col(col)[row].is_finite());
            }
        }
    }

    #[test]
    fn look_at_produces_finite_matrix() {
        let m = look_at(Vec3::new(0.0, 0.0, 5.0), Vec3::ZERO, Vec3::Y);
        for col in 0..4 {
            for row in 0..4 {
                assert!(m.col(col)[row].is_finite());
            }
        }
    }

    #[test]
    fn look_at_at_origin_looking_forward() {
        let m = look_at(Vec3::ZERO, Vec3::new(0.0, 0.0, -1.0), Vec3::Y);
        // The view matrix should be close to identity for this setup
        // (camera at origin looking down -Z is the default OpenGL convention)
        assert!((m - Mat4::IDENTITY).abs_diff_eq(Mat4::ZERO, 1e-6));
    }
}
