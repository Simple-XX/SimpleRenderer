// 视口与渲染配置

#[derive(Debug, Clone, Copy)]
pub struct Viewport {
    pub x: u32,
    pub y: u32,
    pub width: u32,
    pub height: u32,
}

impl Viewport {
    pub fn new(width: u32, height: u32) -> Self {
        Self {
            x: 0,
            y: 0,
            width,
            height,
        }
    }

    pub fn aspect_ratio(&self) -> f32 {
        self.width as f32 / self.height.max(1) as f32
    }
}

#[derive(Debug, Clone)]
pub struct RenderConfig {
    pub width: u32,
    pub height: u32,
    pub vsync: bool,
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_viewport_new() {
        let vp = Viewport::new(800, 600);
        assert_eq!(vp.x, 0);
        assert_eq!(vp.y, 0);
        assert_eq!(vp.width, 800);
        assert_eq!(vp.height, 600);
    }

    #[test]
    fn test_viewport_aspect_ratio() {
        let vp = Viewport::new(1920, 1080);
        let ratio = vp.aspect_ratio();
        assert!((ratio - 16.0 / 9.0).abs() < 0.01);
    }

    #[test]
    fn test_viewport_aspect_ratio_zero_height() {
        // height 为 0 时不应 panic，max(1) 保护
        let vp = Viewport::new(800, 0);
        assert_eq!(vp.aspect_ratio(), 800.0);
    }
}
