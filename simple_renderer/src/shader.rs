use std::collections::HashMap;
use std::sync::RwLock;

use crate::color::Color;
use crate::fragment::Fragment;
use crate::light::Light;
use crate::material::{Material, Texture};
use crate::math::{Mat3, Mat4, Vec2, Vec3};
use crate::uniform::{UniformBuffer, UniformValue};
use crate::vertex::Vertex;

// ── Constants ─────────────────────────────────────────────────────────────

const SPECULAR_LUT_RESOLUTION: usize = 256;

// ── Specular LUT ──────────────────────────────────────────────────────────

/// Precomputed lookup table for `cos_theta^shininess`.
#[derive(Clone)]
struct SpecularLut {
    values: [f32; SPECULAR_LUT_RESOLUTION],
}

// ── Caches ────────────────────────────────────────────────────────────────

/// Cached vertex-shader matrices, avoiding per-vertex HashMap lookups.
///
/// Mirrors C++ `VertexUniformCache`.
#[derive(Clone)]
struct VertexUniformCache {
    model: Mat4,
    view: Mat4,
    projection: Mat4,
    model_view: Mat4,
    mvp: Mat4,
    normal: Mat3,
    has_model: bool,
    has_view: bool,
    has_projection: bool,
    derived_valid: bool,
}

impl Default for VertexUniformCache {
    fn default() -> Self {
        Self {
            model: Mat4::IDENTITY,
            view: Mat4::IDENTITY,
            projection: Mat4::IDENTITY,
            model_view: Mat4::IDENTITY,
            mvp: Mat4::IDENTITY,
            normal: Mat3::IDENTITY,
            has_model: false,
            has_view: false,
            has_projection: false,
            derived_valid: false,
        }
    }
}

/// Cached fragment-shader uniforms (lights + camera), avoiding per-fragment
/// HashMap lookups.
///
/// Mirrors C++ `FragmentUniformCache`.
#[derive(Clone)]
struct FragmentUniformCache {
    lights: Vec<Light>,
    camera_pos: Vec3,
    light_dirs_normalized: Vec<Vec3>,
    has_lights: bool,
    has_camera: bool,
    derived_valid: bool,
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

// ── Shader ────────────────────────────────────────────────────────────────

/// Vertex + fragment shader with uniform caching and specular LUT.
///
/// Port of C++ `Shader` class.
///
/// - `vertex_shader` takes `&mut self` (writes `frag_pos_varying`)
/// - `fragment_shader` takes `&self` (interior mutability via `RwLock` for
///   specular LUT cache)
pub struct Shader {
    uniform_buffer: UniformBuffer,
    frag_pos_varying: Vec3,
    vertex_cache: VertexUniformCache,
    fragment_cache: FragmentUniformCache,
    specular_lut_cache: RwLock<HashMap<u32, SpecularLut>>,
}

impl Shader {
    /// Create a new shader with default (empty) state.
    pub fn new() -> Self {
        Self {
            uniform_buffer: UniformBuffer::new(),
            frag_pos_varying: Vec3::ZERO,
            vertex_cache: VertexUniformCache::default(),
            fragment_cache: FragmentUniformCache::default(),
            specular_lut_cache: RwLock::new(HashMap::new()),
        }
    }

    // ── Uniform management ────────────────────────────────────────────

    /// Store a uniform and update relevant caches.
    ///
    /// Port of C++ `Shader::SetUniform`.
    pub fn set_uniform(&mut self, name: &str, value: impl Into<UniformValue>) {
        let value = value.into();
        self.uniform_buffer.set(name, value.clone());

        match &value {
            UniformValue::Mat4(m) => self.update_matrix_cache(name, *m),
            UniformValue::Light(l) => self.update_fragment_cache_light(name, l),
            UniformValue::Lights(ls) => self.update_fragment_cache_lights(name, ls),
            UniformValue::Vec3(v) => self.update_fragment_cache_vec3(name, *v),
            _ => {}
        }
    }

    /// Convenience: set multiple lights under the `"lights"` uniform.
    pub fn set_lights(&mut self, lights: &[Light]) {
        self.set_uniform("lights", lights.to_vec());
    }

    /// Ensure both vertex and fragment caches are up-to-date before a render
    /// pass. Call once before processing all vertices/fragments of a frame.
    ///
    /// Port of C++ `Shader::PrepareUniformCaches`.
    pub fn prepare_caches(&mut self) {
        self.prepare_vertex_cache();
        self.prepare_fragment_cache();
    }

