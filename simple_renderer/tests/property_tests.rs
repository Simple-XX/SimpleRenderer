// Copyright (c) Simple-XX/SimpleRenderer
// SPDX-License-Identifier: MIT

//! 数学正确性的属性测试。

use glam::{Mat4, Vec2, Vec3, Vec4};
use proptest::prelude::*;
use simple_renderer::vertex::Vertex;
use simple_renderer::{Buffer, Color, Shader};


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
        // 无论输入如何，所有通道都应在 [0, 255] 范围内
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


    #[test]
    fn vertex_transform_identity_preserves_position(
        x in -1000.0f32..1000.0,
        y in -1000.0f32..1000.0,
        z in -1000.0f32..1000.0,
        nx in -1.0f32..1.0,
        ny in -1.0f32..1.0,
        nz in -1.0f32..1.0,
        u in 0.0f32..1.0,
        v in 0.0f32..1.0,
    ) {
        let pos = Vec4::new(x, y, z, 1.0);
        let normal = Vec3::new(nx, ny, nz);
        let tex = Vec2::new(u, v);
        let vertex = Vertex::new(pos, normal, tex, Color::WHITE);
        let transformed = vertex.transform(&Mat4::IDENTITY);
        prop_assert!(
            (transformed.position.x - x).abs() < 1e-4,
            "x mismatch: {} vs {}",
            transformed.position.x,
            x
        );
        prop_assert!(
            (transformed.position.y - y).abs() < 1e-4,
            "y mismatch: {} vs {}",
            transformed.position.y,
            y
        );
        prop_assert!(
            (transformed.position.z - z).abs() < 1e-4,
            "z mismatch: {} vs {}",
            transformed.position.z,
            z
        );
    }

    #[test]
    fn vertex_transform_preserves_tex_coords(
        x in -100.0f32..100.0,
        y in -100.0f32..100.0,
        z in -100.0f32..100.0,
        u in 0.0f32..1.0,
        v in 0.0f32..1.0,
        scale in 0.1f32..10.0,
    ) {
        let pos = Vec4::new(x, y, z, 1.0);
        let tex = Vec2::new(u, v);
        let vertex = Vertex::new(pos, Vec3::Y, tex, Color::WHITE);
        let mat = Mat4::from_scale(Vec3::splat(scale));
        let transformed = vertex.transform(&mat);
        prop_assert_eq!(transformed.tex_coords, tex);
    }


    #[test]
    fn vertex_shader_identity_matrices_preserve_position(
        x in -10.0f32..10.0,
        y in -10.0f32..10.0,
        z in -10.0f32..10.0,
    ) {
        let mut shader = Shader::new();
        shader.set_uniform("modelMatrix", Mat4::IDENTITY);
        shader.set_uniform("viewMatrix", Mat4::IDENTITY);
        shader.set_uniform("projectionMatrix", Mat4::IDENTITY);

        let vertex = Vertex::new(
            Vec4::new(x, y, z, 1.0),
            Vec3::Y,
            Vec2::ZERO,
            Color::WHITE,
        );
        let result = shader.vertex_shader(&vertex);
        prop_assert!(
            (result.position.x - x).abs() < 1e-4,
            "x: {} vs {}",
            result.position.x,
            x
        );
        prop_assert!(
            (result.position.y - y).abs() < 1e-4,
            "y: {} vs {}",
            result.position.y,
            y
        );
        prop_assert!(
            (result.position.z - z).abs() < 1e-4,
            "z: {} vs {}",
            result.position.z,
            z
        );
    }

    #[test]
    fn vertex_shader_preserves_tex_coords(
        x in -10.0f32..10.0,
        y in -10.0f32..10.0,
        z in -10.0f32..10.0,
        u in 0.0f32..1.0,
        v in 0.0f32..1.0,
    ) {
        let mut shader = Shader::new();
        shader.set_uniform("modelMatrix", Mat4::IDENTITY);
        shader.set_uniform("viewMatrix", Mat4::IDENTITY);
        shader.set_uniform("projectionMatrix", Mat4::IDENTITY);

        let tex = Vec2::new(u, v);
        let vertex = Vertex::new(Vec4::new(x, y, z, 1.0), Vec3::Y, tex, Color::RED);
        let result = shader.vertex_shader(&vertex);
        prop_assert_eq!(result.tex_coords, tex);
    }


    #[test]
    fn buffer_double_swap_draw_buffer_unchanged(
        r in 0u8..=255,
        g in 0u8..=255,
        b in 0u8..=255,
        a in 0u8..=255,
        w in 1usize..32,
        h in 1usize..32,
    ) {
        let color = Color::new(r, g, b, a);
        let color_u32: u32 = color.into();
        let mut buf = Buffer::new(w, h);
        buf.clear_draw_buffer(color);

        buf.swap();
        buf.swap();

        prop_assert!(
            buf.draw_buffer().iter().all(|&p| p == color_u32),
            "draw buffer changed after double swap"
        );
    }

    #[test]
    fn buffer_swap_moves_draw_to_display(
        r in 0u8..=255,
        g in 0u8..=255,
        b in 0u8..=255,
        a in 0u8..=255,
        w in 1usize..32,
        h in 1usize..32,
    ) {
        let color = Color::new(r, g, b, a);
        let color_u32: u32 = color.into();
        let mut buf = Buffer::new(w, h);
        buf.clear_draw_buffer(color);

        prop_assert!(buf.draw_buffer().iter().all(|&p| p == color_u32));
        prop_assert!(buf.display_buffer().iter().all(|&p| p == 0));

        buf.swap();

        prop_assert!(buf.display_buffer().iter().all(|&p| p == color_u32));
        prop_assert!(buf.draw_buffer().iter().all(|&p| p == 0));
    }


    #[test]
    fn color_normalized_roundtrip_in_range(
        r in 0.0f32..=1.0,
        g in 0.0f32..=1.0,
        b in 0.0f32..=1.0,
        a in 0.0f32..=1.0,
    ) {
        let color = Color::from_normalized(r, g, b, a);
        let r_back = color.r() as f32 / 255.0;
        let g_back = color.g() as f32 / 255.0;
        let b_back = color.b() as f32 / 255.0;
        let a_back = color.a() as f32 / 255.0;
        prop_assert!((r_back - r).abs() <= 1.0 / 255.0 + 1e-6, "r: {} vs {}", r_back, r);
        prop_assert!((g_back - g).abs() <= 1.0 / 255.0 + 1e-6, "g: {} vs {}", g_back, g);
        prop_assert!((b_back - b).abs() <= 1.0 / 255.0 + 1e-6, "b: {} vs {}", b_back, b);
        prop_assert!((a_back - a).abs() <= 1.0 / 255.0 + 1e-6, "a: {} vs {}", a_back, a);
    }

    #[test]
    fn color_add_saturating_monotone(
        r1 in 0u8..=255, g1 in 0u8..=255, b1 in 0u8..=255, a1 in 0u8..=255,
        r2 in 0u8..=255, g2 in 0u8..=255, b2 in 0u8..=255, a2 in 0u8..=255,
    ) {
        let c1 = Color::new(r1, g1, b1, a1);
        let c2 = Color::new(r2, g2, b2, a2);
        let result = c1 + c2;
        prop_assert!(result.r() >= c1.r(), "r decreased: {} < {}", result.r(), c1.r());
        prop_assert!(result.g() >= c1.g(), "g decreased: {} < {}", result.g(), c1.g());
        prop_assert!(result.b() >= c1.b(), "b decreased: {} < {}", result.b(), c1.b());
        prop_assert!(result.a() >= c1.a(), "a decreased: {} < {}", result.a(), c1.a());
    }
}
