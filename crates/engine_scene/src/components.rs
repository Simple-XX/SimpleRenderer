use engine_core::Handle;
use glam::Vec3;

pub struct Name(pub String);

pub struct MeshRenderer {
    pub model_handle: Handle<engine_render_sw::Model>,
}

pub struct CameraComponent {
    pub fov: f32,
    pub near: f32,
    pub far: f32,
}

impl Default for CameraComponent {
    fn default() -> Self {
        Self {
            fov: 60.0,
            near: 0.1,
            far: 100.0,
        }
    }
}

/// 包围盒组件 — 用于射线拾取和碰撞检测
pub struct BoundingBox {
    pub aabb: engine_math::AABB,
}

impl Default for BoundingBox {
    fn default() -> Self {
        Self {
            aabb: engine_math::AABB::new(
                glam::Vec3::new(-0.5, -0.5, -0.5),
                glam::Vec3::new(0.5, 0.5, 0.5),
            ),
        }
    }
}

pub struct LightComponent {
    pub direction: Vec3,
    pub color: engine_render_sw::Color,
    pub intensity: f32,
}

impl Default for LightComponent {
    fn default() -> Self {
        Self {
            direction: Vec3::new(0.0, 0.0, 1.0),
            color: engine_render_sw::Color::WHITE,
            intensity: 1.0,
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn camera_default() {
        let c = CameraComponent::default();
        assert!((c.fov - 60.0).abs() < f32::EPSILON);
    }

    #[test]
    fn light_default() {
        let l = LightComponent::default();
        assert_eq!(l.color, engine_render_sw::Color::WHITE);
    }
}
