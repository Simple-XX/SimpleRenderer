// Copyright The SimpleGameEngine Contributors


#[cfg(not(target_endian = "little"))]
compile_error!("SimpleGameEngine assumes little-endian byte order for Color ↔ u32 conversions");

use std::fmt;
use std::ops;

/// 32 位 RGBA 颜色。
///
/// 内部存储为 `[u8; 4]`，排列为 `[R, G, B, A]`。
/// 内存布局与 u32 重新解释兼容（小端序 RGBA）：
/// R 占位 0–7，G 占位 8–15，B 占位 16–23，A 占位 24–31。
#[derive(Debug, Clone, Copy, PartialEq, Eq, Default)]
pub struct Color {
    channels: [u8; 4],
}


impl Color {
    pub const WHITE: Color = Color::new(255, 255, 255, 255);
    pub const BLACK: Color = Color::new(0, 0, 0, 255);
    pub const RED: Color = Color::new(255, 0, 0, 255);
    pub const GREEN: Color = Color::new(0, 255, 0, 255);
    pub const BLUE: Color = Color::new(0, 0, 255, 255);
}


impl Color {
    /// 从单独的 RGBA 通道值创建颜色。
    #[inline]
    pub const fn new(r: u8, g: u8, b: u8, a: u8) -> Self {
        Self {
            channels: [r, g, b, a],
        }
    }

    /// 从 `[0.0, 255.0]` 范围内的浮点数创建颜色。
    ///
    /// 截断前会加上 `+0.5` 的偏移量，以补偿浮点→整数截断
    /// （等效于四舍五入到最近整数）。
    /// 超出范围的值会被钳制到 `[0.0, 255.0]`。
    /// 注意：这些是原始的 `[0, 255]` 浮点数，而非归一化的 `[0, 1]`。
    #[inline]
    pub fn from_f32(r: f32, g: f32, b: f32, a: f32) -> Self {
        Self {
            channels: [
                ((r + 0.5).clamp(0.0, 255.0)) as u8,
                ((g + 0.5).clamp(0.0, 255.0)) as u8,
                ((b + 0.5).clamp(0.0, 255.0)) as u8,
                ((a + 0.5).clamp(0.0, 255.0)) as u8,
            ],
        }
    }

    /// 从 `[0.0, 1.0]` 范围内的归一化浮点数创建颜色。
    ///
    /// 每个分量乘以 255.0，然后截断为 `u8`。
    /// 超出范围的值在转换前会被钳制到 `[0.0, 255.0]`。
    #[inline]
    pub fn from_normalized(r: f32, g: f32, b: f32, a: f32) -> Self {
        Self {
            channels: [
                ((r * 255.0).clamp(0.0, 255.0)) as u8,
                ((g * 255.0).clamp(0.0, 255.0)) as u8,
                ((b * 255.0).clamp(0.0, 255.0)) as u8,
                ((a * 255.0).clamp(0.0, 255.0)) as u8,
            ],
        }
    }
}


impl Color {
    #[inline]
    pub const fn r(&self) -> u8 {
        self.channels[0]
    }
    #[inline]
    pub const fn g(&self) -> u8 {
        self.channels[1]
    }
    #[inline]
    pub const fn b(&self) -> u8 {
        self.channels[2]
    }
    #[inline]
    pub const fn a(&self) -> u8 {
        self.channels[3]
    }

    /// 每像素字节数（始终为 4）。
    #[inline]
    pub const fn bpp() -> usize {
        4
    }
}

//
// 布局：R 占位 0-7，G 占位 8-15，B 占位 16-23，A 占位 24-31。
// 4 字节结构体在小端序上重新解释为 u32。

impl From<u32> for Color {
    #[inline]
    fn from(val: u32) -> Self {
        Self {
            channels: [
                (val & 0xFF) as u8,
                ((val >> 8) & 0xFF) as u8,
                ((val >> 16) & 0xFF) as u8,
                ((val >> 24) & 0xFF) as u8,
            ],
        }
    }
}

impl From<Color> for u32 {
    #[inline]
    fn from(c: Color) -> Self {
        (c.channels[0] as u32)
            | ((c.channels[1] as u32) << 8)
            | ((c.channels[2] as u32) << 16)
            | ((c.channels[3] as u32) << 24)
    }
}


