mod camera;
mod display;

use glam::{Mat4, Vec3};
use log::info;
use engine_render_sw::uniform::names as u;
use engine_render_sw::{triple_buffer, Color, Light, Model, RenderingMode, Shader, SimpleRenderer};
use std::sync::atomic::{AtomicBool, AtomicU8, Ordering};
use std::sync::{Arc, Mutex};
use std::time::{Duration, Instant};

use camera::Camera;
use display::Display;

const WIDTH: usize = 800;
const HEIGHT: usize = 600;

/// 每帧从主线程发送到渲染线程的状态。
#[derive(Clone)]
struct RenderCommand {
    camera_pos: Vec3,
    view_matrix: Mat4,
    projection_matrix: Mat4,
}

/// 主线程与渲染线程之间的共享状态。
struct SharedState {
    command: Mutex<RenderCommand>,
    rendering_mode: AtomicU8,
    double_buffer_mode: AtomicBool,
    running: AtomicBool,
}

fn main() {
    env_logger::init();

    let obj_path = std::env::args()
        .nth(1)
        .expect("usage: system_test <obj_dir>");

    let model = Arc::new(
        Model::load(&format!("{}/utah-teapot-texture/teapot.obj", obj_path))
            .expect("failed to load model"),
    );

    let mut camera = Camera::new(Vec3::new(0.0, 0.0, 1.0));
    let initial_mode = RenderingMode::TileBased;

    let shared = Arc::new(SharedState {
        command: Mutex::new(RenderCommand {
            camera_pos: camera.position(),
            view_matrix: camera.view_matrix(),
            projection_matrix: camera.projection_matrix(
                60.0,
                WIDTH as f32 / HEIGHT as f32,
                0.1,
                100.0,
            ),
        }),
        rendering_mode: AtomicU8::new(initial_mode as u8),
        double_buffer_mode: AtomicBool::new(false),
        running: AtomicBool::new(true),
    });

    let (writer, mut reader) = triple_buffer::create_triple_buffer(WIDTH, HEIGHT);

    let render_shared = Arc::clone(&shared);
    let render_model = Arc::clone(&model);
    let render_handle = std::thread::spawn(move || {
        render_loop(writer, render_model, render_shared);
    });

    let mut display = Display::new(WIDTH, HEIGHT);
    let mut current_mode = initial_mode;
    let mut vsync_enabled = true;
    let mut double_buffer = false;

    let mut frame_count = 0u32;
    let mut fps_timer = Instant::now();
    let mut last_frame = Instant::now();
    let mut render_frames = 0u32;

    while display.is_open() {
        let now = Instant::now();
        let delta_time = now.duration_since(last_frame).as_secs_f32();
        last_frame = now;

        if let Some(action) = display.handle_input(&mut camera, delta_time) {
            match action {
                display::InputAction::SetMode(mode) => {
                    if mode != current_mode {
                        current_mode = mode;
                        shared.rendering_mode.store(mode as u8, Ordering::Relaxed);
                        info!("switched rendering mode: {}", mode);
                    }
                }
                display::InputAction::ToggleVSync => {
                    vsync_enabled = !vsync_enabled;
                    info!("VSync: {}", if vsync_enabled { "ON" } else { "OFF" });
                }
                display::InputAction::ToggleBufferMode => {
                    double_buffer = !double_buffer;
                    shared
                        .double_buffer_mode
                        .store(double_buffer, Ordering::Relaxed);
                    info!(
                        "buffer mode: {}",
                        if double_buffer {
                            "double-buffered"
                        } else {
                            "triple-buffered"
                        }
                    );
                }
            }
        }

        {
            let mut cmd = shared.command.lock().unwrap();
            cmd.camera_pos = camera.position();
            cmd.view_matrix = camera.view_matrix();
            cmd.projection_matrix =
                camera.projection_matrix(60.0, WIDTH as f32 / HEIGHT as f32, 0.1, 100.0);
        }

        if reader.update() {
            render_frames += 1;
        }
        display.update(reader.front_buffer());

        if vsync_enabled {
            let frame_time = now.elapsed();
            let target = Duration::from_micros(16_667); // 约60 Hz
            if frame_time < target {
                std::thread::sleep(target - frame_time);
            }
        }

        frame_count += 1;
        if fps_timer.elapsed().as_secs_f64() >= 1.0 {
            let elapsed = fps_timer.elapsed().as_secs_f64();
            let display_fps = frame_count as f64 / elapsed;
            let render_fps = render_frames as f64 / elapsed;
            let buf_mode = if double_buffer { "Double" } else { "Triple" };
            let vs = if vsync_enabled { " VSync" } else { "" };
            display.set_title(&format!(
                "SimpleGameEngine | {} | Display {:.0} Render {:.0} FPS | {}{}",
                current_mode, display_fps, render_fps, buf_mode, vs,
            ));
            frame_count = 0;
            render_frames = 0;
            fps_timer = Instant::now();
        }
    }

    shared.running.store(false, Ordering::Relaxed);
    drop(display);
    render_handle.join().unwrap();
}

fn render_loop(
    mut writer: triple_buffer::TripleBufferWriter,
    model: Arc<Model>,
    shared: Arc<SharedState>,
) {
    let model_matrix = Mat4::from_scale(Vec3::splat(0.02))
        * Mat4::from_translation(Vec3::new(0.0, -5.0, 0.0))
        * Mat4::from_rotation_x((-105.0_f32).to_radians());

    let mut shader = Shader::new();
    shader.set_uniform(u::MODEL_MATRIX, model_matrix);
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

    let width = writer.width();
    let height = writer.height();
    let mut renderer = SimpleRenderer::new(width, height);

    while shared.running.load(Ordering::Relaxed) {
        let cmd = shared.command.lock().unwrap().clone();

        let mode_u8 = shared.rendering_mode.load(Ordering::Relaxed);
        let mode = RenderingMode::try_from(mode_u8).unwrap_or(RenderingMode::TileBasedDeferred);
        if mode != renderer.rendering_mode() {
            renderer.set_rendering_mode(mode);
        }

        shader.set_uniform(u::CAMERA_POS, cmd.camera_pos);
        shader.set_uniform(u::VIEW_MATRIX, cmd.view_matrix);
        shader.set_uniform(u::PROJECTION_MATRIX, cmd.projection_matrix);

        writer.clear(Color::BLACK);
        let buf = writer.render_buffer_mut();
        if let Err(e) = renderer.draw_model(&model, &mut shader, buf) {
            log::error!("render failed: {}", e);
        }

        if shared.double_buffer_mode.load(Ordering::Relaxed) {
            writer.publish_and_wait();
        } else {
            writer.publish();
        }
    }
}
