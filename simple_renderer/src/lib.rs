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
pub(crate) mod uniform;
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