/// 逐通道乘以标量，钳制到 `[0, 255]`。
impl ops::Mul<f32> for Color {
    type Output = Color;

    #[inline]
    fn mul(self, rhs: f32) -> Color {
        Color {
            channels: [
                ((self.channels[0] as f32 * rhs).clamp(0.0, 255.0)) as u8,
                ((self.channels[1] as f32 * rhs).clamp(0.0, 255.0)) as u8,
                ((self.channels[2] as f32 * rhs).clamp(0.0, 255.0)) as u8,
                ((self.channels[3] as f32 * rhs).clamp(0.0, 255.0)) as u8,
            ],
        }
    }
}

/// 逐通道乘赋值标量，钳制到 `[0, 255]`。
impl ops::MulAssign<f32> for Color {
    #[inline]
    fn mul_assign(&mut self, rhs: f32) {
        for ch in &mut self.channels {
            *ch = ((*ch as f32 * rhs).clamp(0.0, 255.0)) as u8;
        }
    }
}

/// 逐通道加法，钳制到 `[0, 255]`。
impl ops::Add for Color {
    type Output = Color;

    #[inline]
    fn add(self, rhs: Color) -> Color {
        Color {
            channels: [
                self.channels[0].saturating_add(rhs.channels[0]),
                self.channels[1].saturating_add(rhs.channels[1]),
                self.channels[2].saturating_add(rhs.channels[2]),
                self.channels[3].saturating_add(rhs.channels[3]),
            ],
        }
    }
}

/// 按索引访问通道：0=R, 1=G, 2=B, 3=A。
impl ops::Index<usize> for Color {
    type Output = u8;

    #[inline]
    fn index(&self, idx: usize) -> &u8 {
        &self.channels[idx]
    }
}


impl fmt::Display for Color {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(
            f,
            "RGBA[0x{:02X}, 0x{:02X}, 0x{:02X}, 0x{:02X}]",
            self.channels[0], self.channels[1], self.channels[2], self.channels[3]
        )
    }
}


#[cfg(test)]
mod tests {
    use super::*;


    #[test]
    fn new_stores_channels_correctly() {
        let c = Color::new(10, 20, 30, 40);
        assert_eq!(c.r(), 10);
        assert_eq!(c.g(), 20);
        assert_eq!(c.b(), 30);
        assert_eq!(c.a(), 40);
    }

    #[test]
    fn default_is_all_zeros() {
        let c = Color::default();
        assert_eq!(c, Color::new(0, 0, 0, 0));
    }

    #[test]
    fn from_f32_rounds_with_half() {
        // 128.3 + 0.5 = 128.8 → 截断为 u8 得 128
        // 0.0 + 0.5 = 0.5 → 截断为 u8 得 0
        // 254.6 + 0.5 = 255.1 → 截断为 u8 得 255
        let c = Color::from_f32(128.3, 0.0, 254.6, 200.0);
        assert_eq!(c.r(), 128);
        assert_eq!(c.g(), 0);
        assert_eq!(c.b(), 255);
        assert_eq!(c.a(), 200);
    }

    #[test]
    fn from_f32_exact_values() {
        let c = Color::from_f32(0.0, 127.0, 255.0, 255.0);
        assert_eq!(c.r(), 0);
        assert_eq!(c.g(), 127);
        assert_eq!(c.b(), 255);
        assert_eq!(c.a(), 255);
    }

    #[test]
    fn from_normalized_basic() {
        let c = Color::from_normalized(1.0, 0.0, 0.5, 1.0);
        assert_eq!(c.r(), 255);
        assert_eq!(c.g(), 0);
        assert_eq!(c.b(), 127);
        assert_eq!(c.a(), 255);
    }

    #[test]
    fn from_normalized_zero() {
        let c = Color::from_normalized(0.0, 0.0, 0.0, 0.0);
        assert_eq!(c, Color::new(0, 0, 0, 0));
    }

    #[test]
    fn from_normalized_one() {
        let c = Color::from_normalized(1.0, 1.0, 1.0, 1.0);
        assert_eq!(c, Color::new(255, 255, 255, 255));
    }

