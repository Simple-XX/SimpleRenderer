// Copyright (c) Simple-XX/SimpleRenderer
// SPDX-License-Identifier: MIT

use crate::color::Color;
use crate::fragment::Fragment;
use crate::light::Light;
use crate::material::{Material, Texture};
use crate::math::{Vec2, Vec3};
use crate::uniform;

use super::Shader;


/// 缓存的片段着色器 uniform（光源 + 相机），避免逐片段的 HashMap 查找。
#[derive(Clone)]
pub(crate) struct FragmentUniformCache {
    pub(crate) lights: Vec<Light>,
    pub(crate) camera_pos: Vec3,
    pub(crate) light_dirs_normalized: Vec<Vec3>,
    pub(crate) has_lights: bool,
    pub(crate) has_camera: bool,
    pub(crate) derived_valid: bool,
}

impl Default for FragmentUniformCache {
    fn default() -> Self {
        Self {
            lights: Vec::new(),
            camera_pos: Vec3::ZERO,
            light_dirs_normalized: Vec::new(),
            has_lights: false,
            has_camera: false,
            derived_valid: false,
        }
    }
}

impl Shader {

    /// 使用 Blinn-Phong 着色模型计算片段的最终颜色。
    pub fn fragment_shader(&self, fragment: &Fragment, material: &Material) -> Color {
        // 辅助函数：Color → 归一化 Vec3，范围 [0, 1]
        let color_to_vec = |c: &Color| -> Vec3 {
            const INV255: f32 = 1.0 / 255.0;
            Vec3::new(
                c.r() as f32 * INV255,
                c.g() as f32 * INV255,
                c.b() as f32 * INV255,
            )
        };

        let base_color = color_to_vec(&fragment.color);
        let normal = fragment.normal.normalize_or_zero();
        let uv = fragment.uv;

        // 从缓存或 uniform 缓冲区获取光源和相机
        let fallback_dirs;
        let light_dirs: &[Vec3] = if self.fragment_cache.derived_valid {
            &self.fragment_cache.light_dirs_normalized
        } else {
            // 回退：从 uniform 缓冲区读取
            fallback_dirs = if let Some(ls) = self.uniform_buffer.get_lights(uniform::names::LIGHTS)
            {
                ls.iter()
                    .map(|l| l.direction.normalize_or_zero())
                    .collect::<Vec<_>>()
            } else if let Some(l) = self.uniform_buffer.get_light(uniform::names::LIGHT) {
                vec![l.direction.normalize_or_zero()]
            } else {
                Vec::new()
            };
            &fallback_dirs
        };
        let camera_pos = if self.fragment_cache.derived_valid {
            self.fragment_cache.camera_pos
        } else {
            self.uniform_buffer
                .get_vec3(uniform::names::CAMERA_POS)
                .unwrap_or(Vec3::ZERO)
        };

        // 视线方向（从相机指向片段）
        let view_dir = (fragment.world_position - camera_pos).normalize_or_zero();

        // 环境光（仅一次，使用环境光纹理或基础颜色）
        let ambient_rgb = if let Some(ref tex) = material.ambient_texture {
            color_to_vec(&Self::sample_texture(tex, uv))
        } else {
            base_color
        };

        // 逐光源累加漫反射 + 高光
        let mut diffuse_accum = Vec3::ZERO;
        let mut specular_accum = Vec3::ZERO;

        for ldir in light_dirs.iter() {
            let intensity = normal.dot(*ldir).max(0.0);

            // 漫反射
            let kd = if let Some(ref tex) = material.diffuse_texture {
                color_to_vec(&Self::sample_texture(tex, uv))
            } else {
                base_color
            };
            diffuse_accum += kd * intensity;

            // 高光（Blinn-Phong）
            let half_vector = (*ldir + view_dir).normalize_or_zero();
            let cos_theta = normal.dot(half_vector).max(0.0);
            let spec = self.evaluate_specular(cos_theta, material.shininess);
            let ks = if let Some(ref tex) = material.specular_texture {
                color_to_vec(&Self::sample_texture(tex, uv))
            } else {
                Vec3::ONE
            };
            specular_accum += ks * spec;
        }

        // 最终颜色：ambient * 0.1 + diffuse + specular * 0.2
        let out_rgb = ambient_rgb * 0.1 + diffuse_accum + specular_accum * 0.2;
        let r = out_rgb.x.clamp(0.0, 1.0);
        let g = out_rgb.y.clamp(0.0, 1.0);
        let b = out_rgb.z.clamp(0.0, 1.0);

        Color::from_normalized(r, g, b, 1.0)
    }


