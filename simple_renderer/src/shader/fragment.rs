use crate::color::Color;
use crate::fragment::Fragment;
use crate::light::Light;
use crate::material::{Material, Texture};
use crate::math::{Vec2, Vec3};

use super::Shader;

// ── Caches ────────────────────────────────────────────────────────────────

/// Cached fragment-shader uniforms (lights + camera), avoiding per-fragment
/// HashMap lookups.
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
    // ── Fragment shader ───────────────────────────────────────────────

    /// Compute the final color of a fragment using Blinn-Phong shading.
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

        // View direction (from camera toward fragment)
        let view_dir = (fragment.world_position - camera_pos).normalize_or_zero();

        // Ambient (once, using ambient texture or base color)
        let ambient_rgb = if let Some(ref tex) = material.ambient_texture {
            color_to_vec(&Self::sample_texture(tex, uv))
        } else {
            base_color
        };

        // Accumulate diffuse + specular per light
        let mut diffuse_accum = Vec3::ZERO;
        let mut specular_accum = Vec3::ZERO;

        for ldir in light_dirs.iter() {
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

    // ── Fragment cache updates (private) ──────────────────────────────

    pub(super) fn update_fragment_cache_light(&mut self, name: &str, value: &Light) {
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

    pub(super) fn update_fragment_cache_lights(&mut self, name: &str, value: &[Light]) {
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

    pub(super) fn update_fragment_cache_vec3(&mut self, name: &str, value: Vec3) {
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

    // ── Fragment cache preparation (pre-render) ───────────────────────

    pub(super) fn prepare_fragment_cache(&mut self) {
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

    // ── Texture sampling (private static) ─────────────────────────────

    /// Sample a texture at the given UV coordinates with wrapping.
    pub(super) fn sample_texture(texture: &Texture, uv: Vec2) -> Color {
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