    // ── Vertex shader ─────────────────────────────────────────────────

    /// Transform a vertex from model space to clip space.
    ///
    /// Writes `frag_pos_varying` (world-space position) for fragment shader.
    ///
    /// Port of C++ `Shader::VertexShader` (shader.cpp:54-101).
    pub fn vertex_shader(&mut self, vertex: &Vertex) -> Vertex {
        let (model, mvp, normal_mat) = if self.vertex_cache.derived_valid {
            (
                self.vertex_cache.model,
                self.vertex_cache.mvp,
                self.vertex_cache.normal,
            )
        } else {
            let model = self
                .uniform_buffer
                .get_mat4("modelMatrix")
                .unwrap_or(Mat4::IDENTITY);
            let view = self
                .uniform_buffer
                .get_mat4("viewMatrix")
                .unwrap_or(Mat4::IDENTITY);
            let projection = self
                .uniform_buffer
                .get_mat4("projectionMatrix")
                .unwrap_or(Mat4::IDENTITY);
            let mvp = projection * view * model;
            let normal_mat = Mat3::from_mat4(model).inverse().transpose();
            (model, mvp, normal_mat)
        };

        let position = vertex.position;
        let world_position = model * position;

        // Store world-space position for fragment shader
        self.frag_pos_varying = world_position.truncate();

        let clip_position = mvp * position;
        let transformed_normal = (normal_mat * vertex.normal).normalize_or_zero();

        Vertex::new(
            clip_position,
            transformed_normal,
            vertex.tex_coords,
            vertex.color,
        )
        .with_clip_position(clip_position)
    }

    // ── Fragment shader ───────────────────────────────────────────────

