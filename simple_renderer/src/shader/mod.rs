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

// ── Shader ────────────────────────────────────────────────────────────────

/// Vertex + fragment shader with uniform caching and specular LUT.
///
/// - `vertex_shader` takes `&self` (world position stored on returned Vertex)
/// - `fragment_shader` takes `&self` (interior mutability via `RwLock` for
///   specular LUT cache)
pub struct Shader {
    pub(crate) uniform_buffer: UniformBuffer,
    pub(crate) vertex_cache: VertexUniformCache,
    pub(crate) fragment_cache: FragmentUniformCache,
    pub(crate) specular_lut_cache: Arc<RwLock<HashMap<u32, SpecularLut>>>,
}

impl Shader {
    /// Create a new shader with default (empty) state.
    pub fn new() -> Self {
        Self {
            uniform_buffer: UniformBuffer::new(),
            vertex_cache: VertexUniformCache::default(),
            fragment_cache: FragmentUniformCache::default(),
            specular_lut_cache: Arc::new(RwLock::new(HashMap::new())),
        }
    }

    // ── Uniform management ────────────────────────────────────────────

    /// Store a uniform and update relevant caches.
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

    /// Convenience: set multiple lights under the `"lights"` uniform.
    pub fn set_lights(&mut self, lights: &[Light]) {
        self.set_uniform("lights", lights.to_vec());
    }

    /// Ensure both vertex and fragment caches are up-to-date before a render
    /// pass. Call once before processing all vertices/fragments of a frame.
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

// ── Tests ─────────────────────────────────────────────────────────────────

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

    // ── UniformBuffer via Shader ──────────────────────────────────────

    #[test]
    fn shader_default() {
        let shader = Shader::new();
        assert!(!shader.vertex_cache.derived_valid);
        assert!(!shader.fragment_cache.derived_valid);
    }

    // ── Vertex shader ────────────────────────────────────────────────

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

        // With identity matrices, position should be unchanged
        assert!((result.position.x - 1.0).abs() < 1e-5);
        assert!((result.position.y - 2.0).abs() < 1e-5);
        assert!((result.position.z - 3.0).abs() < 1e-5);
        assert!((result.position.w - 1.0).abs() < 1e-5);

        // Normal should be unchanged
        assert!((result.normal.y - 1.0).abs() < 1e-5);

        // Position IS the clip position after vertex shader
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

        // Position translated by (10, 0, 0)
        assert!((result.position.x - 10.0).abs() < 1e-5);

        // world_position should hold world-space position
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

    // ── Fragment shader ──────────────────────────────────────────────

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

