use crate::color::Color;
use crate::math::{Vec2, Vec3};

/// A fragment produced by rasterization, ready for fragment shading.
///
/// Note: material is NOT stored here (passed separately for thread safety).
#[derive(Debug, Clone)]
pub struct Fragment {
    pub screen_coord: [i32; 2],
    pub normal: Vec3,
    pub uv: Vec2,
    pub color: Color,
    pub depth: f32,
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn fragment_creation() {
        let f = Fragment {
            screen_coord: [100, 200],
            normal: Vec3::Y,
            uv: Vec2::new(0.5, 0.5),
            color: Color::WHITE,
            depth: 0.5,
        };
        assert_eq!(f.screen_coord, [100, 200]);
        assert_eq!(f.normal, Vec3::Y);
        assert_eq!(f.depth, 0.5);
    }

    #[test]
    fn fragment_clone() {
        let f = Fragment {
            screen_coord: [10, 20],
            normal: Vec3::X,
            uv: Vec2::ZERO,
            color: Color::RED,
            depth: 0.1,
        };
        let f2 = f.clone();
        assert_eq!(f2.screen_coord, f.screen_coord);
        assert_eq!(f2.color, f.color);
    }
}