    #[test]
    fn from_f32_clamps_negative() {
        let c = Color::from_f32(-10.0, -5.0, 0.0, 255.0);
        assert_eq!(c.r(), 0);
        assert_eq!(c.g(), 0);
        assert_eq!(c.b(), 0);
        assert_eq!(c.a(), 255);
    }

    #[test]
    fn from_f32_clamps_overflow() {
        let c = Color::from_f32(300.0, 256.0, 255.0, 255.0);
        assert_eq!(c.r(), 255);
        assert_eq!(c.g(), 255);
        assert_eq!(c.b(), 255);
        assert_eq!(c.a(), 255);
    }

    #[test]
    fn from_normalized_clamps_overflow() {
        let c = Color::from_normalized(1.5, -0.5, 0.5, 1.0);
        assert_eq!(c.r(), 255);
        assert_eq!(c.g(), 0);
        assert_eq!(c.b(), 127);
        assert_eq!(c.a(), 255);
    }


    #[test]
    fn constants_are_correct() {
        assert_eq!(Color::WHITE, Color::new(255, 255, 255, 255));
        assert_eq!(Color::BLACK, Color::new(0, 0, 0, 255));
        assert_eq!(Color::RED, Color::new(255, 0, 0, 255));
        assert_eq!(Color::GREEN, Color::new(0, 255, 0, 255));
        assert_eq!(Color::BLUE, Color::new(0, 0, 255, 255));
    }


    #[test]
    fn from_u32_little_endian_rgba() {
        // R=0xAA 占位 0-7，G=0xBB 占位 8-15，
        // B=0xCC 占位 16-23，A=0xDD 占位 24-31
        let val: u32 = 0xDDCCBBAA;
        let c = Color::from(val);
        assert_eq!(c.r(), 0xAA);
        assert_eq!(c.g(), 0xBB);
        assert_eq!(c.b(), 0xCC);
        assert_eq!(c.a(), 0xDD);
    }

    #[test]
    fn to_u32_little_endian_rgba() {
        let c = Color::new(0xAA, 0xBB, 0xCC, 0xDD);
        let val: u32 = c.into();
        assert_eq!(val, 0xDDCCBBAA);
    }

    #[test]
    fn u32_roundtrip() {
        let original = Color::new(100, 150, 200, 250);
        let val: u32 = original.into();
        let restored = Color::from(val);
        assert_eq!(original, restored);
    }

    #[test]
    fn u32_roundtrip_all_zeros() {
        let c = Color::new(0, 0, 0, 0);
        let val: u32 = c.into();
        assert_eq!(val, 0x00000000);
        assert_eq!(Color::from(val), c);
    }

    #[test]
    fn u32_roundtrip_all_ones() {
        let c = Color::new(255, 255, 255, 255);
        let val: u32 = c.into();
        assert_eq!(val, 0xFFFFFFFF);
        assert_eq!(Color::from(val), c);
    }

    #[test]
    fn u32_red_channel_only() {
        let c = Color::new(0xFF, 0, 0, 0);
        let val: u32 = c.into();
        assert_eq!(val, 0x000000FF);
    }

    #[test]
    fn u32_green_channel_only() {
        let c = Color::new(0, 0xFF, 0, 0);
        let val: u32 = c.into();
        assert_eq!(val, 0x0000FF00);
    }

    #[test]
    fn u32_blue_channel_only() {
        let c = Color::new(0, 0, 0xFF, 0);
        let val: u32 = c.into();
        assert_eq!(val, 0x00FF0000);
    }

    #[test]
    fn u32_alpha_channel_only() {
        let c = Color::new(0, 0, 0, 0xFF);
        let val: u32 = c.into();
        assert_eq!(val, 0xFF000000);
    }


    #[test]
    fn mul_by_scalar() {
        let c = Color::new(100, 200, 50, 255);
        let result = c * 0.5;
        assert_eq!(result.r(), 50);
        assert_eq!(result.g(), 100);
        assert_eq!(result.b(), 25);
        assert_eq!(result.a(), 127);
    }

