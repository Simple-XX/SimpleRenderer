// Copyright The SimpleGameEngine Contributors


use engine_core::Component;
use glam::{Quat, Vec3};

use crate::asset::Handle;

pub struct Transform {
    pub position: Vec3,
    pub rotation: Quat,
    pub scale: Vec3,
}

impl Component for Transform {}

impl Default for Transform {
    fn default() -> Self {
        Self {
            position: Vec3::ZERO,
            rotation: Quat::IDENTITY,
            scale: Vec3::ONE,
        }
    }
}

pub struct Name(pub String);
impl Component for Name {}

pub struct MeshRenderer {
    pub model_handle: Handle<engine_renderer::Model>,
}
impl Component for MeshRenderer {}

pub struct CameraComponent {
    pub fov: f32,
    pub near: f32,
    pub far: f32,
}
impl Component for CameraComponent {}

impl Default for CameraComponent {
    fn default() -> Self {
        Self {
            fov: 60.0,
            near: 0.1,
            far: 100.0,
        }
    }
}

pub struct LightComponent {
    pub direction: Vec3,
    pub color: engine_renderer::Color,
    pub intensity: f32,
}
impl Component for LightComponent {}

impl Default for LightComponent {
    fn default() -> Self {
        Self {
            direction: Vec3::new(0.0, 0.0, 1.0),
            color: engine_renderer::Color::WHITE,
            intensity: 1.0,
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn transform_default() {
        let t = Transform::default();
        assert_eq!(t.position, Vec3::ZERO);
        assert_eq!(t.scale, Vec3::ONE);
    }

    #[test]
    fn camera_default() {
        let c = CameraComponent::default();
        assert!((c.fov - 60.0).abs() < f32::EPSILON);
    }

    #[test]
    fn light_default() {
        let l = LightComponent::default();
        assert_eq!(l.color, engine_renderer::Color::WHITE);
    }
}
