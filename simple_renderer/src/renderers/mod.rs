pub(crate) mod base;
pub(crate) mod deferred;
pub(crate) mod per_triangle;
#[cfg(test)]
pub(crate) mod test_utils;
pub(crate) mod tile_based;
pub(crate) mod tile_based_deferred;
pub(crate) mod tile_common;

use crate::model::Model;
use crate::shader::Shader;

/// Common interface for all rendering strategies.
pub trait Renderer: Send {
    /// Render `model` into `out_buffer` (row-major, `width × height` pixels).
    ///
    /// Returns `Ok(())` on success.
    fn render(
        &mut self,
        model: &Model,
        shader: &Shader,
        out_buffer: &mut [u32],
        width: usize,
        height: usize,
    ) -> crate::error::Result<()>;
}
