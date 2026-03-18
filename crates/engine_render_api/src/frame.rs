// 帧数据定义

use glam::{Mat4, Vec3};

use crate::color::Color;
use crate::material::MaterialHandle;
use crate::mesh::MeshHandle;
use crate::viewport::Viewport;

pub struct CameraData {
    pub view: Mat4,
    pub projection: Mat4,
    pub position: Vec3,
    pub fov: f32,
    pub near: f32,
    pub far: f32,
}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum LightKind {
    Directional,
    Point,
    Spot,
}

pub struct LightData {
    pub kind: LightKind,
    pub position: Vec3,
    pub direction: Vec3,
    pub color: Vec3,
    pub intensity: f32,
}

pub struct DrawCall {
    pub mesh: MeshHandle,
    pub material: MaterialHandle,
    pub transform: Mat4,
}

pub struct FrameData {
    pub camera: CameraData,
    pub lights: Vec<LightData>,
    pub draw_calls: Vec<DrawCall>,
    pub clear_color: Color,
    pub viewport: Viewport,
}

pub struct FrameOutput {
    pub frame_time_ms: f64,
    pub triangle_count: u32,
    pub draw_call_count: u32,
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_frame_output_construction() {
        let output = FrameOutput {
            frame_time_ms: 16.67,
            triangle_count: 1000,
            draw_call_count: 5,
        };
        assert!((output.frame_time_ms - 16.67).abs() < f64::EPSILON);
        assert_eq!(output.triangle_count, 1000);
        assert_eq!(output.draw_call_count, 5);
    }

    #[test]
    fn test_light_kind_eq() {
        assert_eq!(LightKind::Directional, LightKind::Directional);
        assert_ne!(LightKind::Point, LightKind::Spot);
    }
}
