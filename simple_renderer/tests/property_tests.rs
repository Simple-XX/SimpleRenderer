//! Property-based tests for mathematical correctness.

use proptest::prelude::*;
use simple_renderer::Color;

// ── Color roundtrip properties ──────────────────────────────────────

proptest! {
    #[test]
    fn color_u32_roundtrip(r in 0u8..=255, g in 0u8..=255, b in 0u8..=255, a in 0u8..=255) {
        let color = Color::new(r, g, b, a);
        let val: u32 = color.into();
        let restored = Color::from(val);
        prop_assert_eq!(color, restored);
    }

    #[test]
    fn color_add_commutative(
        r1 in 0u8..=255, g1 in 0u8..=255, b1 in 0u8..=255, a1 in 0u8..=255,
        r2 in 0u8..=255, g2 in 0u8..=255, b2 in 0u8..=255, a2 in 0u8..=255,
    ) {
        let c1 = Color::new(r1, g1, b1, a1);
        let c2 = Color::new(r2, g2, b2, a2);
        prop_assert_eq!(c1 + c2, c2 + c1);
    }

    #[test]
    fn color_mul_by_one_is_identity(r in 0u8..=255, g in 0u8..=255, b in 0u8..=255, a in 0u8..=255) {
        let color = Color::new(r, g, b, a);
        prop_assert_eq!(color * 1.0, color);
    }

    #[test]
    fn color_mul_by_zero_is_black(r in 0u8..=255, g in 0u8..=255, b in 0u8..=255, a in 0u8..=255) {
        let color = Color::new(r, g, b, a);
        let result = color * 0.0;
        prop_assert_eq!(result, Color::new(0, 0, 0, 0));
    }

    #[test]
    fn color_add_with_zero_is_identity(r in 0u8..=255, g in 0u8..=255, b in 0u8..=255, a in 0u8..=255) {
        let color = Color::new(r, g, b, a);
        let zero = Color::new(0, 0, 0, 0);
        prop_assert_eq!(color + zero, color);
    }

    #[test]
    fn color_from_f32_clamped(r in -100.0f32..400.0, g in -100.0f32..400.0, b in -100.0f32..400.0, a in -100.0f32..400.0) {
        let color = Color::from_f32(r, g, b, a);
        // All channels should be in [0, 255] regardless of input
        prop_assert!(color.r() <= 255);
        prop_assert!(color.g() <= 255);
        prop_assert!(color.b() <= 255);
        prop_assert!(color.a() <= 255);
    }

    #[test]
    fn color_from_normalized_clamped(r in -1.0f32..2.0, g in -1.0f32..2.0, b in -1.0f32..2.0, a in -1.0f32..2.0) {
        let color = Color::from_normalized(r, g, b, a);
        prop_assert!(color.r() <= 255);
        prop_assert!(color.g() <= 255);
        prop_assert!(color.b() <= 255);
        prop_assert!(color.a() <= 255);
    }
}
