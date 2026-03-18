// Copyright The SimpleGameEngine Contributors


mod fragment;
mod specular_lut;
mod vertex;

use std::collections::HashMap;
use std::sync::{Arc, RwLock};

use crate::light::Light;
use crate::uniform::{UniformBuffer, UniformValue};

use fragment::FragmentUniformCache;
use specular_lut::SpecularLut;
use vertex::VertexUniformCache;


/// 顶点 + 片段着色器，带有 uniform 缓存和高光查找表。
///
/// - `vertex_shader` 接受 `&self`（世界坐标存储在返回的 Vertex 上）
/// - `fragment_shader` 接受 `&self`（通过 `RwLock` 实现内部可变性，
///   用于高光查找表缓存）
pub struct Shader {
    pub(crate) uniform_buffer: UniformBuffer,
    pub(crate) vertex_cache: VertexUniformCache,
    pub(crate) fragment_cache: FragmentUniformCache,
    pub(crate) specular_lut_cache: Arc<RwLock<HashMap<u32, SpecularLut>>>,
}

impl Shader {
    /// 创建一个具有默认（空）状态的新着色器。
    pub fn new() -> Self {
        Self {
            uniform_buffer: UniformBuffer::new(),
            vertex_cache: VertexUniformCache::default(),
            fragment_cache: FragmentUniformCache::default(),
            specular_lut_cache: Arc::new(RwLock::new(HashMap::new())),
        }
    }


    /// 存储一个 uniform 并更新相关缓存。
    pub fn set_uniform(&mut self, name: &str, value: impl Into<UniformValue>) {
        let value = value.into();

        match &value {
            UniformValue::Mat4(m) => self.update_matrix_cache(name, *m),
            UniformValue::Light(l) => self.update_fragment_cache_light(name, l),
            UniformValue::Lights(ls) => self.update_fragment_cache_lights(name, ls),
            UniformValue::Vec3(v) => self.update_fragment_cache_vec3(name, *v),
            _ => {}
        }

        self.uniform_buffer.set(name, value);
    }

    /// 便捷方法：在 `"lights"` uniform 下设置多个光源。
    pub fn set_lights(&mut self, lights: &[Light]) {
        self.set_uniform("lights", lights.to_vec());
    }

    /// 确保顶点和片段缓存在渲染通道开始前是最新的。
    /// 在处理一帧的所有顶点/片段之前调用一次。
    pub fn prepare_caches(&mut self) {
        self.prepare_vertex_cache();
        self.prepare_fragment_cache();
    }
}

impl Default for Shader {
    fn default() -> Self {
        Self::new()
    }
}

impl Clone for Shader {
    fn clone(&self) -> Self {
        Self {
            uniform_buffer: self.uniform_buffer.clone(),
            vertex_cache: self.vertex_cache.clone(),
            fragment_cache: self.fragment_cache.clone(),
            specular_lut_cache: Arc::clone(&self.specular_lut_cache),
        }
    }
}


#[cfg(test)]
mod tests {
    use super::*;
    use crate::color::Color;
    use crate::fragment::Fragment;
    use crate::light::Light;
    use crate::material::{Material, Texture};
    use crate::math::{Mat4, Vec2, Vec3, Vec4};
    use crate::vertex::Vertex;

    use specular_lut::SPECULAR_LUT_RESOLUTION;


    #[test]
    fn shader_default() {
        let shader = Shader::new();
        assert!(!shader.vertex_cache.derived_valid);
        assert!(!shader.fragment_cache.derived_valid);
    }


    #[test]
    fn vertex_shader_identity_matrices() {
        let mut shader = Shader::new();
        shader.set_uniform("modelMatrix", Mat4::IDENTITY);
        shader.set_uniform("viewMatrix", Mat4::IDENTITY);
        shader.set_uniform("projectionMatrix", Mat4::IDENTITY);

        let v = Vertex::new(
            Vec4::new(1.0, 2.0, 3.0, 1.0),
            Vec3::Y,
            Vec2::new(0.5, 0.5),
            Color::WHITE,
        );

        let result = shader.vertex_shader(&v);

        // 使用单位矩阵时，位置应保持不变
        assert!((result.position.x - 1.0).abs() < 1e-5);
        assert!((result.position.y - 2.0).abs() < 1e-5);
        assert!((result.position.z - 3.0).abs() < 1e-5);
        assert!((result.position.w - 1.0).abs() < 1e-5);

        // 法线应保持不变
        assert!((result.normal.y - 1.0).abs() < 1e-5);

        // 顶点着色器后的 position 就是裁剪空间坐标
        assert!((result.position.x - 1.0).abs() < 1e-5);
    }

