// Copyright The SimpleGameEngine Contributors


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

/// 所有渲染策略的公共接口。
pub trait Renderer: Send {
    /// 将 `model` 渲染到 `out_buffer` 中（行优先，`width × height` 像素）。
    ///
    /// 成功时返回 `Ok(())`。
    fn render(
        &mut self,
        model: &Model,
        shader: &Shader,
        out_buffer: &mut [u32],
        width: usize,
        height: usize,
    ) -> crate::error::Result<()>;
}