    /// Compute the final color of a fragment using Phong shading.
    ///
    /// Port of C++ `Shader::FragmentShader` (shader.cpp:289-365).
    pub fn fragment_shader(&self, fragment: &Fragment, material: &Material) -> Color {
        // Helper: Color → normalized Vec3 in [0, 1]
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

        // Get lights + camera from cache or uniform buffer
        let fallback_dirs;
        let light_dirs: &[Vec3] = if self.fragment_cache.derived_valid {
            &self.fragment_cache.light_dirs_normalized
        } else {
            // Fallback: read from uniform buffer
            fallback_dirs = if let Some(ls) = self.uniform_buffer.get_lights("lights") {
                ls.iter()
                    .map(|l| l.direction.normalize_or_zero())
                    .collect::<Vec<_>>()
            } else if let Some(l) = self.uniform_buffer.get_light("light") {
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
                .get_vec3("cameraPos")
                .unwrap_or(Vec3::ZERO)
        };

        // View direction (from camera toward fragment, matching C++)
        let view_dir = (self.frag_pos_varying - camera_pos).normalize_or_zero();

        // Ambient (once, using ambient texture or base color)
        let ambient_rgb = if let Some(ref tex) = material.ambient_texture {
            color_to_vec(&Self::sample_texture(tex, uv))
        } else {
            base_color
        };

        // Accumulate diffuse + specular per light
        let mut diffuse_accum = Vec3::ZERO;
        let mut specular_accum = Vec3::ZERO;

        for (i, ldir) in light_dirs.iter().enumerate() {
            let _ = i; // index used for lights[i] if needed
            let intensity = normal.dot(*ldir).max(0.0);

            // Diffuse
            let kd = if let Some(ref tex) = material.diffuse_texture {
                color_to_vec(&Self::sample_texture(tex, uv))
            } else {
                base_color
            };
            diffuse_accum += kd * intensity;

            // Specular (Blinn-Phong)
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

        // Final color: ambient * 0.1 + diffuse + specular * 0.2
        let out_rgb = ambient_rgb * 0.1 + diffuse_accum + specular_accum * 0.2;
        let r = out_rgb.x.clamp(0.0, 1.0);
        let g = out_rgb.y.clamp(0.0, 1.0);
        let b = out_rgb.z.clamp(0.0, 1.0);

        Color::from_normalized(r, g, b, 1.0)
    }

    // ── Cache updates (private) ───────────────────────────────────────

    /// Port of C++ `Shader::UpdateMatrixCache`.
    fn update_matrix_cache(&mut self, name: &str, value: Mat4) {
        match name {
            "modelMatrix" => {
                self.vertex_cache.model = value;
                self.vertex_cache.has_model = true;
            }
            "viewMatrix" => {
                self.vertex_cache.view = value;
                self.vertex_cache.has_view = true;
            }
            "projectionMatrix" => {
                self.vertex_cache.projection = value;
                self.vertex_cache.has_projection = true;
            }
            _ => return,
        }

        // Any base matrix update invalidates derived matrices
        self.vertex_cache.derived_valid = false;
        if self.vertex_cache.has_model
            && self.vertex_cache.has_view
            && self.vertex_cache.has_projection
        {
            self.recalculate_derived_matrices();
        }
    }

    /// Port of C++ `Shader::RecalculateDerivedMatrices`.
    fn recalculate_derived_matrices(&mut self) {
        self.vertex_cache.model_view = self.vertex_cache.view * self.vertex_cache.model;
        self.vertex_cache.mvp = self.vertex_cache.projection * self.vertex_cache.model_view;
        self.vertex_cache.normal = Mat3::from_mat4(self.vertex_cache.model)
            .inverse()
            .transpose();
        self.vertex_cache.derived_valid = true;
    }

    /// Port of C++ `Shader::UpdateFragmentCache` for single Light.
    fn update_fragment_cache_light(&mut self, name: &str, value: &Light) {
        if name != "light" {
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

    /// Port of C++ `Shader::UpdateFragmentCache` for `Vec<Light>`.
    fn update_fragment_cache_lights(&mut self, name: &str, value: &[Light]) {
        if name != "lights" {
            return;
        }
        self.fragment_cache.lights = value.to_vec();
        self.fragment_cache.has_lights = true;
        self.fragment_cache.derived_valid = false;
        if self.fragment_cache.has_lights && self.fragment_cache.has_camera {
            self.recalculate_fragment_derived();
        }
    }

    /// Port of C++ `Shader::UpdateFragmentCache` for Vec3 (camera position).
    fn update_fragment_cache_vec3(&mut self, name: &str, value: Vec3) {
        if name != "cameraPos" {
            return;
        }
        self.fragment_cache.camera_pos = value;
        self.fragment_cache.has_camera = true;
        self.fragment_cache.derived_valid = false;
        if self.fragment_cache.has_lights && self.fragment_cache.has_camera {
            self.recalculate_fragment_derived();
        }
    }

    /// Precompute normalized light directions.
    fn recalculate_fragment_derived(&mut self) {
        self.fragment_cache.light_dirs_normalized = self
            .fragment_cache
            .lights
            .iter()
            .map(|l| l.direction.normalize_or_zero())
            .collect();
        self.fragment_cache.derived_valid = true;
    }

    // ── Cache preparation (public, pre-render) ────────────────────────

    /// Port of C++ `Shader::PrepareVertexUniformCache`.
    fn prepare_vertex_cache(&mut self) {
        if self.vertex_cache.derived_valid {
            return;
        }
        if let (Some(model), Some(view), Some(proj)) = (
            self.uniform_buffer.get_mat4("modelMatrix"),
            self.uniform_buffer.get_mat4("viewMatrix"),
            self.uniform_buffer.get_mat4("projectionMatrix"),
        ) {
            self.vertex_cache.model = model;
            self.vertex_cache.view = view;
            self.vertex_cache.projection = proj;
            self.vertex_cache.has_model = true;
            self.vertex_cache.has_view = true;
            self.vertex_cache.has_projection = true;
            self.recalculate_derived_matrices();
        }
    }

    /// Port of C++ `Shader::PrepareFragmentUniformCache`.
    fn prepare_fragment_cache(&mut self) {
        if self.fragment_cache.derived_valid {
            return;
        }

        // Prefer multi-light path
        if let (Some(lights), Some(cam)) = (
            self.uniform_buffer.get_lights("lights"),
            self.uniform_buffer.get_vec3("cameraPos"),
        ) {
            self.fragment_cache.lights = lights.clone();
            self.fragment_cache.has_lights = true;
            self.fragment_cache.camera_pos = cam;
            self.fragment_cache.has_camera = true;
            self.recalculate_fragment_derived();
            return;
        }

        // Single-light fallback
        if let (Some(light), Some(cam)) = (
            self.uniform_buffer.get_light("light"),
            self.uniform_buffer.get_vec3("cameraPos"),
        ) {
            self.fragment_cache.lights = vec![light.clone()];
            self.fragment_cache.has_lights = true;
            self.fragment_cache.camera_pos = cam;
            self.fragment_cache.has_camera = true;
            self.recalculate_fragment_derived();
        }
    }

    // ── Specular LUT (private) ────────────────────────────────────────

    /// Build a lookup table for `cos_theta^shininess`.
    ///
    /// Port of C++ `Shader::BuildSpecularLUT` (shader.cpp:239-252).
    fn build_specular_lut(shininess: f32) -> SpecularLut {
        let mut values = [0.0_f32; SPECULAR_LUT_RESOLUTION];
        if shininess <= 0.0 {
            values.fill(1.0);
            return SpecularLut { values };
        }
        for i in 0..SPECULAR_LUT_RESOLUTION {
            let cos_theta = i as f32 / (SPECULAR_LUT_RESOLUTION - 1) as f32;
            values[i] = if cos_theta <= 0.0 {
                0.0
            } else {
                cos_theta.powf(shininess)
            };
        }
        SpecularLut { values }
    }

    /// Get or create a cached specular LUT for the given shininess.
    ///
    /// Port of C++ `Shader::GetSpecularLUT` (shader.cpp:254-268).
    /// Returns a clone of the LUT values (RwLock prevents returning a reference).
    fn get_specular_lut(&self, shininess: f32) -> [f32; SPECULAR_LUT_RESOLUTION] {
        let key = shininess.to_bits();

        // Try read lock first
        {
            let cache = self.specular_lut_cache.read().unwrap();
            if let Some(lut) = cache.get(&key) {
                return lut.values;
            }
        }

        // Build and insert under write lock
        let lut = Self::build_specular_lut(shininess);
        let values = lut.values;
        let mut cache = self.specular_lut_cache.write().unwrap();
        cache.entry(key).or_insert(lut);
        values
    }

    /// Evaluate specular contribution using cached LUT with linear interpolation.
    ///
    /// Port of C++ `Shader::EvaluateSpecular` (shader.cpp:270-287).
    fn evaluate_specular(&self, cos_theta: f32, shininess: f32) -> f32 {
        let cos_theta = cos_theta.clamp(0.0, 1.0);
        if shininess <= 0.0 {
            return 1.0;
        }
        if cos_theta <= 0.0 {
            return 0.0;
        }

        let lut = self.get_specular_lut(shininess);
        let scaled = cos_theta * (SPECULAR_LUT_RESOLUTION - 1) as f32;
        let index = scaled as usize;
        let frac = scaled - index as f32;

        let v0 = lut[index];
        let v1 = lut[(index + 1).min(SPECULAR_LUT_RESOLUTION - 1)];
        v0 + (v1 - v0) * frac
    }

    // ── Texture sampling (private static) ─────────────────────────────

    /// Sample a texture at the given UV coordinates with wrapping.
    ///
    /// Port of C++ `Shader::SampleTexture` (shader.cpp:376-395).
    fn sample_texture(texture: &Texture, uv: Vec2) -> Color {
        // Wrap to [0, 1]
        let u = uv.x - uv.x.floor();
        let v = uv.y - uv.y.floor();

        // Convert to pixel space
        let x = (u * texture.width as f32) as i32;
        let y = (v * texture.height as f32) as i32;

        // Clamp to bounds
        let x = x.clamp(0, texture.width as i32 - 1);
        let y = y.clamp(0, texture.height as i32 - 1);

        texture.get_pixel(x, y)
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
            frag_pos_varying: self.frag_pos_varying,
            vertex_cache: self.vertex_cache.clone(),
            fragment_cache: self.fragment_cache.clone(),
            specular_lut_cache: RwLock::new(self.specular_lut_cache.read().unwrap().clone()),
        }
    }
}

// ── Tests ─────────────────────────────────────────────────────────────────

#[cfg(test)]
mod tests {
    use super::*;
    use crate::math::Vec4;

    // ── UniformBuffer via Shader ──────────────────────────────────────

    #[test]
    fn shader_default() {
        let shader = Shader::new();
        assert_eq!(shader.frag_pos_varying, Vec3::ZERO);
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

        // Clip position should be set
        assert!(result.clip_position.is_some());
        let clip = result.clip_position.unwrap();
        assert!((clip.x - 1.0).abs() < 1e-5);
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

        // frag_pos_varying should hold world-space position
        assert!((shader.frag_pos_varying.x - 10.0).abs() < 1e-5);
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
        let cache = shader.specular_lut_cache.read().unwrap();
        assert_eq!(cache.len(), 1);
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
