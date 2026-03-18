use std::collections::HashMap;

use crate::color::Color;
use crate::light::Light;
use crate::material::Material;
use crate::math::{Mat3, Mat4, Vec2, Vec3, Vec4};

/// 预定义的 uniform 名称常量，防止字符串键拼写错误。
///
/// 使用这些常量代替原始字符串，可在编译期检测出
/// 拼写错误的 uniform 名称。
pub mod names {
    pub const MODEL_MATRIX: &str = "modelMatrix";
    pub const VIEW_MATRIX: &str = "viewMatrix";
    pub const PROJECTION_MATRIX: &str = "projectionMatrix";
    pub const CAMERA_POS: &str = "cameraPos";
    pub const LIGHT: &str = "light";
    pub const LIGHTS: &str = "lights";
}

/// 可存储在 [`UniformBuffer`] 中的动态类型值。
#[derive(Debug, Clone)]
pub enum UniformValue {
    Int(i32),
    Float(f32),
    Vec2(Vec2),
    Vec3(Vec3),
    Vec4(Vec4),
    Mat3(Mat3),
    Mat4(Mat4),
    Color(Color),
    Material(Material),
    Light(Light),
    Lights(Vec<Light>),
}

impl From<i32> for UniformValue {
    #[inline]
    fn from(v: i32) -> Self {
        UniformValue::Int(v)
    }
}

impl From<f32> for UniformValue {
    #[inline]
    fn from(v: f32) -> Self {
        UniformValue::Float(v)
    }
}

impl From<Vec2> for UniformValue {
    #[inline]
    fn from(v: Vec2) -> Self {
        UniformValue::Vec2(v)
    }
}

impl From<Vec3> for UniformValue {
    #[inline]
    fn from(v: Vec3) -> Self {
        UniformValue::Vec3(v)
    }
}

impl From<Vec4> for UniformValue {
    #[inline]
    fn from(v: Vec4) -> Self {
        UniformValue::Vec4(v)
    }
}

impl From<Mat3> for UniformValue {
    #[inline]
    fn from(v: Mat3) -> Self {
        UniformValue::Mat3(v)
    }
}

impl From<Mat4> for UniformValue {
    #[inline]
    fn from(v: Mat4) -> Self {
        UniformValue::Mat4(v)
    }
}

impl From<Color> for UniformValue {
    #[inline]
    fn from(v: Color) -> Self {
        UniformValue::Color(v)
    }
}

impl From<Material> for UniformValue {
    #[inline]
    fn from(v: Material) -> Self {
        UniformValue::Material(v)
    }
}

impl From<Light> for UniformValue {
    #[inline]
    fn from(v: Light) -> Self {
        UniformValue::Light(v)
    }
}

impl From<Vec<Light>> for UniformValue {
    #[inline]
    fn from(v: Vec<Light>) -> Self {
        UniformValue::Lights(v)
    }
}

/// 按名称存储的 uniform 容器，提供类型化的访问方法。
///
/// 通过字符串键存储 uniform，并提供返回 `Option<T>`
/// 的类型化访问方法（而非在类型不匹配时 panic）。
#[derive(Debug, Clone, Default)]
pub struct UniformBuffer {
    uniforms: HashMap<String, UniformValue>,
}

impl UniformBuffer {
    /// 创建一个空的 uniform 缓冲区。
    pub fn new() -> Self {
        Self {
            uniforms: HashMap::new(),
        }
    }

    /// 设置 uniform 值。接受任何实现了 `Into<UniformValue>` 的类型。
    pub fn set(&mut self, name: &str, value: impl Into<UniformValue>) {
        self.uniforms.insert(name.to_string(), value.into());
    }

    /// 检查是否存在指定名称的 uniform。
    pub fn has(&self, name: &str) -> bool {
        self.uniforms.contains_key(name)
    }

    pub fn get_int(&self, name: &str) -> Option<i32> {
        match self.uniforms.get(name)? {
            UniformValue::Int(v) => Some(*v),
            _ => None,
        }
    }

    pub fn get_float(&self, name: &str) -> Option<f32> {
        match self.uniforms.get(name)? {
            UniformValue::Float(v) => Some(*v),
            _ => None,
        }
    }

    pub fn get_vec2(&self, name: &str) -> Option<Vec2> {
        match self.uniforms.get(name)? {
            UniformValue::Vec2(v) => Some(*v),
            _ => None,
        }
    }

    pub fn get_vec3(&self, name: &str) -> Option<Vec3> {
        match self.uniforms.get(name)? {
            UniformValue::Vec3(v) => Some(*v),
            _ => None,
        }
    }

    pub fn get_vec4(&self, name: &str) -> Option<Vec4> {
        match self.uniforms.get(name)? {
            UniformValue::Vec4(v) => Some(*v),
            _ => None,
        }
    }

    pub fn get_mat3(&self, name: &str) -> Option<Mat3> {
        match self.uniforms.get(name)? {
            UniformValue::Mat3(v) => Some(*v),
            _ => None,
        }
    }

    pub fn get_mat4(&self, name: &str) -> Option<Mat4> {
        match self.uniforms.get(name)? {
            UniformValue::Mat4(v) => Some(*v),
            _ => None,
        }
    }

