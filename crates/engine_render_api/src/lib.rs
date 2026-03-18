// 渲染后端抽象 API

pub mod backend;
pub mod color;
pub mod frame;
pub mod material;
pub mod mesh;
pub mod texture;
pub mod viewport;

pub use backend::{RenderBackend, RenderError, Result};
pub use color::Color;
pub use frame::{CameraData, DrawCall, FrameData, FrameOutput, LightData, LightKind};
pub use material::{MaterialData, MaterialHandle};
pub use mesh::{MeshData, MeshHandle};
pub use texture::{TextureData, TextureFormat, TextureHandle};
pub use viewport::{RenderConfig, Viewport};