    #[test]
    fn vertex_shader_with_translation() {
        let mut shader = Shader::new();
        let model = Mat4::from_translation(Vec3::new(10.0, 0.0, 0.0));
        shader.set_uniform("modelMatrix", model);
        shader.set_uniform("viewMatrix", Mat4::IDENTITY);
        shader.set_uniform("projectionMatrix", Mat4::IDENTITY);

        let v = Vertex::new(
            Vec4::new(0.0, 0.0, 0.0, 1.0),
            Vec3::Y,
            Vec2::ZERO,
            Color::WHITE,
        );

        let result = shader.vertex_shader(&v);

        // 位置被平移了 (10, 0, 0)
        assert!((result.position.x - 10.0).abs() < 1e-5);

        // world_position 应保存世界空间坐标
        assert!((result.world_position.x - 10.0).abs() < 1e-5);
    }

    #[test]
    fn vertex_shader_preserves_tex_coords_and_color() {
        let mut shader = Shader::new();
        shader.set_uniform("modelMatrix", Mat4::IDENTITY);
        shader.set_uniform("viewMatrix", Mat4::IDENTITY);
        shader.set_uniform("projectionMatrix", Mat4::IDENTITY);

        let v = Vertex::new(
            Vec4::new(0.0, 0.0, 0.0, 1.0),
            Vec3::Y,
            Vec2::new(0.3, 0.7),
            Color::RED,
        );

        let result = shader.vertex_shader(&v);
        assert_eq!(result.tex_coords, Vec2::new(0.3, 0.7));
        assert_eq!(result.color, Color::RED);
    }


    #[test]
    fn fragment_shader_produces_non_zero_color() {
        let mut shader = Shader::new();
        let light = Light {
            name: "test".to_string(),
            position: Vec3::ZERO,
            direction: Vec3::new(0.0, 0.0, 1.0),
            color: Color::WHITE,
        };
        shader.set_lights(&[light]);
        shader.set_uniform("cameraPos", Vec3::new(0.0, 0.0, 5.0));
        shader.prepare_caches();

        let fragment = Fragment {
            screen_coord: [100, 100],
            normal: Vec3::new(0.0, 0.0, 1.0),
            uv: Vec2::ZERO,
            color: Color::new(200, 200, 200, 255),
            depth: 0.5,
            world_position: Vec3::ZERO,
        };
        let material = Material::default();

        let color = shader.fragment_shader(&fragment, &material);

        // 应产生非零、非白色的颜色
        assert!(color.r() > 0 || color.g() > 0 || color.b() > 0);
        assert_eq!(color.a(), 255);
    }

    #[test]
    fn fragment_shader_no_light_produces_ambient_only() {
        let shader = Shader::new();
        // 未设置光源 ── light_dirs 将为空，仅有环境光参与计算

        let fragment = Fragment {
            screen_coord: [0, 0],
            normal: Vec3::Y,
            uv: Vec2::ZERO,
            color: Color::new(255, 255, 255, 255),
            depth: 0.5,
            world_position: Vec3::ZERO,
        };
        let material = Material::default();

        let color = shader.fragment_shader(&fragment, &material);

        // 环境光 * 0.1 = 白色 * 0.1 → 约 25
        assert!(color.r() > 0);
        assert!(color.r() < 50);
    }

    #[test]
    fn fragment_shader_phong_formula() {
        let mut shader = Shader::new();

        // 光线指向 +Z 方向，片段法线朝向 +Z
        let light = Light {
            name: "sun".to_string(),
            position: Vec3::ZERO,
            direction: Vec3::new(0.0, 0.0, 1.0),
            color: Color::WHITE,
        };
        shader.set_lights(&[light]);
        shader.set_uniform("cameraPos", Vec3::new(0.0, 0.0, 5.0));
        shader.prepare_caches();

        let fragment = Fragment {
            screen_coord: [0, 0],
            normal: Vec3::new(0.0, 0.0, 1.0),
            uv: Vec2::ZERO,
            color: Color::new(255, 255, 255, 255), // 白色基底
            depth: 0.5,
            world_position: Vec3::ZERO,
        };
        let material = Material {
            shininess: 32.0,
            ..Material::default()
        };

        let color = shader.fragment_shader(&fragment, &material);
        // 光线方向与法线对齐时，漫反射强度 = 1.0
        // 环境光*0.1 + 漫反射 + 高光*0.2 应产生明显的亮度
        assert!(color.r() > 100);
    }


    #[test]
    fn specular_lut_shininess_zero_all_ones() {
        let lut = Shader::build_specular_lut(0.0);
        for &v in &lut.values {
            assert!((v - 1.0).abs() < 1e-6);
        }
    }

    #[test]
    fn specular_lut_shininess_negative_all_ones() {
        let lut = Shader::build_specular_lut(-5.0);
        for &v in &lut.values {
            assert!((v - 1.0).abs() < 1e-6);
        }
    }

