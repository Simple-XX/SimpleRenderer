// Copyright The SimpleGameEngine Contributors


use crate::math::{Mat3, Mat4};
use crate::uniform;
use crate::vertex::Vertex;

use super::Shader;


/// 缓存的顶点着色器矩阵，避免每个顶点都进行 HashMap 查找。
#[derive(Clone)]
pub(crate) struct VertexUniformCache {
    pub(crate) model: Mat4,
    pub(crate) view: Mat4,
    pub(crate) projection: Mat4,
    pub(crate) model_view: Mat4,
    pub(crate) mvp: Mat4,
    pub(crate) normal: Mat3,
    pub(crate) has_model: bool,
    pub(crate) has_view: bool,
    pub(crate) has_projection: bool,
    pub(crate) derived_valid: bool,
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

impl Shader {

    /// 将顶点从模型空间变换到裁剪空间。
    ///
    /// 将世界空间坐标存储在返回的 Vertex 的 `world_position` 字段中。
    pub fn vertex_shader(&self, vertex: &Vertex) -> Vertex {
        let (model, mvp, normal_mat) = if self.vertex_cache.derived_valid {
            (
                self.vertex_cache.model,
                self.vertex_cache.mvp,
                self.vertex_cache.normal,
            )
        } else {
            let model = self
                .uniform_buffer
                .get_mat4(uniform::names::MODEL_MATRIX)
                .unwrap_or(Mat4::IDENTITY);
            let view = self
                .uniform_buffer
                .get_mat4(uniform::names::VIEW_MATRIX)
                .unwrap_or(Mat4::IDENTITY);
            let projection = self
                .uniform_buffer
                .get_mat4(uniform::names::PROJECTION_MATRIX)
                .unwrap_or(Mat4::IDENTITY);
            let mvp = projection * view * model;
            let normal_mat = Mat3::from_mat4(model).inverse().transpose();
            (model, mvp, normal_mat)
        };

        let position = vertex.position;
        let world_position = model * position;

        let clip_position = mvp * position;
        let transformed_normal = (normal_mat * vertex.normal).normalize_or_zero();

        Vertex::new(
            clip_position,
            transformed_normal,
            vertex.tex_coords,
            vertex.color,
        )
        .with_world_position(world_position.truncate())
    }


    pub(super) fn update_matrix_cache(&mut self, name: &str, value: Mat4) {
        match name {
            uniform::names::MODEL_MATRIX => {
                self.vertex_cache.model = value;
                self.vertex_cache.has_model = true;
            }
            uniform::names::VIEW_MATRIX => {
                self.vertex_cache.view = value;
                self.vertex_cache.has_view = true;
            }
            uniform::names::PROJECTION_MATRIX => {
                self.vertex_cache.projection = value;
                self.vertex_cache.has_projection = true;
            }
            _ => return,
        }

        // 任何基础矩阵的更新都会使派生矩阵失效
        self.vertex_cache.derived_valid = false;
        if self.vertex_cache.has_model
            && self.vertex_cache.has_view
            && self.vertex_cache.has_projection
        {
            self.recalculate_derived_matrices();
        }
    }

    fn recalculate_derived_matrices(&mut self) {
        self.vertex_cache.model_view = self.vertex_cache.view * self.vertex_cache.model;
        self.vertex_cache.mvp = self.vertex_cache.projection * self.vertex_cache.model_view;
        self.vertex_cache.normal = Mat3::from_mat4(self.vertex_cache.model)
            .inverse()
            .transpose();
        self.vertex_cache.derived_valid = true;
    }


    pub(super) fn prepare_vertex_cache(&mut self) {
        if self.vertex_cache.derived_valid {
            return;
        }
        if let (Some(model), Some(view), Some(proj)) = (
            self.uniform_buffer.get_mat4(uniform::names::MODEL_MATRIX),
            self.uniform_buffer.get_mat4(uniform::names::VIEW_MATRIX),
            self.uniform_buffer
                .get_mat4(uniform::names::PROJECTION_MATRIX),
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
}
