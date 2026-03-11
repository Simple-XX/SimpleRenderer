mod camera;
mod display;

use glam::{Mat4, Vec3};
use log::info;
use simple_renderer::{Buffer, Color, Light, Model, RenderingMode, Shader, SimpleRenderer};
use std::time::Instant;

use camera::Camera;
use display::Display;

/// 窗口宽度（像素）
const WIDTH: usize = 800;
/// 窗口高度（像素）
const HEIGHT: usize = 600;

fn main() {
    env_logger::init();

    let obj_path = std::env::args()
        .nth(1)
        .expect("用法: system_test <obj_dir>");

    let mut buffer = Buffer::new(WIDTH, HEIGHT);
    let mut renderer = SimpleRenderer::new(WIDTH, HEIGHT);

    let model = Model::load(&format!("{}/utah-teapot-texture/teapot.obj", obj_path))
        .expect("加载模型失败");

    // 模型矩阵：缩放 × 平移 × 旋转
    let model_matrix = Mat4::from_scale(Vec3::splat(0.02))
        * Mat4::from_translation(Vec3::new(0.0, -5.0, 0.0))
        * Mat4::from_rotation_x((-105.0_f32).to_radians());

    let mut shader = Shader::new();
    shader.set_uniform("modelMatrix", model_matrix);

    // 多光源设置
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

    let mut frame_count = 0u32;
    let mut fps_timer = Instant::now();
    let mut last_frame = Instant::now();

    while display.is_open() {
        // 计算帧间隔时间（delta time），用于帧率无关的移动
        let now = Instant::now();
        let delta_time = now.duration_since(last_frame).as_secs_f32();
        last_frame = now;

        // 处理键盘和鼠标输入
        if let Some(new_mode) = display.handle_input(&mut camera, delta_time) {
            if new_mode != renderer.rendering_mode() {
                renderer.set_rendering_mode(new_mode);
                info!("切换渲染模式: {}", new_mode);
            }
        }

        // 更新相机相关 uniform 变量
        shader.set_uniform("cameraPos", camera.position());
        shader.set_uniform("viewMatrix", camera.view_matrix());
        shader.set_uniform(
            "projectionMatrix",
            camera.projection_matrix(60.0, WIDTH as f32 / HEIGHT as f32, 0.1, 100.0),
        );

        // 清空帧缓冲并渲染
        buffer.clear_draw_buffer(Color::BLACK);
        renderer.draw_model(&model, &shader, buffer.draw_buffer_mut());
        buffer.swap();

        // 显示到窗口
        display.update(buffer.display_buffer());

        // FPS 统计与标题更新
        frame_count += 1;
        if fps_timer.elapsed().as_secs_f64() >= 1.0 {
            let fps = frame_count as f64 / fps_timer.elapsed().as_secs_f64();
            display.set_title(&format!(
                "SimpleRenderer (Rust) | {} | {:.1} FPS | 速度: {:.1}",
                renderer.rendering_mode(),
                fps,
                camera.movement_speed(),
            ));
            frame_count = 0;
            fps_timer = Instant::now();
        }
    }
}
