// Copyright The SimpleRenderer Contributors

use criterion::{black_box, criterion_group, criterion_main, Criterion};
use glam::{Mat4, Vec3};
use simple_renderer::{Light, Model, RenderingMode, Shader, SimpleRenderer};

const BENCH_W: usize = 200;
const BENCH_H: usize = 150;
const TEAPOT_PATH: &str = "../obj/utah-teapot-texture/teapot.obj";

fn setup_shader() -> Shader {
    let model_matrix = Mat4::from_scale(Vec3::splat(0.02))
        * Mat4::from_translation(Vec3::new(0.0, -5.0, 0.0))
        * Mat4::from_rotation_x((-105.0_f32).to_radians());

    let mut shader = Shader::new();
    shader.set_uniform("modelMatrix", model_matrix);
    shader.set_uniform(
        "viewMatrix",
        Mat4::look_at_rh(Vec3::new(0.0, 0.0, 1.0), Vec3::ZERO, Vec3::Y),
    );
    shader.set_uniform(
        "projectionMatrix",
        Mat4::perspective_rh_gl(
            60.0_f32.to_radians(),
            BENCH_W as f32 / BENCH_H as f32,
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
    ]);
    shader
}

fn bench_render_mode(c: &mut Criterion, name: &str, mode: RenderingMode) {
    let model = Model::load(TEAPOT_PATH).expect("load model");
    let mut shader = setup_shader();
    let mut renderer = SimpleRenderer::new(BENCH_W, BENCH_H);
    renderer.set_rendering_mode(mode);
    let mut buffer = vec![0u32; BENCH_W * BENCH_H];

    c.bench_function(name, |b| {
        b.iter(|| {
            buffer.fill(0);
            renderer
                .draw_model(
                    black_box(&model),
                    black_box(&mut shader),
                    black_box(&mut buffer),
                )
                .unwrap();
        })
    });
}

fn render_benchmarks(c: &mut Criterion) {
    bench_render_mode(c, "per_triangle_200x150", RenderingMode::PerTriangle);
    bench_render_mode(c, "tile_based_200x150", RenderingMode::TileBased);
    bench_render_mode(c, "deferred_200x150", RenderingMode::Deferred);
    bench_render_mode(
        c,
        "tile_based_deferred_200x150",
        RenderingMode::TileBasedDeferred,
    );
}

criterion_group!(benches, render_benchmarks);
criterion_main!(benches);