        // Should produce a non-zero, non-white color
        assert!(color.r() > 0 || color.g() > 0 || color.b() > 0);
        assert_eq!(color.a(), 255);
    }

    #[test]
    fn fragment_shader_no_light_produces_ambient_only() {
        let shader = Shader::new();
        // No lights set — light_dirs will be empty, only ambient contributes

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

        // ambient * 0.1 = white * 0.1 → ~25
        assert!(color.r() > 0);
        assert!(color.r() < 50);
    }

    #[test]
    fn fragment_shader_phong_formula() {
        let mut shader = Shader::new();

        // Light pointing in +Z, fragment normal facing +Z
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
            color: Color::new(255, 255, 255, 255), // white base
            depth: 0.5,
            world_position: Vec3::ZERO,
        };
        let material = Material {
            shininess: 32.0,
            ..Material::default()
        };

        let color = shader.fragment_shader(&fragment, &material);
        // With light direction aligned with normal, diffuse intensity = 1.0
        // ambient*0.1 + diffuse + specular*0.2 should be significant
        assert!(color.r() > 100);
    }

    // ── Specular LUT ─────────────────────────────────────────────────

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
        // For shininess=1, cos_theta^1 = cos_theta
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
        // Low cos_theta values should be near 0 with high shininess
        assert!(lut.values[0] < 1e-6); // cos_theta = 0
        assert!(lut.values[128] < 0.01); // cos_theta = 0.502 → 0.502^128 ≈ 0
                                         // High cos_theta should be closer to 1
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
        // Mid-range should produce a reasonable value via LUT interpolation
        let val = shader.evaluate_specular(0.5, 1.0);
        // For shininess=1, specular = cos_theta = 0.5
        assert!((val - 0.5).abs() < 0.01);
    }

    #[test]
    fn specular_lut_caching() {
        let shader = Shader::new();
        // First call builds the LUT
        let val1 = shader.evaluate_specular(0.5, 32.0);
        // Second call should hit the cache
        let val2 = shader.evaluate_specular(0.5, 32.0);
        assert!((val1 - val2).abs() < 1e-10);

        // Verify cache has exactly one entry
        let cache = shader
            .specular_lut_cache
            .read()
            .expect("specular LUT lock poisoned");
        assert_eq!(cache.len(), 1);
    }

    #[test]
    fn specular_lut_shared_across_clones() {
        let shader = Shader::new();
        // Build a LUT entry
        let _ = shader.evaluate_specular(0.5, 32.0);

        // Clone shares the same Arc
        let shader2 = shader.clone();
        {
            let cache = shader2
                .specular_lut_cache
                .read()
                .expect("specular LUT lock poisoned");
            assert_eq!(cache.len(), 1);
        }

        // Building a new entry via the clone is visible to original
        let _ = shader2.evaluate_specular(0.5, 64.0);
        {
            let cache = shader
                .specular_lut_cache
                .read()
                .expect("specular LUT lock poisoned");
            assert_eq!(cache.len(), 2);
        }
    }

    // ── Texture sampling ─────────────────────────────────────────────

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
        // UV (0, 0) → pixel (0, 0) → red
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
        // UV (1.5, 2.7) should wrap: u = 0.5, v = 0.7 → still pixel (0, 0) for 1×1
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
        // Negative UV: -0.3 - floor(-0.3) = -0.3 - (-1) = 0.7
        let c = Shader::sample_texture(&texture, Vec2::new(-0.3, -0.3));
        assert_eq!(c, Color::new(42, 43, 44, 255));
    }

    // ── Cache invalidation ───────────────────────────────────────────

    #[test]
    fn cache_invalidation_on_matrix_update() {
        let mut shader = Shader::new();
        shader.set_uniform("modelMatrix", Mat4::IDENTITY);
        shader.set_uniform("viewMatrix", Mat4::IDENTITY);
        shader.set_uniform("projectionMatrix", Mat4::IDENTITY);

        // After setting all three, cache should be valid
        assert!(shader.vertex_cache.derived_valid);

        // Updating model should invalidate, then auto-recalculate
        let new_model = Mat4::from_translation(Vec3::new(1.0, 0.0, 0.0));
        shader.set_uniform("modelMatrix", new_model);

        // Still valid because all three exist → auto-recalculated
        assert!(shader.vertex_cache.derived_valid);
        // MVP should reflect new model
        let expected_mvp = Mat4::IDENTITY * Mat4::IDENTITY * new_model;
        let diff = (shader.vertex_cache.mvp - expected_mvp).abs_diff_eq(Mat4::ZERO, 1e-6);
        assert!(diff);
    }

    #[test]
    fn cache_not_valid_with_partial_matrices() {
        let mut shader = Shader::new();
        shader.set_uniform("modelMatrix", Mat4::IDENTITY);
        // Only model set, not view or projection
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
        // Only lights set, not camera
        assert!(!shader.fragment_cache.derived_valid);

        shader.set_uniform("cameraPos", Vec3::new(0.0, 0.0, 5.0));
        // Now both lights and camera are set → auto-recalculated
        assert!(shader.fragment_cache.derived_valid);
        assert_eq!(shader.fragment_cache.light_dirs_normalized.len(), 1);
    }

    #[test]
    fn prepare_caches_fills_from_uniform_buffer() {
        let mut shader = Shader::new();
        // Set uniforms directly in the buffer without triggering cache updates
        // (this simulates the PrepareUniformCaches pathway)
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

    // ── set_lights convenience ───────────────────────────────────────

    #[test]
    fn set_lights_stores_in_buffer() {
        let mut shader = Shader::new();
        shader.set_lights(&[Light::default(), Light::default()]);
        assert!(shader.uniform_buffer.has("lights"));
        assert_eq!(shader.uniform_buffer.get_lights("lights").unwrap().len(), 2);
    }
}