    #[test]
    fn mul_clamps_high() {
        let c = Color::new(200, 200, 200, 200);
        let result = c * 2.0;
        assert_eq!(result, Color::new(255, 255, 255, 255));
    }

    #[test]
    fn mul_clamps_negative() {
        let c = Color::new(100, 100, 100, 100);
        let result = c * -1.0;
        assert_eq!(result, Color::new(0, 0, 0, 0));
    }

    #[test]
    fn mul_by_zero() {
        let c = Color::new(255, 255, 255, 255);
        let result = c * 0.0;
        assert_eq!(result, Color::new(0, 0, 0, 0));
    }

    #[test]
    fn mul_by_one() {
        let c = Color::new(100, 150, 200, 250);
        let result = c * 1.0;
        assert_eq!(result, c);
    }

    #[test]
    fn mul_assign() {
        let mut c = Color::new(100, 200, 50, 255);
        c *= 0.5;
        assert_eq!(c.r(), 50);
        assert_eq!(c.g(), 100);
        assert_eq!(c.b(), 25);
        assert_eq!(c.a(), 127);
    }

    #[test]
    fn mul_assign_clamps() {
        let mut c = Color::new(200, 200, 200, 200);
        c *= 2.0;
        assert_eq!(c, Color::new(255, 255, 255, 255));
    }

    #[test]
    fn add_basic() {
        let a = Color::new(10, 20, 30, 40);
        let b = Color::new(5, 10, 15, 20);
        let result = a + b;
        assert_eq!(result, Color::new(15, 30, 45, 60));
    }

    #[test]
    fn add_clamps_to_255() {
        let a = Color::new(200, 200, 200, 200);
        let b = Color::new(100, 100, 100, 100);
        let result = a + b;
        assert_eq!(result, Color::new(255, 255, 255, 255));
    }

    #[test]
    fn add_with_zero() {
        let a = Color::new(100, 150, 200, 250);
        let b = Color::new(0, 0, 0, 0);
        assert_eq!(a + b, a);
    }


    #[test]
    fn index_access() {
        let c = Color::new(10, 20, 30, 40);
        assert_eq!(c[0], 10); // R
        assert_eq!(c[1], 20); // G
        assert_eq!(c[2], 30); // B
        assert_eq!(c[3], 40); // A
    }

    #[test]
    #[should_panic]
    fn index_out_of_bounds_panics() {
        let c = Color::new(10, 20, 30, 40);
        let _ = c[4];
    }


    #[test]
    fn bpp_is_four() {
        assert_eq!(Color::bpp(), 4);
    }

    #[test]
    fn copy_semantics() {
        let a = Color::new(1, 2, 3, 4);
        let b = a;
        assert_eq!(a, b);
    }

    #[test]
    fn clone_semantics() {
        let a = Color::new(1, 2, 3, 4);
        let b = a.clone();
        assert_eq!(a, b);
    }

    #[test]
    fn display_format() {
        let c = Color::new(0xFF, 0x00, 0xAB, 0xCD);
        let s = format!("{}", c);
        assert_eq!(s, "RGBA[0xFF, 0x00, 0xAB, 0xCD]");
    }

    #[test]
    fn debug_format_exists() {
        let c = Color::new(1, 2, 3, 4);
        let s = format!("{:?}", c);
        assert!(s.contains("Color"));
    }

    #[test]
    fn equality() {
        assert_eq!(Color::new(1, 2, 3, 4), Color::new(1, 2, 3, 4));
        assert_ne!(Color::new(1, 2, 3, 4), Color::new(1, 2, 3, 5));
    }


    #[test]
    fn white_constant_is_fully_opaque() {
        assert_eq!(Color::WHITE, Color::new(0xFF, 0xFF, 0xFF, 0xFF));
    }

    #[test]
    fn u32_reinterpret_roundtrip() {
        // 4 字节结构体在小端序上重新解释为 u32
        // [R, G, B, A] → R 是最低字节
        let c = Color::new(0x12, 0x34, 0x56, 0x78);
        let val: u32 = c.into();
        assert_eq!(val, 0x78563412);
        assert_eq!(Color::from(val), c);
    }
}
