// Copyright (c) Simple-XX/SimpleRenderer
// SPDX-License-Identifier: MIT

//! # SimpleRenderer
//!
//! 一个教育用途的软件渲染器，完全使用安全 Rust 实现了简化的 GPU 渲染
//! 管线。展示了顶点处理、光栅化、片段着色和多缓冲显示输出。
//!
//! ## 快速开始
//!
//! ```no_run
//! use simple_renderer::{SimpleRenderer, Model, Shader, Buffer, Light, RenderingMode};
//! use simple_renderer::vertex::Vertex;
//!
//! let mut renderer = SimpleRenderer::new(800, 600);
//! let model = Model::load("path/to/model.obj").unwrap();
//! let mut shader = Shader::new();
//! let mut buffer = Buffer::new(800, 600);
//! renderer.draw_model(&model, &mut shader, buffer.draw_buffer_mut()).unwrap();
//! ```
//!
//! ## 架构
//!
//! 该管线模拟了典型的 GPU 流程：**顶点着色器 → 透视除法 →
//! 视口变换 → 光栅化 → 片段着色器 → 深度测试 → 帧缓冲**。
//!
//! 通过 [`RenderingMode`] 可选择四种渲染策略：
//! - [`RenderingMode::PerTriangle`] — 按三角形分块并行的前向渲染
//! - [`RenderingMode::TileBased`] — 基于瓦片并行，支持可选的 Early-Z
//! - [`RenderingMode::Deferred`] — 延迟着色（仅对通过深度测试的像素着色）
//! - [`RenderingMode::TileBasedDeferred`] — 基于瓦片的延迟渲染（结合两者优势）

pub mod buffer;
pub mod color;
pub mod error;
pub(crate) mod face;
pub(crate) mod fragment;
pub mod light;
pub(crate) mod material;
pub(crate) mod math;
pub mod model;
pub(crate) mod rasterizer;
pub mod renderer;
pub(crate) mod renderers;
pub mod shader;
pub mod triple_buffer;
pub mod uniform;
pub mod vertex;

// 便捷的重导出
pub use buffer::Buffer;
pub use color::Color;
pub use error::{RendererError, Result};
pub use light::Light;
pub use model::Model;
pub use renderer::{RenderingMode, SimpleRenderer};
pub use shader::Shader;
pub use vertex::Vertex;
