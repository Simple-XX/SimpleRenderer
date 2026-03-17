// Copyright The SimpleRenderer Contributors

//! # SimpleRenderer
//!
//! An educational software renderer that implements a simplified GPU rendering
//! pipeline entirely in safe Rust. Demonstrates vertex processing, rasterization,
//! fragment shading, and multi-buffered display output.
//!
//! ## Quick Start
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
//! ## Architecture
//!
//! The pipeline mirrors a typical GPU: **Vertex Shader → Perspective Division →
//! Viewport Transform → Rasterization → Fragment Shader → Depth Test → Framebuffer**.
//!
//! Four rendering strategies are available via [`RenderingMode`]:
//! - [`RenderingMode::PerTriangle`] — chunk-parallel forward rendering
//! - [`RenderingMode::TileBased`] — tile-parallel with optional early-Z
//! - [`RenderingMode::Deferred`] — deferred shading (shade only depth winners)
//! - [`RenderingMode::TileBasedDeferred`] — tile-parallel deferred (best of both)

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

// Convenience re-exports
pub use buffer::Buffer;
pub use color::Color;
pub use error::{RendererError, Result};
pub use light::Light;
pub use model::Model;
pub use renderer::{RenderingMode, SimpleRenderer};
pub use shader::Shader;
pub use vertex::Vertex;
