pub mod base;
pub mod per_triangle;
pub mod deferred;
pub mod tile_common;
pub mod tile_based;
pub mod tile_based_deferred;

use crate::model::Model;
use crate::shader::Shader;

/// Common interface for all rendering strategies.
///
/// Mirrors C++ `RendererBase::Render()`.
pub trait Renderer: Send {
    /// Render `model` into `out_buffer` (row-major, `width × height` pixels).
    ///
    /// Returns `true` on success.
    fn render(
        &self,
        model: &Model,
        shader: &Shader,
        out_buffer: &mut [u32],
        width: usize,
        height: usize,
    ) -> bool;
}
