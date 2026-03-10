use std::collections::HashMap;
use std::path::{Path, PathBuf};

use log::warn;

use crate::color::Color;
use crate::error::{RendererError, Result};
use crate::face::Face;
use crate::material::{Material, Texture};
use crate::math::{Vec2, Vec3, Vec4};
use crate::vertex::Vertex;

/// An OBJ/MTL model loaded via tobj.
///
/// Mirrors the C++ `Model` class: loads vertices, faces and materials
/// (including texture maps) from Wavefront OBJ files.
#[derive(Debug, Clone)]
pub struct Model {
    vertices: Vec<Vertex>,
    faces: Vec<Face>,
    directory: String,
}

impl Model {
    /// Load an OBJ model from `path`.
    ///
    /// Uses `tobj` with triangulation and single-index enabled.
    /// Materials and textures are loaded from the same directory.
    /// A texture cache prevents duplicate image loads.
    pub fn load(path: &str) -> Result<Self> {
        let obj_path = Path::new(path);
        let directory = obj_path
            .parent()
            .unwrap_or_else(|| Path::new("."))
            .to_string_lossy()
            .into_owned();

        let load_options = tobj::LoadOptions {
            triangulate: true,
            single_index: true,
            ..Default::default()
        };

        let (models, materials_result) =
            tobj::load_obj(obj_path, &load_options).map_err(|e| {
                RendererError::ModelLoad(format!("{}: {}", path, e))
            })?;

        // Load materials — warn on failure, fall back to empty vec.
        let materials = match materials_result {
            Ok(mats) => mats,
            Err(e) => {
                warn!("Failed to load materials for {}: {}", path, e);
                Vec::new()
            }
        };

        let mut vertices: Vec<Vertex> = Vec::new();
        let mut faces: Vec<Face> = Vec::new();
        let mut texture_cache: HashMap<PathBuf, Texture> = HashMap::new();

        for model in &models {
            let mesh = &model.mesh;
            let vertex_offset = vertices.len();
            let num_vertices = mesh.positions.len() / 3;
            let has_normals = !mesh.normals.is_empty();
            let has_texcoords = !mesh.texcoords.is_empty();

            // Build vertices for this mesh
            for i in 0..num_vertices {
                let px = mesh.positions[i * 3];
                let py = mesh.positions[i * 3 + 1];
                let pz = mesh.positions[i * 3 + 2];

                let normal = if has_normals && i * 3 + 2 < mesh.normals.len() {
                    Vec3::new(
                        mesh.normals[i * 3],
                        mesh.normals[i * 3 + 1],
                        mesh.normals[i * 3 + 2],
                    )
                } else {
                    Vec3::ZERO
                };

                let tex_coords = if has_texcoords && i * 2 + 1 < mesh.texcoords.len() {
                    Vec2::new(mesh.texcoords[i * 2], mesh.texcoords[i * 2 + 1])
                } else {
                    Vec2::ZERO
                };

                vertices.push(Vertex::new(
                    Vec4::new(px, py, pz, 1.0),
                    normal,
                    tex_coords,
                    Color::WHITE,
                ));
            }

            // Build material for this mesh
            let material = Self::build_material(
                mesh.material_id,
                &materials,
                &directory,
                &mut texture_cache,
            );

            // Build faces (triangles) from indices
            let num_faces = mesh.indices.len() / 3;
            for i in 0..num_faces {
                let i0 = mesh.indices[i * 3] as usize + vertex_offset;
                let i1 = mesh.indices[i * 3 + 1] as usize + vertex_offset;
                let i2 = mesh.indices[i * 3 + 2] as usize + vertex_offset;
                faces.push(Face {
                    indices: [i0, i1, i2],
                    material: material.clone(),
                });
            }
        }

        log::info!(
            "Loaded model: {}, vertices: {}, faces: {}",
            path,
            vertices.len(),
            faces.len()
        );

        Ok(Self {
            vertices,
            faces,
            directory,
        })
    }

    /// Build a `Material` from tobj material data, loading textures via cache.
    fn build_material(
        material_id: Option<usize>,
        materials: &[tobj::Material],
        directory: &str,
        texture_cache: &mut HashMap<PathBuf, Texture>,
    ) -> Material {
        let mat_idx = match material_id {
            Some(idx) if idx < materials.len() => idx,
            _ => return Material::default(),
        };

        let tobj_mat = &materials[mat_idx];

        let ambient = tobj_mat
            .ambient
            .map(|a| Vec3::new(a[0], a[1], a[2]))
            .unwrap_or(Vec3::ZERO);

        let diffuse = tobj_mat
            .diffuse
            .map(|d| Vec3::new(d[0], d[1], d[2]))
            .unwrap_or(Vec3::ZERO);

        let specular = tobj_mat
            .specular
            .map(|s| Vec3::new(s[0], s[1], s[2]))
            .unwrap_or(Vec3::ZERO);

        let shininess = tobj_mat.shininess.unwrap_or(0.0);

        let ambient_texture = Self::load_texture_cached(
            tobj_mat.ambient_texture.as_deref(),
            directory,
            texture_cache,
        );

        let diffuse_texture = Self::load_texture_cached(
            tobj_mat.diffuse_texture.as_deref(),
            directory,
            texture_cache,
        );

        let specular_texture = Self::load_texture_cached(
            tobj_mat.specular_texture.as_deref(),
            directory,
            texture_cache,
        );

        Material {
            shininess,
            ambient,
            diffuse,
            specular,
            ambient_texture,
            diffuse_texture,
            specular_texture,
        }
    }

