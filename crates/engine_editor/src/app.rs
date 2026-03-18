use std::path::PathBuf;
use std::time::Instant;

use egui::TextureHandle;
use engine_render_sw::{Color, Light, RenderingMode};
use glam::Vec3;

use crate::camera_control::EditorCamera;
use crate::panels;
use crate::render_bridge::{RenderBridge, RenderCommand};

const DEFAULT_RENDER_WIDTH: usize = 800;
const DEFAULT_RENDER_HEIGHT: usize = 600;

/// 编辑器 UI 状态（对应属性面板中的各项参数）
pub struct EditorState {
    pub rendering_mode_index: usize,
    pub rendering_mode_changed: bool,
    pub tile_size: usize,
    pub tile_size_changed: bool,
    pub early_z: bool,
    pub early_z_changed: bool,
    pub vsync: bool,
    pub light_direction: [f32; 3],
    pub light_color: egui::Color32,
    pub light_changed: bool,
    pub shininess: f32,
    pub shininess_changed: bool,
    pub camera_speed: f32,
    pub camera_fov: f32,
}

impl Default for EditorState {
    fn default() -> Self {
        Self {
            rendering_mode_index: 1, // TileBased
            rendering_mode_changed: false,
            tile_size: 64,
            tile_size_changed: false,
            early_z: true,
            early_z_changed: false,
            vsync: false,
            light_direction: [1.0, 5.0, 1.0],
            light_color: egui::Color32::WHITE,
            light_changed: false,
            shininess: 32.0,
            shininess_changed: false,
            camera_speed: 2.5,
            camera_fov: 60.0,
        }
    }
}

pub struct EditorApp {
    render_bridge: RenderBridge,
    texture: Option<TextureHandle>,
    camera: EditorCamera,
    state: EditorState,
    scene_entities: Vec<String>,
    selected_entity: Option<usize>,
    model_loaded: bool,
    display_fps: f32,
    frame_count: u32,
    fps_timer: Instant,
    last_frame: Instant,
}

impl EditorApp {
    pub fn new(cc: &eframe::CreationContext<'_>) -> Self {
        Self::configure_cjk_fonts(&cc.egui_ctx);
        let render_bridge = RenderBridge::new(DEFAULT_RENDER_WIDTH, DEFAULT_RENDER_HEIGHT);

        // 尝试加载默认茶壶模型
        let default_model = PathBuf::from("assets/models/utah-teapot-texture/teapot.obj");
        let model_loaded = if default_model.exists() {
            render_bridge.send(RenderCommand::LoadModel(default_model));
            true
        } else {
            log::warn!("默认模型路径不存在，等待用户手动加载");
            false
        };

        let mut scene_entities = Vec::new();
        if model_loaded {
            scene_entities.push("🫖 Teapot".to_string());
        }
        scene_entities.push("💡 主光源".to_string());
        scene_entities.push("📷 编辑器相机".to_string());

        Self {
            render_bridge,
            texture: None,
            camera: EditorCamera::default(),
            state: EditorState::default(),
            scene_entities,
            selected_entity: None,
            model_loaded,
            display_fps: 0.0,
            frame_count: 0,
            fps_timer: Instant::now(),
            last_frame: Instant::now(),
        }
    }

    fn configure_cjk_fonts(ctx: &egui::Context) {
        let font_paths = [
            "3rd/LxgwWenKai-Lite/fonts/TTF/LXGWWenKaiLite-Regular.ttf",
            "/usr/share/fonts/opentype/noto/NotoSansCJK-Regular.ttc",
            "/usr/share/fonts/truetype/noto/NotoSansCJK-Regular.ttc",
        ];

        let font_data = font_paths.iter().find_map(|path| std::fs::read(path).ok());

        if let Some(data) = font_data {
            let mut fonts = egui::FontDefinitions::default();
            fonts
                .font_data
                .insert("cjk".to_owned(), egui::FontData::from_owned(data).into());
            fonts
                .families
                .entry(egui::FontFamily::Proportional)
                .or_default()
                .push("cjk".to_owned());
            fonts
                .families
                .entry(egui::FontFamily::Monospace)
                .or_default()
                .push("cjk".to_owned());
            ctx.set_fonts(fonts);
        } else {
            log::warn!("未找到 CJK 字体，中文可能无法显示。请运行: git submodule update --init");
        }
    }

    fn handle_menu_bar(&mut self, ctx: &egui::Context) {
        egui::TopBottomPanel::top("menu_bar").show(ctx, |ui| {
            egui::menu::bar(ui, |ui| {
                ui.menu_button("文件", |ui| {
                    if ui.button("打开模型...").clicked() {
                        ui.close_menu();
                        self.open_model_dialog();
                    }
                    ui.separator();
                    if ui.button("退出").clicked() {
                        ctx.send_viewport_cmd(egui::ViewportCommand::Close);
                    }
                });
                ui.menu_button("视图", |ui| {
                    if ui.button("重置相机").clicked() {
                        self.camera = EditorCamera::default();
                        ui.close_menu();
                    }
                });
            });
        });
    }

    fn open_model_dialog(&mut self) {
        if let Some(path) = rfd::FileDialog::new()
            .add_filter("OBJ 模型", &["obj"])
            .pick_file()
        {
            let name = path
                .file_stem()
                .map(|s| s.to_string_lossy().to_string())
                .unwrap_or_else(|| "Model".to_string());

            self.render_bridge.send(RenderCommand::LoadModel(path));

            if !self.model_loaded {
                self.scene_entities.insert(0, format!("🧊 {}", name));
            } else {
                self.scene_entities[0] = format!("🧊 {}", name);
            }
            self.model_loaded = true;
        }
    }