    pub fn get_light(&self, name: &str) -> Option<&Light> {
        match self.uniforms.get(name)? {
            UniformValue::Light(v) => Some(v),
            _ => None,
        }
    }

    pub fn get_lights(&self, name: &str) -> Option<&Vec<Light>> {
        match self.uniforms.get(name)? {
            UniformValue::Lights(v) => Some(v),
            _ => None,
        }
    }

    pub fn get_material(&self, name: &str) -> Option<&Material> {
        match self.uniforms.get(name)? {
            UniformValue::Material(v) => Some(v),
            _ => None,
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn new_buffer_is_empty() {
        let buf = UniformBuffer::new();
        assert!(!buf.has("anything"));
    }

    #[test]
    fn set_and_get_int() {
        let mut buf = UniformBuffer::new();
        buf.set("count", 42_i32);
        assert!(buf.has("count"));
        assert_eq!(buf.get_int("count"), Some(42));
    }

    #[test]
    fn set_and_get_float() {
        let mut buf = UniformBuffer::new();
        buf.set("time", 1.5_f32);
        assert_eq!(buf.get_float("time"), Some(1.5));
    }

    #[test]
    fn set_and_get_vec2() {
        let mut buf = UniformBuffer::new();
        let v = Vec2::new(1.0, 2.0);
        buf.set("uv", v);
        assert_eq!(buf.get_vec2("uv"), Some(v));
    }

    #[test]
    fn set_and_get_vec3() {
        let mut buf = UniformBuffer::new();
        let v = Vec3::new(1.0, 2.0, 3.0);
        buf.set("position", v);
        assert_eq!(buf.get_vec3("position"), Some(v));
    }

    #[test]
    fn set_and_get_vec4() {
        let mut buf = UniformBuffer::new();
        let v = Vec4::new(1.0, 2.0, 3.0, 4.0);
        buf.set("clip", v);
        assert_eq!(buf.get_vec4("clip"), Some(v));
    }

    #[test]
    fn set_and_get_mat3() {
        let mut buf = UniformBuffer::new();
        let m = Mat3::IDENTITY;
        buf.set("normalMatrix", m);
        assert_eq!(buf.get_mat3("normalMatrix"), Some(m));
    }

    #[test]
    fn set_and_get_mat4() {
        let mut buf = UniformBuffer::new();
        let m = Mat4::IDENTITY;
        buf.set("modelMatrix", m);
        assert_eq!(buf.get_mat4("modelMatrix"), Some(m));
    }

    #[test]
    fn set_and_get_light() {
        let mut buf = UniformBuffer::new();
        let light = Light {
            name: "sun".to_string(),
            position: Vec3::new(0.0, 10.0, 0.0),
            direction: Vec3::new(0.0, -1.0, 0.0),
            color: Color::WHITE,
        };
        buf.set("light", light.clone());
        let got = buf.get_light("light").unwrap();
        assert_eq!(got.name, "sun");
        assert_eq!(got.position, Vec3::new(0.0, 10.0, 0.0));
    }

    #[test]
    fn set_and_get_lights() {
        let mut buf = UniformBuffer::new();
        let lights = vec![Light::default(), Light::default()];
        buf.set("lights", lights.clone());
        let got = buf.get_lights("lights").unwrap();
        assert_eq!(got.len(), 2);
    }

    #[test]
    fn set_and_get_material() {
        let mut buf = UniformBuffer::new();
        let mat = Material::default();
        buf.set("material", mat);
        let got = buf.get_material("material").unwrap();
        assert_eq!(got.shininess, 32.0);
    }

    #[test]
    fn missing_key_returns_none() {
        let buf = UniformBuffer::new();
        assert_eq!(buf.get_int("missing"), None);
        assert_eq!(buf.get_float("missing"), None);
        assert_eq!(buf.get_vec3("missing"), None);
        assert_eq!(buf.get_mat4("missing"), None);
        assert!(buf.get_light("missing").is_none());
        assert!(buf.get_lights("missing").is_none());
    }

    #[test]
    fn wrong_type_returns_none() {
        let mut buf = UniformBuffer::new();
        buf.set("value", 42_i32);
        // 存储为 Int，但请求 Float
        assert_eq!(buf.get_float("value"), None);
        assert_eq!(buf.get_mat4("value"), None);
    }

    #[test]
    fn overwrite_uniform() {
        let mut buf = UniformBuffer::new();
        buf.set("x", 1.0_f32);
        assert_eq!(buf.get_float("x"), Some(1.0));
        buf.set("x", 2.0_f32);
        assert_eq!(buf.get_float("x"), Some(2.0));
    }

    #[test]
    fn has_returns_false_for_missing() {
        let buf = UniformBuffer::new();
        assert!(!buf.has("nope"));
    }

    #[test]
    fn clone_buffer() {
        let mut buf = UniformBuffer::new();
        buf.set("a", 1_i32);
        let buf2 = buf.clone();
        assert_eq!(buf2.get_int("a"), Some(1));
    }

    #[test]
    fn default_buffer() {
        let buf = UniformBuffer::default();
        assert!(!buf.has("anything"));
    }
}
