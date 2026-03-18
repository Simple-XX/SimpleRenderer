// 材质数据定义

use crate::texture::TextureHandle;

#[derive(Debug, Clone, Copy, PartialEq, Eq, Hash)]
pub struct MaterialHandle(pub u64);

#[derive(Debug, Clone)]
pub struct MaterialData {
    pub diffuse_color: glam::Vec3,
    pub specular_color: glam::Vec3,
    pub ambient_color: glam::Vec3,
    pub shininess: f32,
    pub diffuse_texture: Option<TextureHandle>,
    pub specular_texture: Option<TextureHandle>,
    pub normal_texture: Option<TextureHandle>,
}

impl Default for MaterialData {
    fn default() -> Self {
        Self {
            diffuse_color: glam::Vec3::splat(0.8),
            specular_color: glam::Vec3::splat(1.0),
            ambient_color: glam::Vec3::splat(0.1),
            shininess: 32.0,
            diffuse_texture: None,
            specular_texture: None,
            normal_texture: None,
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_material_data_default() {
        let mat = MaterialData::default();
        assert_eq!(mat.diffuse_color, glam::Vec3::splat(0.8));
        assert_eq!(mat.specular_color, glam::Vec3::splat(1.0));
        assert_eq!(mat.ambient_color, glam::Vec3::splat(0.1));
        assert_eq!(mat.shininess, 32.0);
        assert!(mat.diffuse_texture.is_none());
        assert!(mat.specular_texture.is_none());
        assert!(mat.normal_texture.is_none());
    }
}
