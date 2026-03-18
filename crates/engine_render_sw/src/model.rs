use std::collections::HashMap;
use std::path::{Path, PathBuf};
use std::sync::Arc;

use tracing::warn;

use crate::color::Color;
use crate::error::{RendererError, Result};
use crate::face::Face;
use crate::material::{Material, Texture};
use crate::math::{Vec2, Vec3, Vec4};
use crate::vertex::Vertex;

/// 通过 tobj 加载的 OBJ/MTL 模型。
///
/// OBJ 模型加载器：从 Wavefront .obj 文件中加载顶点、面和材质。
#[derive(Debug, Clone)]
pub struct Model {
    vertices: Vec<Vertex>,
    faces: Vec<Face>,
    directory: String,
}

impl Model {
    /// 从 `path` 加载 OBJ 模型。
    ///
    /// 使用 `tobj` 并启用三角化和单索引模式。
    /// 材质和纹理从同一目录加载。
    /// 纹理缓存可防止重复加载图像。
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
            tobj::load_obj(obj_path, &load_options).map_err(|e| RendererError::ModelLoad {
                path: path.to_string(),
                source: Box::new(e),
            })?;

        // 加载材质 ── 失败时发出警告，回退到空向量。
        let materials = match materials_result {
            Ok(mats) => mats,
            Err(e) => {
                warn!("Failed to load materials for {}: {}", path, e);
                Vec::new()
            }
        };

        let mut vertices: Vec<Vertex> = Vec::new();
        let mut faces: Vec<Face> = Vec::new();
        let mut texture_cache: HashMap<PathBuf, Arc<Texture>> = HashMap::new();

        for model in &models {
            let mesh = &model.mesh;
            let vertex_offset = vertices.len();
            let num_vertices = mesh.positions.len() / 3;
            let has_normals = !mesh.normals.is_empty();
            let has_texcoords = !mesh.texcoords.is_empty();

            // 为此网格构建顶点
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

            // 为此网格构建材质（所有面共享）
            let material = Arc::new(Self::build_material(
                mesh.material_id,
                &materials,
                &directory,
                &mut texture_cache,
            ));

            // 从索引构建面（三角形）
            let num_faces = mesh.indices.len() / 3;
            for i in 0..num_faces {
                let i0 = mesh.indices[i * 3] as usize + vertex_offset;
                let i1 = mesh.indices[i * 3 + 1] as usize + vertex_offset;
                let i2 = mesh.indices[i * 3 + 2] as usize + vertex_offset;
                faces.push(Face {
                    indices: [i0, i1, i2],
                    material: Arc::clone(&material),
                });
            }
        }

        tracing::info!(
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

    /// 从 tobj 材质数据构建 `Material`，通过缓存加载纹理。
    fn build_material(
        material_id: Option<usize>,
        materials: &[tobj::Material],
        directory: &str,
        texture_cache: &mut HashMap<PathBuf, Arc<Texture>>,
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

    /// 从相对路径加载纹理，使用缓存避免重复加载。
    fn load_texture_cached(
        texture_name: Option<&str>,
        directory: &str,
        cache: &mut HashMap<PathBuf, Arc<Texture>>,
    ) -> Option<Arc<Texture>> {
        let name = texture_name?;
        if name.is_empty() {
            return None;
        }

        let full_path = PathBuf::from(directory).join(name);

        if let Some(tex) = cache.get(&full_path) {
            return Some(Arc::clone(tex));
        }

        match Texture::load_from_file(&full_path) {
            Ok(tex) => {
                let tex = Arc::new(tex);
                cache.insert(full_path, Arc::clone(&tex));
                Some(tex)
            }
            Err(e) => {
                warn!("Failed to load texture '{}': {}", full_path.display(), e);
                None
            }
        }
    }

    /// 模型中的所有顶点。
    pub fn vertices(&self) -> &[Vertex] {
        &self.vertices
    }

    /// 模型中的所有三角形面。
    pub fn faces(&self) -> &[Face] {
        &self.faces
    }

    /// 模型文件所在的目录。
    pub fn model_path(&self) -> &str {
        &self.directory
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    /// 加载不存在的路径必须返回 Err，不能 panic。
    #[test]
    fn load_nonexistent_file_returns_error() {
        let result = Model::load("/nonexistent/path/model.obj");
        assert!(result.is_err());
        match result.unwrap_err() {
            RendererError::ModelLoad { path, .. } => {
                assert!(!path.is_empty());
            }
            other => panic!("Expected ModelLoad error, got: {:?}", other),
        }
    }

    /// 加载没有 MTL 文件的模型 ── 应仍能成功加载并使用默认材质。
    #[test]
    fn load_model_without_mtl_uses_defaults() {
        let path = concat!(env!("CARGO_MANIFEST_DIR"), "/../../assets/models/cube2.obj");
        let result = Model::load(path);
        assert!(
            result.is_ok(),
            "Model without MTL should load: {:?}",
            result.err()
        );
    }

    /// 茶壶加载综合测试 ── 单次加载，多项断言。
    /// 避免并行加载模型 11 次（有 OOM 风险）。
    #[test]
    fn load_teapot_comprehensive() {
        let path = concat!(
            env!("CARGO_MANIFEST_DIR"),
            "/../../assets/models/utah-teapot-texture/teapot.obj"
        );
        let model = Model::load(path).expect("Failed to load teapot.obj");

        // 基本结构
        assert!(!model.vertices().is_empty(), "Model should have vertices");
        assert!(!model.faces().is_empty(), "Model should have faces");

        // 面索引必须有效
        let n = model.vertices().len();
        for (i, face) in model.faces().iter().enumerate() {
            for &idx in &face.indices {
                assert!(
                    idx < n,
                    "Face {} index {} out of range (vertices: {})",
                    i,
                    idx,
                    n
                );
            }
        }

        // 顶点：w=1.0，颜色=WHITE
        for (i, v) in model.vertices().iter().enumerate() {
            assert!(
                (v.position.w - 1.0).abs() < f32::EPSILON,
                "Vertex {} position.w = {}, expected 1.0",
                i,
                v.position.w
            );
            assert_eq!(v.color, Color::WHITE, "Vertex {} color should be WHITE", i);
        }

        // 法线和 UV 存在
        let has_nonzero_normal = model.vertices().iter().any(|v| v.normal.length() > 0.0);
        assert!(
            has_nonzero_normal,
            "At least some vertices should have non-zero normals"
        );

        let has_nonzero_uv = model.vertices().iter().any(|v| v.tex_coords.length() > 0.0);
        assert!(
            has_nonzero_uv,
            "At least some vertices should have non-zero UVs"
        );

        // 材质：shininess > 0（teapot.mtl 中 Ns 为 20.0）
        let mat = &model.faces()[0].material;
        assert!(mat.shininess > 0.0, "shininess should be > 0");

        // 纹理：环境光（brick.png）、漫反射（brick.png）、镜面反射（brick-specular.png）
        assert!(
            mat.ambient_texture.is_some(),
            "Teapot should have an ambient texture"
        );
        assert!(
            mat.diffuse_texture.is_some(),
            "Teapot should have a diffuse texture"
        );
        if let Some(dtex) = mat.diffuse_texture.as_ref() {
            assert!(dtex.width > 0);
            assert!(dtex.height > 0);
        }
        assert!(
            mat.specular_texture.is_some(),
            "Teapot should have a specular texture"
        );

        // 目录
        assert!(
            model.model_path().contains("utah-teapot-texture"),
            "Directory '{}' should contain 'utah-teapot-texture'",
            model.model_path()
        );
    }
}