    #[test]
    fn specular_lut_shininess_1_is_linear() {
        let lut = Shader::build_specular_lut(1.0);
        // 当 shininess=1 时，cos_theta^1 = cos_theta
        for i in 0..SPECULAR_LUT_RESOLUTION {
            let expected = i as f32 / (SPECULAR_LUT_RESOLUTION - 1) as f32;
            assert!(
                (lut.values[i] - expected).abs() < 1e-5,
                "at index {}: got {} expected {}",
                i,
                lut.values[i],
                expected
            );
        }
    }

    #[test]
    fn specular_lut_high_shininess_concentrated() {
        let lut = Shader::build_specular_lut(128.0);
        // 高光泽度下，较低的 cos_theta 值应接近 0
        assert!(lut.values[0] < 1e-6); // cos_theta = 0
        assert!(lut.values[128] < 0.01); // cos_theta = 0.502 → 0.502^128 ≈ 0
                                         // 较高的 cos_theta 应更接近 1
        assert!(lut.values[255] > 0.99); // cos_theta = 1.0 → 1.0^128 = 1.0
    }

    #[test]
    fn evaluate_specular_shininess_zero() {
        let shader = Shader::new();
        assert!((shader.evaluate_specular(0.5, 0.0) - 1.0).abs() < 1e-6);
    }

    #[test]
    fn evaluate_specular_cos_zero() {
        let shader = Shader::new();
        assert!((shader.evaluate_specular(0.0, 32.0)).abs() < 1e-6);
    }

    #[test]
    fn evaluate_specular_cos_one() {
        let shader = Shader::new();
        // cos_theta=1.0 → 1.0^shininess = 1.0
        let val = shader.evaluate_specular(1.0, 32.0);
        assert!((val - 1.0).abs() < 1e-3);
    }

    #[test]
    fn evaluate_specular_interpolation() {
        let shader = Shader::new();
        // 中间范围应通过查找表插值产生合理的值
        // 当 shininess=1 时，specular = cos_theta = 0.5
        let val = shader.evaluate_specular(0.5, 1.0);
        assert!((val - 0.5).abs() < 0.01);
    }

    #[test]
    fn specular_lut_caching() {
        let shader = Shader::new();
        // 第一次调用构建查找表
        let val1 = shader.evaluate_specular(0.5, 32.0);
        // 第二次调用应命中缓存
        let val2 = shader.evaluate_specular(0.5, 32.0);
        assert!((val1 - val2).abs() < 1e-10);

        // 验证缓存中恰好有一个条目
        let cache = shader
            .specular_lut_cache
            .read()
            .expect("specular LUT lock poisoned");
        assert_eq!(cache.len(), 1);
    }

    #[test]
    fn specular_lut_shared_across_clones() {
        let shader = Shader::new();
        // 构建一个查找表条目
        let _ = shader.evaluate_specular(0.5, 32.0);

        // Clone 共享同一个 Arc
        let shader2 = shader.clone();
        {
            let cache = shader2
                .specular_lut_cache
                .read()
                .expect("specular LUT lock poisoned");
            assert_eq!(cache.len(), 1);
        }

        // 通过克隆体构建的新条目对原始实例可见
        let _ = shader2.evaluate_specular(0.5, 64.0);
        {
            let cache = shader
                .specular_lut_cache
                .read()
                .expect("specular LUT lock poisoned");
            assert_eq!(cache.len(), 2);
        }
    }


    #[test]
    fn sample_texture_basic() {
        let texture = Texture {
            data: vec![
                255, 0, 0, 255, 0, 255, 0, 255, 0, 0, 255, 255, 128, 128, 128, 255,
            ],
            width: 2,
            height: 2,
            channels: 4,
        };
        // UV (0, 0) → 像素 (0, 0) → 红色
        let c = Shader::sample_texture(&texture, Vec2::new(0.0, 0.0));
        assert_eq!(c, Color::new(255, 0, 0, 255));
    }

    #[test]
    fn sample_texture_uv_wrapping() {
        let texture = Texture {
            data: vec![100, 150, 200, 255],
            width: 1,
            height: 1,
            channels: 4,
        };
        // UV (1.5, 2.7) 应环绕：u = 0.5, v = 0.7 → 对于 1×1 纹理仍是像素 (0, 0)
        let c = Shader::sample_texture(&texture, Vec2::new(1.5, 2.7));
        assert_eq!(c, Color::new(100, 150, 200, 255));
    }

