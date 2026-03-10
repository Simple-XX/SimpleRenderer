use crate::color::Color;
use crate::math::Vec3;

/// A light source with position, direction, and color.
#[derive(Debug, Clone)]
pub struct Light {
    pub name: String,
    pub position: Vec3,
    pub direction: Vec3,
    pub color: Color,
}

impl Default for Light {
    fn default() -> Self {
        Self {
            name: String::new(),
            position: Vec3::ZERO,
            direction: Vec3::new(0.0, 0.0, 1.0),
            color: Color::WHITE,
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn default_light() {
        let l = Light::default();
        assert_eq!(l.position, Vec3::ZERO);
        assert_eq!(l.direction, Vec3::new(0.0, 0.0, 1.0));
        assert_eq!(l.color, Color::WHITE);
        assert!(l.name.is_empty());
    }

    #[test]
    fn light_clone() {
        let l = Light {
            name: "sun".to_string(),
            position: Vec3::new(1.0, 2.0, 3.0),
            direction: Vec3::new(0.0, -1.0, 0.0),
            color: Color::RED,
        };
        let l2 = l.clone();
        assert_eq!(l2.name, "sun");
        assert_eq!(l2.position, Vec3::new(1.0, 2.0, 3.0));
        assert_eq!(l2.color, Color::RED);
    }
}
