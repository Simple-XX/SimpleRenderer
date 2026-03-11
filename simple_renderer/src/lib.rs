pub mod buffer;
pub mod color;
pub mod error;
pub mod face;
pub mod fragment;
pub mod light;
pub mod material;
pub mod math;
pub mod model;
pub mod rasterizer;
pub mod renderer;
pub mod renderers;
pub mod shader;
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
