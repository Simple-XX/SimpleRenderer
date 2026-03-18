use std::path::Path;
use std::sync::Arc;

use crate::color::Color;
use crate::error::{RendererError, Result};
use crate::math::Vec3;

/// 图像纹理，像素数据以扁平 `Vec<u8>` 存储。
#[derive(Debug, Clone, Default)]
pub struct Texture {
    pub data: Vec<u8>,
    pub width: u32,
    pub height: u32,
    pub channels: u32,
}

impl Texture {
    /// 从图像文件加载纹理（PNG、JPEG、BMP、TGA）。
    pub fn load_from_file<P: AsRef<Path>>(path: P) -> Result<Texture> {
        let path = path.as_ref();
        let img = image::open(path).map_err(|e| RendererError::TextureLoad {
            path: path.display().to_string(),
            source: Box::new(e),
        })?;

        let rgba = img.to_rgba8();
        let (width, height) = rgba.dimensions();

        Ok(Texture {
            data: rgba.into_raw(),
            width,
            height,
            channels: 4,
        })
    }

    /// 获取 `(x, y)` 处像素的颜色。
    ///
    /// 坐标会被钳制到纹理范围内。
    /// 支持 3 通道（RGB）和 4 通道（RGBA）数据。
    pub fn get_pixel(&self, x: i32, y: i32) -> Color {
        if self.width == 0 || self.height == 0 || self.data.is_empty() {
            return Color::default();
        }

        let x = x.clamp(0, self.width as i32 - 1) as u32;
        let y = y.clamp(0, self.height as i32 - 1) as u32;
        let idx = (y * self.width + x) as usize * self.channels as usize;

        match self.channels {
            4 => Color::new(
                self.data[idx],
                self.data[idx + 1],
                self.data[idx + 2],
                self.data[idx + 3],
            ),
            3 => Color::new(self.data[idx], self.data[idx + 1], self.data[idx + 2], 255),
            _ => Color::default(),
        }
    }
}

/// Phong 光照的材质属性。
#[derive(Debug, Clone)]
pub struct Material {
    pub shininess: f32,
    pub ambient: Vec3,
    pub diffuse: Vec3,
    pub specular: Vec3,
    pub ambient_texture: Option<Arc<Texture>>,
    pub diffuse_texture: Option<Arc<Texture>>,
    pub specular_texture: Option<Arc<Texture>>,
}

impl Default for Material {
    fn default() -> Self {
        Self {
            shininess: 32.0,
            ambient: Vec3::new(0.2, 0.2, 0.2),
            diffuse: Vec3::new(0.8, 0.8, 0.8),
            specular: Vec3::new(1.0, 1.0, 1.0),
            ambient_texture: None,
            diffuse_texture: None,
            specular_texture: None,
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn texture_default_is_empty() {
        let t = Texture::default();
        assert!(t.data.is_empty());
        assert_eq!(t.width, 0);
        assert_eq!(t.height, 0);
        assert_eq!(t.channels, 0);
    }

    #[test]
    fn texture_get_pixel_empty_returns_default() {
        let t = Texture::default();
        assert_eq!(t.get_pixel(0, 0), Color::default());
    }

    #[test]
    fn texture_get_pixel_rgba() {
        let t = Texture {
            data: vec![10, 20, 30, 40, 50, 60, 70, 80],
            width: 2,
            height: 1,
            channels: 4,
        };
        assert_eq!(t.get_pixel(0, 0), Color::new(10, 20, 30, 40));
        assert_eq!(t.get_pixel(1, 0), Color::new(50, 60, 70, 80));
    }

    #[test]
    fn texture_get_pixel_rgb() {
        let t = Texture {
            data: vec![10, 20, 30, 40, 50, 60],
            width: 2,
            height: 1,
            channels: 3,
        };
        assert_eq!(t.get_pixel(0, 0), Color::new(10, 20, 30, 255));
        assert_eq!(t.get_pixel(1, 0), Color::new(40, 50, 60, 255));
    }

    #[test]
    fn texture_get_pixel_clamps_coords() {
        let t = Texture {
            data: vec![10, 20, 30, 40],
            width: 1,
            height: 1,
            channels: 4,
        };
        // 越界坐标应被钳制到有效范围
        assert_eq!(t.get_pixel(-5, -5), Color::new(10, 20, 30, 40));
        assert_eq!(t.get_pixel(100, 100), Color::new(10, 20, 30, 40));
    }

    #[test]
    fn texture_load_nonexistent_file() {
        let result = Texture::load_from_file("/nonexistent/path.png");
        assert!(result.is_err());
    }

    #[test]
    fn material_default() {
        let m = Material::default();
        assert_eq!(m.shininess, 32.0);
        assert!(m.ambient_texture.is_none());
        assert!(m.diffuse_texture.is_none());
        assert!(m.specular_texture.is_none());
    }
}