    /// Load a texture from a relative path, using a cache to avoid duplicates.
    fn load_texture_cached(
        texture_name: Option<&str>,
        directory: &str,
        cache: &mut HashMap<PathBuf, Texture>,
    ) -> Option<Texture> {
        let name = texture_name?;
        if name.is_empty() {
            return None;
        }

        let full_path = PathBuf::from(directory).join(name);

        if let Some(cached) = cache.get(&full_path) {
            return Some(cached.clone());
        }

        match Texture::load_from_file(&full_path) {
            Ok(tex) => {
                cache.insert(full_path, tex.clone());
                Some(tex)
            }
            Err(e) => {
                warn!("Failed to load texture '{}': {}", full_path.display(), e);
                None
            }
        }
    }

    /// All vertices in the model.
    pub fn vertices(&self) -> &[Vertex] {
        &self.vertices
    }

    /// All triangle faces in the model.
    pub fn faces(&self) -> &[Face] {
        &self.faces
    }

    /// Directory containing the model file.
    pub fn model_path(&self) -> &str {
        &self.directory
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    /// Loading a nonexistent path must return Err, not panic.
    #[test]
    fn load_nonexistent_file_returns_error() {
        let result = Model::load("/nonexistent/path/model.obj");
        assert!(result.is_err());
        match result.unwrap_err() {
            RendererError::ModelLoad(msg) => {
                assert!(!msg.is_empty());
            }
            other => panic!("Expected ModelLoad error, got: {:?}", other),
        }
    }

    /// Load a model without MTL file — should still succeed with default materials.
    #[test]
    fn load_model_without_mtl_uses_defaults() {
        let path = concat!(env!("CARGO_MANIFEST_DIR"), "/../obj/cube2.obj");
        let result = Model::load(path);
        assert!(result.is_ok(), "Model without MTL should load: {:?}", result.err());
    }

    /// Comprehensive test for teapot loading — single load, many assertions.
    /// Avoids loading the model 11 times in parallel (OOM risk).
    #[test]
    fn load_teapot_comprehensive() {
        let path = concat!(env!("CARGO_MANIFEST_DIR"), "/../obj/utah-teapot-texture/teapot.obj");
        let model = Model::load(path).expect("Failed to load teapot.obj");

        // Basic structure
        assert!(!model.vertices().is_empty(), "Model should have vertices");
        assert!(!model.faces().is_empty(), "Model should have faces");

        // Face indices must be valid
        let n = model.vertices().len();
        for (i, face) in model.faces().iter().enumerate() {
            for &idx in &face.indices {
                assert!(
                    idx < n,
                    "Face {} index {} out of range (vertices: {})",
                    i, idx, n
                );
            }
        }

        // Vertices: w=1.0, color=WHITE
        for (i, v) in model.vertices().iter().enumerate() {
            assert!(
                (v.position.w - 1.0).abs() < f32::EPSILON,
                "Vertex {} position.w = {}, expected 1.0",
                i, v.position.w
            );
            assert_eq!(
                v.color,
                Color::WHITE,
                "Vertex {} color should be WHITE",
                i
            );
        }

        // Normals and UVs present
        let has_nonzero_normal = model.vertices().iter().any(|v| v.normal.length() > 0.0);
        assert!(has_nonzero_normal, "At least some vertices should have non-zero normals");

        let has_nonzero_uv = model.vertices().iter().any(|v| v.tex_coords.length() > 0.0);
        assert!(has_nonzero_uv, "At least some vertices should have non-zero UVs");

        // Materials: shininess > 0 (teapot.mtl has Ns 20.0)
        let mat = &model.faces()[0].material;
        assert!(mat.shininess > 0.0, "shininess should be > 0");

        // Textures: ambient (brick.png), diffuse (brick.png), specular (brick-specular.png)
        assert!(
            mat.ambient_texture.is_some(),
            "Teapot should have an ambient texture"
        );
        assert!(
            mat.diffuse_texture.is_some(),
            "Teapot should have a diffuse texture"
        );
        let dtex = mat.diffuse_texture.as_ref().unwrap();
        assert!(dtex.width > 0);
        assert!(dtex.height > 0);
        assert!(
            mat.specular_texture.is_some(),
            "Teapot should have a specular texture"
        );

        // Directory
        assert!(
            model.model_path().contains("utah-teapot-texture"),
            "Directory '{}' should contain 'utah-teapot-texture'",
            model.model_path()
        );
    }
}
