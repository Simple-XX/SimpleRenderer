//! Integration tests for the simple_renderer library.
//!
//! These tests render a real model (teapot) with all 4 rendering modes.
//! Run with `cargo test --test integration_test` (they may use significant memory).

use glam::{Mat4, Vec3};
use simple_renderer::{Buffer, Color, Light, Model, RenderingMode, Shader, SimpleRenderer};

/// Test model path relative to workspace root
const TEAPOT_PATH: &str = "../obj/utah-teapot-texture/teapot.obj";

/// Small render resolution to avoid OOM in test environments.
const TEST_W: usize = 100;
const TEST_H: usize = 75;

fn setup_shader() -> Shader {
    let model_matrix = Mat4::from_scale(Vec3::splat(0.02))
        * Mat4::from_translation(Vec3::new(0.0, -5.0, 0.0))
        * Mat4::from_rotation_x((-105.0_f32).to_radians());

    let mut shader = Shader::new();
    shader.set_uniform("modelMatrix", model_matrix);
    shader.set_uniform(
        "viewMatrix",
        Mat4::look_at_rh(Vec3::new(0.0, 0.0, 1.0), Vec3::new(0.0, 0.0, 0.0), Vec3::Y),
    );
    shader.set_uniform(
        "projectionMatrix",
        Mat4::perspective_rh_gl(
            60.0_f32.to_radians(),
            TEST_W as f32 / TEST_H as f32,
            0.1,
            100.0,
        ),
    );
    shader.set_uniform("cameraPos", Vec3::new(0.0, 0.0, 1.0));
    shader.set_lights(&[
        Light {
            direction: Vec3::new(1.0, 5.0, 1.0),
            ..Light::default()
        },
        Light {
            direction: Vec3::new(-3.0, -2.0, 2.0),
            ..Light::default()
        },
        Light {
            direction: Vec3::new(2.0, 1.0, -1.0),
            ..Light::default()
        },
    ]);
    shader
}

fn render_with_mode(mode: RenderingMode) -> usize {
    let model = Model::load(TEAPOT_PATH).expect("Failed to load teapot");
    let shader = setup_shader();
    let mut renderer = SimpleRenderer::new(TEST_W, TEST_H);
    renderer.set_rendering_mode(mode);

    let mut buffer = vec![0u32; TEST_W * TEST_H];
    let result = renderer.draw_model(&model, &shader, &mut buffer);
    result.expect(&format!("{:?} render should succeed", mode));
    buffer.iter().filter(|&&p| p != 0).count()
}

#[test]
fn load_teapot_model() {
    let model = Model::load(TEAPOT_PATH).expect("Failed to load teapot");
    assert!(
        model.vertices().len() > 100,
        "teapot should have many vertices"
    );
    assert!(model.faces().len() > 100, "teapot should have many faces");
}

#[test]
fn render_per_triangle_produces_pixels() {
    let nonzero = render_with_mode(RenderingMode::PerTriangle);
    assert!(
        nonzero > 0,
        "PerTriangle should produce non-zero pixels, got {}",
        nonzero
    );
}

#[test]
fn render_tile_based_produces_pixels() {
    let nonzero = render_with_mode(RenderingMode::TileBased);
    assert!(
        nonzero > 0,
        "TileBased should produce non-zero pixels, got {}",
        nonzero
    );
}

#[test]
fn render_deferred_produces_pixels() {
    let nonzero = render_with_mode(RenderingMode::Deferred);
    assert!(
        nonzero > 0,
        "Deferred should produce non-zero pixels, got {}",
        nonzero
    );
}

#[test]
fn render_tile_based_deferred_produces_pixels() {
    let nonzero = render_with_mode(RenderingMode::TileBasedDeferred);
    assert!(
        nonzero > 0,
        "TileBasedDeferred should produce non-zero pixels, got {}",
        nonzero
    );
}

/// Run all 4 modes sequentially and verify they produce comparable output.
#[test]
fn all_modes_produce_similar_output() {
    let model = Model::load(TEAPOT_PATH).expect("Failed to load teapot");
    let shader = setup_shader();
    let mut renderer = SimpleRenderer::new(TEST_W, TEST_H);

    let modes = [
        RenderingMode::PerTriangle,
        RenderingMode::TileBased,
        RenderingMode::Deferred,
        RenderingMode::TileBasedDeferred,
    ];

    let mut counts = Vec::new();
    for &mode in &modes {
        renderer.set_rendering_mode(mode);
        let mut buffer = vec![0u32; TEST_W * TEST_H];
        renderer.draw_model(&model, &shader, &mut buffer).expect("render failed");
        let nonzero = buffer.iter().filter(|&&p| p != 0).count();
        counts.push((mode, nonzero));
    }

    let max_count = counts.iter().map(|(_, c)| *c).max().unwrap();
    let min_count = counts.iter().map(|(_, c)| *c).min().unwrap();
    assert!(
        min_count > 0,
        "All modes should produce output. Counts: {:?}",
        counts
    );
    // Allow generous variance between modes (different algorithms)
    assert!(
        min_count as f64 / max_count as f64 > 0.3,
        "Modes should produce broadly similar output. Counts: {:?}",
        counts
    );
}

#[test]
fn buffer_double_buffering() {
    let mut buf = Buffer::new(100, 100);
    buf.clear_draw_buffer(Color::RED);
    let draw_snapshot: Vec<u32> = buf.draw_buffer().to_vec();
    buf.swap();
    // After swap, display should have what was draw
    let display = buf.display_buffer();
    assert_eq!(display, draw_snapshot.as_slice());
    // New draw buffer should be zeroed (from initial state)
    let new_draw = buf.draw_buffer();
    assert_eq!(new_draw.iter().filter(|&&p| p != 0).count(), 0);
}
