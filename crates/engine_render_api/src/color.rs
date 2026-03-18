// 颜色类型定义 — f32 RGBA 颜色表示

#[derive(Debug, Clone, Copy, PartialEq)]
pub struct Color {
    pub r: f32,
    pub g: f32,
    pub b: f32,
    pub a: f32,
}

impl Color {
    pub const WHITE: Self = Self {
        r: 1.0,
        g: 1.0,
        b: 1.0,
        a: 1.0,
    };

    pub const BLACK: Self = Self {
        r: 0.0,
        g: 0.0,
        b: 0.0,
        a: 1.0,
    };

    pub const RED: Self = Self {
        r: 1.0,
        g: 0.0,
        b: 0.0,
        a: 1.0,
    };

    pub const GREEN: Self = Self {
        r: 0.0,
        g: 1.0,
        b: 0.0,
        a: 1.0,
    };

    pub const BLUE: Self = Self {
        r: 0.0,
        g: 0.0,
        b: 1.0,
        a: 1.0,
    };

    pub fn new(r: f32, g: f32, b: f32, a: f32) -> Self {
        Self { r, g, b, a }
    }

    /// 转换为 8 位 RGBA 数组，分量钳制到 [0.0, 1.0] 后乘以 255
    pub fn to_rgba8(&self) -> [u8; 4] {
        [
            (self.r.clamp(0.0, 1.0) * 255.0) as u8,
            (self.g.clamp(0.0, 1.0) * 255.0) as u8,
            (self.b.clamp(0.0, 1.0) * 255.0) as u8,
            (self.a.clamp(0.0, 1.0) * 255.0) as u8,
        ]
    }

    pub fn from_rgba8(rgba: [u8; 4]) -> Self {
        Self {
            r: rgba[0] as f32 / 255.0,
            g: rgba[1] as f32 / 255.0,
            b: rgba[2] as f32 / 255.0,
            a: rgba[3] as f32 / 255.0,
        }
    }
}

impl Default for Color {
    fn default() -> Self {
        Self::BLACK
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_color_new() {
        let c = Color::new(0.5, 0.6, 0.7, 0.8);
        assert_eq!(c.r, 0.5);
        assert_eq!(c.g, 0.6);
        assert_eq!(c.b, 0.7);
        assert_eq!(c.a, 0.8);
    }

    #[test]
    fn test_color_constants() {
        assert_eq!(Color::WHITE, Color::new(1.0, 1.0, 1.0, 1.0));
        assert_eq!(Color::BLACK, Color::new(0.0, 0.0, 0.0, 1.0));
        assert_eq!(Color::RED, Color::new(1.0, 0.0, 0.0, 1.0));
        assert_eq!(Color::GREEN, Color::new(0.0, 1.0, 0.0, 1.0));
        assert_eq!(Color::BLUE, Color::new(0.0, 0.0, 1.0, 1.0));
    }

    #[test]
    fn test_color_default_is_black() {
        assert_eq!(Color::default(), Color::BLACK);
    }

    #[test]
    fn test_to_rgba8() {
        assert_eq!(Color::WHITE.to_rgba8(), [255, 255, 255, 255]);
        assert_eq!(Color::BLACK.to_rgba8(), [0, 0, 0, 255]);
        let mid = Color::new(0.5, 0.5, 0.5, 1.0);
        // 0.5 * 255 = 127.5 → 127 as u8
        assert_eq!(mid.to_rgba8(), [127, 127, 127, 255]);
    }

    #[test]
    fn test_from_rgba8() {
        let c = Color::from_rgba8([255, 0, 128, 255]);
        assert_eq!(c.r, 1.0);
        assert_eq!(c.g, 0.0);
        assert!((c.b - 128.0 / 255.0).abs() < f32::EPSILON);
        assert_eq!(c.a, 1.0);
    }

    #[test]
    fn test_to_rgba8_from_rgba8_roundtrip() {
        // 使用能被 255 整除的值确保无精度损失
        let original = Color::from_rgba8([100, 150, 200, 250]);
        let roundtrip = Color::from_rgba8(original.to_rgba8());
        assert_eq!(original.to_rgba8(), roundtrip.to_rgba8());
    }

    #[test]
    fn test_to_rgba8_clamping() {
        // 超出范围的值应被钳制
        let c = Color::new(-0.5, 1.5, 0.5, 2.0);
        assert_eq!(c.to_rgba8(), [0, 255, 127, 255]);
    }
}