    #[test]
    fn sample_texture_negative_uv() {
        let texture = Texture {
            data: vec![42, 43, 44, 255],
            width: 1,
            height: 1,
            channels: 4,
        };
        // 负值 UV：-0.3 - floor(-0.3) = -0.3 - (-1) = 0.7
        let c = Shader::sample_texture(&texture, Vec2::new(-0.3, -0.3));
        assert_eq!(c, Color::new(42, 43, 44, 255));
    }


    #[test]
    fn cache_invalidation_on_matrix_update() {
        let mut shader = Shader::new();
        shader.set_uniform("modelMatrix", Mat4::IDENTITY);
        shader.set_uniform("viewMatrix", Mat4::IDENTITY);
        shader.set_uniform("projectionMatrix", Mat4::IDENTITY);

        // 设置全部三个矩阵后，缓存应为有效
        assert!(shader.vertex_cache.derived_valid);

        // 更新模型矩阵应使缓存失效，然后自动重新计算
        let new_model = Mat4::from_translation(Vec3::new(1.0, 0.0, 0.0));
        shader.set_uniform("modelMatrix", new_model);

        // 仍然有效，因为三个矩阵都存在 → 自动重新计算
        assert!(shader.vertex_cache.derived_valid);
        // MVP 应反映新的模型矩阵
        let expected_mvp = Mat4::IDENTITY * Mat4::IDENTITY * new_model;
        let diff = (shader.vertex_cache.mvp - expected_mvp).abs_diff_eq(Mat4::ZERO, 1e-6);
        assert!(diff);
    }

    #[test]
    fn cache_not_valid_with_partial_matrices() {
        let mut shader = Shader::new();
        shader.set_uniform("modelMatrix", Mat4::IDENTITY);
        // 仅设置了模型矩阵，未设置视图或投影矩阵
        assert!(!shader.vertex_cache.derived_valid);
    }

    #[test]
    fn fragment_cache_invalidation() {
        let mut shader = Shader::new();
        let light = Light {
            name: "sun".to_string(),
            position: Vec3::ZERO,
            direction: Vec3::new(0.0, -1.0, 0.0),
            color: Color::WHITE,
        };
        shader.set_lights(&[light]);
        // 仅设置了光源，未设置相机
        assert!(!shader.fragment_cache.derived_valid);

        shader.set_uniform("cameraPos", Vec3::new(0.0, 0.0, 5.0));
        // 现在光源和相机都已设置 → 自动重新计算
        assert!(shader.fragment_cache.derived_valid);
        assert_eq!(shader.fragment_cache.light_dirs_normalized.len(), 1);
    }

    #[test]
    fn prepare_caches_fills_from_uniform_buffer() {
        let mut shader = Shader::new();
        // 直接在缓冲区中设置 uniform，不触发缓存更新
        // （模拟 PrepareUniformCaches 路径）
        shader.uniform_buffer.set("modelMatrix", Mat4::IDENTITY);
        shader.uniform_buffer.set("viewMatrix", Mat4::IDENTITY);
        shader
            .uniform_buffer
            .set("projectionMatrix", Mat4::IDENTITY);

        assert!(!shader.vertex_cache.derived_valid);
        shader.prepare_caches();
        assert!(shader.vertex_cache.derived_valid);
    }

    #[test]
    fn prepare_fragment_cache_multi_light() {
        let mut shader = Shader::new();
        let lights = vec![
            Light {
                name: "a".to_string(),
                direction: Vec3::new(1.0, 0.0, 0.0),
                ..Light::default()
            },
            Light {
                name: "b".to_string(),
                direction: Vec3::new(0.0, 1.0, 0.0),
                ..Light::default()
            },
        ];
        shader.uniform_buffer.set("lights", lights);
        shader
            .uniform_buffer
            .set("cameraPos", Vec3::new(0.0, 0.0, 5.0));

        shader.prepare_caches();
        assert!(shader.fragment_cache.derived_valid);
        assert_eq!(shader.fragment_cache.light_dirs_normalized.len(), 2);
    }

    #[test]
    fn prepare_fragment_cache_single_light_fallback() {
        let mut shader = Shader::new();
        shader.uniform_buffer.set(
            "light",
            Light {
                direction: Vec3::new(0.0, 0.0, 1.0),
                ..Light::default()
            },
        );
        shader
            .uniform_buffer
            .set("cameraPos", Vec3::new(0.0, 0.0, 5.0));

        shader.prepare_caches();
        assert!(shader.fragment_cache.derived_valid);
        assert_eq!(shader.fragment_cache.lights.len(), 1);
    }


    #[test]
    fn set_lights_stores_in_buffer() {
        let mut shader = Shader::new();
        shader.set_lights(&[Light::default(), Light::default()]);
        assert!(shader.uniform_buffer.has("lights"));
        assert_eq!(shader.uniform_buffer.get_lights("lights").unwrap().len(), 2);
    }
}
