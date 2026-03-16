pub mod base;
pub mod deferred;
pub mod per_triangle;
#[cfg(test)]
pub(crate) mod test_utils;
pub mod tile_based;
pub mod tile_based_deferred;
pub mod tile_common;

use crate::model::Model;
use crate::shader::Shader;

/// Common interface for all rendering strategies.
///
/// Mirrors C++ `RendererBase::Render()`.
pub trait Renderer: Send {
    /// Render `model` into `out_buffer` (row-major, `width × height` pixels).
    ///
    /// Returns `Ok(())` on success.
    fn render(
        &self,
        model: &Model,
        shader: &Shader,
        out_buffer: &mut [u32],
        width: usize,
        height: usize,
    ) -> crate::error::Result<()>;
}
