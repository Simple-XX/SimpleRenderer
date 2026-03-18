// Copyright The SimpleGameEngine Contributors


use crate::model::Model;
use crate::renderers::deferred::DeferredRenderer;
use crate::renderers::per_triangle::PerTriangleRenderer;
use crate::renderers::tile_based::TileBasedRenderer;
use crate::renderers::tile_based_deferred::TileBasedDeferredRenderer;
use crate::renderers::Renderer;
use crate::shader::Shader;
use log::info;

#[non_exhaustive]
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
#[repr(u8)]
pub enum RenderingMode {
    PerTriangle = 0,
    TileBased = 1,
    Deferred = 2,
    TileBasedDeferred = 3,
}

impl std::fmt::Display for RenderingMode {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        match self {
            Self::PerTriangle => write!(f, "PER_TRIANGLE"),
            Self::TileBased => write!(f, "TILE_BASED"),
            Self::Deferred => write!(f, "DEFERRED"),
            Self::TileBasedDeferred => write!(f, "TILE_BASED_DEFERRED"),
        }
    }
}

impl TryFrom<u8> for RenderingMode {
    type Error = u8;

    fn try_from(value: u8) -> std::result::Result<Self, Self::Error> {
        match value {
            0 => Ok(Self::PerTriangle),
            1 => Ok(Self::TileBased),
            2 => Ok(Self::Deferred),
            3 => Ok(Self::TileBasedDeferred),
            other => Err(other),
        }
    }
}

pub struct SimpleRenderer {
    width: usize,
    height: usize,
    mode: RenderingMode,
    renderer: Box<dyn Renderer>,
    early_z_enabled: bool,
    tile_size: usize,
}

impl SimpleRenderer {
    pub fn new(width: usize, height: usize) -> Self {
        let mode = RenderingMode::TileBased;
        let early_z_enabled = true;
        let tile_size = 64;
        let renderer = Self::create_renderer(mode, width, height, early_z_enabled, tile_size);
        Self {
            width,
            height,
            mode,
            renderer,
            early_z_enabled,
            tile_size,
        }
    }

    pub fn draw_model(
        &mut self,
        model: &Model,
        shader: &mut Shader,
        buffer: &mut [u32],
    ) -> crate::error::Result<()> {
        shader.prepare_caches();
        self.renderer
            .render(model, shader, buffer, self.width, self.height)
    }

    pub fn set_rendering_mode(&mut self, mode: RenderingMode) {
        self.mode = mode;
        info!("Rendering mode set to: {}", mode);
        self.renderer = Self::create_renderer(
            mode,
            self.width,
            self.height,
            self.early_z_enabled,
            self.tile_size,
        );
    }

    pub fn rendering_mode(&self) -> RenderingMode {
        self.mode
    }

    pub fn set_early_z_enabled(&mut self, enabled: bool) {
        self.early_z_enabled = enabled;
        if self.mode == RenderingMode::TileBased {
            self.renderer = Self::create_renderer(
                self.mode,
                self.width,
                self.height,
                self.early_z_enabled,
                self.tile_size,
            );
        }
    }

    pub fn set_tile_size(&mut self, size: usize) {
        self.tile_size = size;
        if self.mode == RenderingMode::TileBased || self.mode == RenderingMode::TileBasedDeferred {
            self.renderer = Self::create_renderer(
                self.mode,
                self.width,
                self.height,
                self.early_z_enabled,
                self.tile_size,
            );
        }
    }

    fn create_renderer(
        mode: RenderingMode,
        width: usize,
        height: usize,
        early_z: bool,
        tile_size: usize,
    ) -> Box<dyn Renderer> {
        match mode {
            RenderingMode::PerTriangle => Box::new(PerTriangleRenderer::new(width, height)),
            RenderingMode::TileBased => Box::new(TileBasedRenderer::with_options(
                width, height, tile_size, early_z,
            )),
            RenderingMode::Deferred => Box::new(DeferredRenderer::new(width, height)),
            RenderingMode::TileBasedDeferred => Box::new(
                TileBasedDeferredRenderer::with_tile_size(width, height, tile_size),
            ),
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn default_mode_is_tile_based() {
        let r = SimpleRenderer::new(100, 100);
        assert_eq!(r.rendering_mode(), RenderingMode::TileBased);
    }

    #[test]
    fn set_rendering_mode() {
        let mut r = SimpleRenderer::new(100, 100);
        r.set_rendering_mode(RenderingMode::Deferred);
        assert_eq!(r.rendering_mode(), RenderingMode::Deferred);
    }

    #[test]
    fn rendering_mode_display() {
        assert_eq!(format!("{}", RenderingMode::PerTriangle), "PER_TRIANGLE");
        assert_eq!(format!("{}", RenderingMode::TileBased), "TILE_BASED");
        assert_eq!(format!("{}", RenderingMode::Deferred), "DEFERRED");
        assert_eq!(
            format!("{}", RenderingMode::TileBasedDeferred),
            "TILE_BASED_DEFERRED"
        );
    }

    #[test]
    fn all_modes_can_be_created() {
        let modes = [
            RenderingMode::PerTriangle,
            RenderingMode::TileBased,
            RenderingMode::Deferred,
            RenderingMode::TileBasedDeferred,
        ];
        for mode in &modes {
            let mut r = SimpleRenderer::new(100, 100);
            r.set_rendering_mode(*mode);
            assert_eq!(r.rendering_mode(), *mode);
        }
    }

    #[test]
    fn try_from_u8_valid() {
        assert_eq!(RenderingMode::try_from(0), Ok(RenderingMode::PerTriangle));
        assert_eq!(RenderingMode::try_from(1), Ok(RenderingMode::TileBased));
        assert_eq!(RenderingMode::try_from(2), Ok(RenderingMode::Deferred));
        assert_eq!(
            RenderingMode::try_from(3),
            Ok(RenderingMode::TileBasedDeferred)
        );
    }

    #[test]
    fn try_from_u8_invalid() {
        assert_eq!(RenderingMode::try_from(4), Err(4));
        assert_eq!(RenderingMode::try_from(255), Err(255));
    }
}