    fn send_state_changes(&mut self) {
        if self.state.rendering_mode_changed {
            let mode = match self.state.rendering_mode_index {
                0 => RenderingMode::PerTriangle,
                1 => RenderingMode::TileBased,
                2 => RenderingMode::Deferred,
                3 => RenderingMode::TileBasedDeferred,
                _ => RenderingMode::TileBased,
            };
            self.render_bridge
                .send(RenderCommand::SetRenderingMode(mode));
            self.state.rendering_mode_changed = false;
        }

        if self.state.tile_size_changed {
            self.render_bridge
                .send(RenderCommand::SetTileSize(self.state.tile_size));
            self.state.tile_size_changed = false;
        }

        if self.state.early_z_changed {
            self.render_bridge
                .send(RenderCommand::SetEarlyZ(self.state.early_z));
            self.state.early_z_changed = false;
        }

        if self.state.light_changed {
            let lc = self.state.light_color;
            let light = Light {
                name: "主光源".to_string(),
                direction: Vec3::from_array(self.state.light_direction),
                color: Color::new(lc.r(), lc.g(), lc.b(), lc.a()),
                ..Light::default()
            };
            self.render_bridge
                .send(RenderCommand::SetLights(vec![light]));
            self.state.light_changed = false;
        }

        if self.state.shininess_changed {
            self.render_bridge
                .send(RenderCommand::SetShininess(self.state.shininess));
            self.state.shininess_changed = false;
        }
    }

    fn handle_camera_input(
        &mut self,
        ctx: &egui::Context,
        viewport_response: Option<&egui::Response>,
    ) {
        let now = Instant::now();
        let dt = now.duration_since(self.last_frame).as_secs_f32();
        self.last_frame = now;

        self.camera.movement_speed = self.state.camera_speed;
        self.camera.fov = self.state.camera_fov;

        // 处理视口内的右键拖拽旋转
        if let Some(resp) = viewport_response {
            if resp.dragged_by(egui::PointerButton::Secondary) {
                let delta = resp.drag_delta();
                self.camera.process_mouse(delta.x, delta.y);
            }
        }

        // 处理 WASD + QE 键盘移动
        ctx.input(|input| {
            if input.key_down(egui::Key::W) {
                self.camera.move_forward(dt);
            }
            if input.key_down(egui::Key::S) {
                self.camera.move_backward(dt);
            }
            if input.key_down(egui::Key::A) {
                self.camera.move_left(dt);
            }
            if input.key_down(egui::Key::D) {
                self.camera.move_right(dt);
            }
            if input.key_down(egui::Key::Q) || input.key_down(egui::Key::Space) {
                self.camera.move_up(dt);
            }
            if input.key_down(egui::Key::E) {
                self.camera.move_down(dt);
            }
        });

        let aspect = self.render_bridge.width() as f32 / self.render_bridge.height() as f32;
        self.render_bridge.send(RenderCommand::SetCamera {
            view: self.camera.view_matrix(),
            projection: self.camera.projection_matrix(aspect),
            pos: self.camera.position(),
        });
    }

    fn update_fps(&mut self) {
        self.frame_count += 1;
        let elapsed = self.fps_timer.elapsed().as_secs_f32();
        if elapsed >= 1.0 {
            self.display_fps = self.frame_count as f32 / elapsed;
            self.frame_count = 0;
            self.fps_timer = Instant::now();
        }
    }
}

impl eframe::App for EditorApp {
    fn update(&mut self, ctx: &egui::Context, _frame: &mut eframe::Frame) {
        // 持续重绘（实时渲染）
        ctx.request_repaint();

        self.update_fps();

        // 从三重缓冲读取最新帧并上传为 egui 纹理
        if self.render_bridge.try_read_frame() {
            let w = self.render_bridge.width();
            let h = self.render_bridge.height();
            let pixels = self.render_bridge.front_buffer();
            panels::viewport::upload_frame(ctx, &mut self.texture, pixels, w, h);
        }

        // 菜单栏
        self.handle_menu_bar(ctx);

        // 底部状态栏
        let render_time_ms = self.render_bridge.frame_time_us() as f32 / 1000.0;
        let mode_names = ["PerTriangle", "TileBased", "Deferred", "TileBasedDeferred"];
        let mode_name = mode_names[self.state.rendering_mode_index];
        egui::TopBottomPanel::bottom("status_bar").show(ctx, |ui| {
            panels::status_bar::show(
                ui,
                self.display_fps,
                render_time_ms,
                mode_name,
                self.model_loaded,
            );
        });

        // 左侧场景树
        egui::SidePanel::left("scene_tree")
            .default_width(180.0)
            .show(ctx, |ui| {
                panels::scene_tree::show(ui, &self.scene_entities, &mut self.selected_entity);
            });

        // 右侧属性面板
        egui::SidePanel::right("properties")
            .default_width(240.0)
            .show(ctx, |ui| {
                panels::properties::show(ui, &mut self.state);
            });

        // 中央视口
        let mut viewport_response = None;
        egui::CentralPanel::default().show(ctx, |ui| {
            viewport_response = panels::viewport::show(ui, &self.texture);
        });

        // 处理相机输入
        self.handle_camera_input(ctx, viewport_response.as_ref());

        // 发送 UI 状态变更到渲染线程
        self.send_state_changes();
    }
}
