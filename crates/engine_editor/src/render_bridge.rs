//! 渲染桥接层：在独立线程中运行软件渲染器，
//! 通过 mpsc 通道接收命令，通过三重缓冲输出像素。

use std::path::PathBuf;
use std::sync::atomic::{AtomicU64, Ordering};
use std::sync::mpsc::{self, Receiver, Sender};
use std::sync::Arc;
use std::time::Instant;

use engine_render_sw::triple_buffer::{self, TripleBufferReader, TripleBufferWriter};
use engine_render_sw::uniform::names as u;
use engine_render_sw::{Color, Light, Model, RenderingMode, Shader, SimpleRenderer};
use glam::{Mat4, Vec3};

/// 从 UI 线程发送到渲染线程的命令
#[allow(dead_code)]
pub enum RenderCommand {
    LoadModel(PathBuf),
    SetCamera {
        view: Mat4,
        projection: Mat4,
        pos: Vec3,
    },
    SetRenderingMode(RenderingMode),
    SetTileSize(usize),
    SetEarlyZ(bool),
    SetLights(Vec<Light>),
    SetShininess(f32),
    Resize {
        width: usize,
        height: usize,
    },
    Shutdown,
}

/// UI 侧持有的渲染桥接句柄
pub struct RenderBridge {
    cmd_tx: Sender<RenderCommand>,
    reader: TripleBufferReader,
    /// 渲染线程报告的帧耗时（微秒），通过原子变量共享
    frame_time_us: Arc<AtomicU64>,
    render_thread: Option<std::thread::JoinHandle<()>>,
    width: usize,
    height: usize,
}

impl RenderBridge {
    /// 启动渲染线程，返回桥接句柄
    pub fn new(width: usize, height: usize) -> Self {
        let (cmd_tx, cmd_rx) = mpsc::channel();
        let (writer, reader) = triple_buffer::create_triple_buffer(width, height);
        let frame_time_us = Arc::new(AtomicU64::new(0));
        let ft_clone = Arc::clone(&frame_time_us);

        let render_thread = std::thread::Builder::new()
            .name("render-thread".into())
            .spawn(move || {
                render_loop(writer, cmd_rx, ft_clone, width, height);
            })
            .expect("启动渲染线程失败");

        Self {
            cmd_tx,
            reader,
            frame_time_us,
            render_thread: Some(render_thread),
            width,
            height,
        }
    }

    /// 发送命令到渲染线程
    pub fn send(&self, cmd: RenderCommand) {
        let _ = self.cmd_tx.send(cmd);
    }

    /// 尝试从三重缓冲读取最新帧，返回是否有新帧
    pub fn try_read_frame(&mut self) -> bool {
        self.reader.update()
    }

    /// 获取当前前台缓冲区像素数据
    pub fn front_buffer(&self) -> &[u32] {
        self.reader.front_buffer()
    }

    /// 获取渲染线程报告的帧耗时（微秒）
    pub fn frame_time_us(&self) -> u64 {
        self.frame_time_us.load(Ordering::Relaxed)
    }

    pub fn width(&self) -> usize {
        self.width
    }

    pub fn height(&self) -> usize {
        self.height
    }
}

impl Drop for RenderBridge {
    fn drop(&mut self) {
        let _ = self.cmd_tx.send(RenderCommand::Shutdown);
        if let Some(handle) = self.render_thread.take() {
            let _ = handle.join();
        }
    }
}

/// 渲染线程主循环
fn render_loop(
    mut writer: TripleBufferWriter,
    cmd_rx: Receiver<RenderCommand>,
    frame_time_us: Arc<AtomicU64>,
    init_width: usize,
    init_height: usize,
) {
    let mut renderer = SimpleRenderer::new(init_width, init_height);
    let mut shader = Shader::new();
    let mut model: Option<Model> = None;
    let mut width = init_width;
    let mut height = init_height;

    // 设置默认模型矩阵（茶壶变换）
    let model_matrix = Mat4::from_scale(Vec3::splat(0.02))
        * Mat4::from_translation(Vec3::new(0.0, -5.0, 0.0))
        * Mat4::from_rotation_x((-105.0_f32).to_radians());
    shader.set_uniform(u::MODEL_MATRIX, model_matrix);

    // 设置默认光源
    shader.set_lights(&[Light {
        name: "主光源".to_string(),
        direction: Vec3::new(1.0, 5.0, 1.0),
        color: Color::WHITE,
        ..Light::default()
    }]);

    // 设置默认相机
    shader.set_uniform(u::CAMERA_POS, Vec3::new(0.0, 0.0, 3.0));
    shader.set_uniform(
        u::VIEW_MATRIX,
        Mat4::look_at_rh(Vec3::new(0.0, 0.0, 3.0), Vec3::ZERO, Vec3::Y),
    );
    shader.set_uniform(
        u::PROJECTION_MATRIX,
        Mat4::perspective_rh_gl(
            60.0_f32.to_radians(),
            width as f32 / height as f32,
            0.1,
            100.0,
        ),
    );

    loop {
        // 消费所有待处理命令（非阻塞）
        while let Ok(cmd) = cmd_rx.try_recv() {
            match cmd {
                RenderCommand::LoadModel(path) => match Model::load(&path.to_string_lossy()) {
                    Ok(m) => {
                        tracing::info!("模型加载成功: {}", path.display());
                        model = Some(m);
                    }
                    Err(e) => {
                        tracing::error!("模型加载失败: {}", e);
                    }
                },
                RenderCommand::SetCamera {
                    view,
                    projection,
                    pos,
                } => {
                    shader.set_uniform(u::VIEW_MATRIX, view);
                    shader.set_uniform(u::PROJECTION_MATRIX, projection);
                    shader.set_uniform(u::CAMERA_POS, pos);
                }
                RenderCommand::SetRenderingMode(mode) => {
                    renderer.set_rendering_mode(mode);
                }
                RenderCommand::SetTileSize(size) => {
                    renderer.set_tile_size(size);
                }
                RenderCommand::SetEarlyZ(enabled) => {
                    renderer.set_early_z_enabled(enabled);
                }
                RenderCommand::SetLights(lights) => {
                    shader.set_lights(&lights);
                }
                RenderCommand::SetShininess(s) => {
                    shader.set_uniform("shininess", s);
                }
                RenderCommand::Resize {
                    width: w,
                    height: h,
                } => {
                    width = w;
                    height = h;
                    renderer = SimpleRenderer::new(width, height);
                }
                RenderCommand::Shutdown => return,
            }
        }

        // 渲染一帧
        if let Some(ref m) = model {
            let frame_start = Instant::now();

            writer.clear(Color::BLACK);
            let buf = writer.render_buffer_mut();

            // 确保缓冲区尺寸匹配
            if buf.len() == width * height {
                if let Err(e) = renderer.draw_model(m, &mut shader, buf) {
                    tracing::error!("渲染失败: {}", e);
                }
            }

            writer.publish();

            let elapsed = frame_start.elapsed().as_micros() as u64;
            frame_time_us.store(elapsed, Ordering::Relaxed);
        } else {
            // 无模型时清屏并发布，避免显示垃圾数据
            writer.clear(Color::new(40, 40, 40, 255));
            writer.publish();

            // 无模型时稍微等待，避免空转
            std::thread::sleep(std::time::Duration::from_millis(16));
        }
    }
}