    pub(super) fn update_fragment_cache_light(&mut self, name: &str, value: &Light) {
        if name != uniform::names::LIGHT {
            return;
        }
        self.fragment_cache.lights.clear();
        self.fragment_cache.lights.push(value.clone());
        self.fragment_cache.has_lights = true;
        self.fragment_cache.derived_valid = false;
        if self.fragment_cache.has_lights && self.fragment_cache.has_camera {
            self.recalculate_fragment_derived();
        }
    }

    pub(super) fn update_fragment_cache_lights(&mut self, name: &str, value: &[Light]) {
        if name != uniform::names::LIGHTS {
            return;
        }
        self.fragment_cache.lights = value.to_vec();
        self.fragment_cache.has_lights = true;
        self.fragment_cache.derived_valid = false;
        if self.fragment_cache.has_lights && self.fragment_cache.has_camera {
            self.recalculate_fragment_derived();
        }
    }

    pub(super) fn update_fragment_cache_vec3(&mut self, name: &str, value: Vec3) {
        if name != uniform::names::CAMERA_POS {
            return;
        }
        self.fragment_cache.camera_pos = value;
        self.fragment_cache.has_camera = true;
        self.fragment_cache.derived_valid = false;
        if self.fragment_cache.has_lights && self.fragment_cache.has_camera {
            self.recalculate_fragment_derived();
        }
    }

    /// 预计算归一化的光源方向。
    fn recalculate_fragment_derived(&mut self) {
        self.fragment_cache.light_dirs_normalized = self
            .fragment_cache
            .lights
            .iter()
            .map(|l| l.direction.normalize_or_zero())
            .collect();
        self.fragment_cache.derived_valid = true;
    }


    pub(super) fn prepare_fragment_cache(&mut self) {
        if self.fragment_cache.derived_valid {
            return;
        }

        // 优先使用多光源路径
        if let (Some(lights), Some(cam)) = (
            self.uniform_buffer.get_lights(uniform::names::LIGHTS),
            self.uniform_buffer.get_vec3(uniform::names::CAMERA_POS),
        ) {
            self.fragment_cache.lights = lights.clone();
            self.fragment_cache.has_lights = true;
            self.fragment_cache.camera_pos = cam;
            self.fragment_cache.has_camera = true;
            self.recalculate_fragment_derived();
            return;
        }

        // 单光源回退
        if let (Some(light), Some(cam)) = (
            self.uniform_buffer.get_light(uniform::names::LIGHT),
            self.uniform_buffer.get_vec3(uniform::names::CAMERA_POS),
        ) {
            self.fragment_cache.lights = vec![light.clone()];
            self.fragment_cache.has_lights = true;
            self.fragment_cache.camera_pos = cam;
            self.fragment_cache.has_camera = true;
            self.recalculate_fragment_derived();
        }
    }


    /// 在给定 UV 坐标处对纹理进行采样（带环绕寻址）。
    pub(super) fn sample_texture(texture: &Texture, uv: Vec2) -> Color {
        // 环绕到 [0, 1]
        let u = uv.x - uv.x.floor();
        let v = uv.y - uv.y.floor();

        // 转换到像素空间
        let x = (u * texture.width as f32) as i32;
        let y = (v * texture.height as f32) as i32;

        // 钳制到边界范围
        let x = x.clamp(0, texture.width as i32 - 1);
        let y = y.clamp(0, texture.height as i32 - 1);

        texture.get_pixel(x, y)
    }
}
