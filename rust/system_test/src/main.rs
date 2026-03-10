mod camera;
mod display;

use glam::{Mat4, Vec3};
use simple_renderer::{Buffer, Color, Light, Model, RenderingMode, Shader, SimpleRenderer};

use camera::Camera;
use display::Display;

const WIDTH: usize = 800;
const HEIGHT: usize = 600;

fn main() {
    env_logger::init();

    let obj_path = std::env::args()
        .nth(1)
        .expect("Usage: system_test <obj_dir>");

    let mut buffer = Buffer::new(WIDTH, HEIGHT);
    let mut renderer = SimpleRenderer::new(WIDTH, HEIGHT);

    let model = Model::load(&format!("{}/utah-teapot-texture/teapot.obj", obj_path))
        .expect("Failed to load model");

    // Model matrix: scale * translate * rotate (same as C++)
    let model_matrix = Mat4::from_scale(Vec3::splat(0.02))
        * Mat4::from_translation(Vec3::new(0.0, -5.0, 0.0))
        * Mat4::from_rotation_x((-105.0_f32).to_radians());

    let mut shader = Shader::new();
    shader.set_uniform("modelMatrix", model_matrix);

    // Multi-light setup (same as C++)
    let lights = vec![
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
    ];
    shader.set_lights(&lights);

    let mut camera = Camera::new(Vec3::new(0.0, 0.0, 1.0));
    renderer.set_rendering_mode(RenderingMode::TileBased);

    let mut display = Display::new(WIDTH, HEIGHT);

    while display.is_open() {
        display.handle_input(&mut camera);

        shader.set_uniform("cameraPos", camera.position());
        shader.set_uniform("viewMatrix", camera.view_matrix());
        shader.set_uniform(
            "projectionMatrix",
            camera.projection_matrix(60.0, WIDTH as f32 / HEIGHT as f32, 0.1, 100.0),
        );

        buffer.clear_draw_buffer(Color::BLACK);
        renderer.draw_model(&model, &shader, buffer.draw_buffer_mut());
        buffer.swap();

        display.update(buffer.display_buffer());
    }
}
