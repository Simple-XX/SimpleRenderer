// 渲染后端 trait 定义

use crate::frame::{FrameData, FrameOutput};
use crate::mesh::{MeshData, MeshHandle};
use crate::texture::{TextureData, TextureHandle};
use crate::viewport::RenderConfig;

#[derive(Debug, thiserror::Error)]
pub enum RenderError {
    #[error("渲染初始化失败: {0}")]
    InitFailed(String),
    #[error("渲染错误: {0}")]
    RenderFailed(String),
    #[error("资源创建失败: {0}")]
    ResourceError(String),
}

pub type Result<T> = std::result::Result<T, RenderError>;

pub trait RenderBackend: Send + 'static {
    fn init(&mut self, config: RenderConfig) -> Result<()>;
    fn render_frame(&mut self, frame: &FrameData) -> Result<FrameOutput>;
    fn resize(&mut self, width: u32, height: u32);
    fn create_mesh(&mut self, data: &MeshData) -> Result<MeshHandle>;
    fn create_texture(&mut self, data: &TextureData) -> Result<TextureHandle>;
    fn read_pixels(&self) -> &[u8];
    fn name(&self) -> &str;
}
