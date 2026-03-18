use std::path::PathBuf;
use std::time::Instant;

use egui::TextureHandle;
use engine_render_sw::{Color, Light, RenderingMode};
use glam::Vec3;

use crate::camera_control::EditorCamera;
use crate::logger::LogBuffer;
use crate::panels;
use crate::panels::console::ConsoleState;
use crate::project::ProjectState;
use crate::render_bridge::{RenderBridge, RenderCommand};

const DEFAULT_RENDER_WIDTH: usize = 800;
const DEFAULT_RENDER_HEIGHT: usize = 600;

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
            rendering_mode_index: 1,
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

    log_buffer: LogBuffer,
    console_state: ConsoleState,
    project: Option<ProjectState>,
    show_console: bool,
}

impl EditorApp {
    pub fn new(cc: &eframe::CreationContext<'_>, log_buffer: LogBuffer) -> Self {
        Self::configure_cjk_fonts(&cc.egui_ctx);
        let render_bridge = RenderBridge::new(DEFAULT_RENDER_WIDTH, DEFAULT_RENDER_HEIGHT);

        let default_model = PathBuf::from("assets/models/utah-teapot-texture/teapot.obj");
        let model_loaded = if default_model.exists() {
            render_bridge.send(RenderCommand::LoadModel(default_model));
            true
        } else {
            tracing::warn!("默认模型路径不存在，等待用户手动加载");
            false
        };

        let mut scene_entities = Vec::new();
        if model_loaded {
            scene_entities.push("🫖 Teapot".to_string());
        }
        scene_entities.push("💡 主光源".to_string());
        scene_entities.push("📷 编辑器相机".to_string());

        tracing::info!("编辑器启动完成");

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
            log_buffer,
            console_state: ConsoleState::default(),
            project: None,
            show_console: true,
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
            tracing::warn!(
                "未找到 CJK 字体，中文可能无法显示。请运行: git submodule update --init"
            );
        }
    }

    fn handle_menu_bar(&mut self, ctx: &egui::Context) {
        egui::TopBottomPanel::top("menu_bar").show(ctx, |ui| {
            egui::menu::bar(ui, |ui| {
                ui.menu_button("文件", |ui| {
                    if ui.button("📁 新建项目...").clicked() {
                        ui.close_menu();
                        self.new_project_dialog();
                    }
                    if ui.button("📂 打开项目...").clicked() {
                        ui.close_menu();
                        self.open_project_dialog();
                    }

                    let has_project = self.project.is_some();
                    ui.add_enabled_ui(has_project, |ui| {
                        if ui.button("💾 保存项目").clicked() {
                            ui.close_menu();
                            self.save_project();
                        }
                    });

                    ui.separator();

                    ui.add_enabled_ui(has_project, |ui| {
                        if ui.button("📦 导入模型...").clicked() {
                            ui.close_menu();
                            self.import_model_dialog();
                        }
                        if ui.button("🖼 导入纹理...").clicked() {
                            ui.close_menu();
                            self.import_texture_dialog();
                        }
                    });

                    ui.separator();

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
                    ui.separator();
                    if ui.selectable_label(self.show_console, "控制台").clicked() {
                        self.show_console = !self.show_console;
                        ui.close_menu();
                    }
                });
            });
        });
    }

    fn new_project_dialog(&mut self) {
        if let Some(dir) = rfd::FileDialog::new()
            .set_title("选择新项目目录")
            .pick_folder()
        {
            let name = dir
                .file_name()
                .map(|s| s.to_string_lossy().to_string())
                .unwrap_or_else(|| "新项目".to_string());

            match ProjectState::create(&dir, &name) {
                Ok(state) => {
                    tracing::info!("新建项目: {} -> {}", name, dir.display());
                    self.project = Some(state);
                }
                Err(e) => {
                    tracing::error!("创建项目失败: {}", e);
                }
            }
        }
    }

    fn open_project_dialog(&mut self) {
        if let Some(dir) = rfd::FileDialog::new()
            .set_title("选择项目目录")
            .pick_folder()
        {
            match ProjectState::open(&dir) {
                Ok(state) => {
                    self.apply_project_config(&state);
                    tracing::info!("打开项目: {}", state.name());
                    self.project = Some(state);
                }
                Err(e) => {
                    tracing::error!("打开项目失败: {}", e);
                }
            }
        }
    }

    fn save_project(&mut self) {
        if let Some(ref mut proj) = self.project {
            Self::sync_state_to_project(&self.state, proj);
            match proj.save() {
                Ok(()) => {
                    proj.dirty = false;
                    tracing::info!("项目已保存");
                }
                Err(e) => {
                    tracing::error!("保存项目失败: {}", e);
                }
            }
        }
    }

    fn import_model_dialog(&mut self) {
        if let Some(path) = rfd::FileDialog::new()
            .add_filter("3D 模型", &["obj"])
            .set_title("选择要导入的模型")
            .pick_file()
        {
            if let Some(ref mut proj) = self.project {
                match proj.import_model(&path) {
                    Ok(entry) => {
                        let model_path = proj.resolve_model_path(&entry);
                        self.render_bridge
                            .send(RenderCommand::LoadModel(model_path));

                        if !self.model_loaded {
                            self.scene_entities.insert(0, format!("🧊 {}", entry.name));
                        } else {
                            self.scene_entities[0] = format!("🧊 {}", entry.name);
                        }
                        self.model_loaded = true;
                    }
                    Err(e) => {
                        tracing::error!("导入模型失败: {}", e);
                    }
                }
            }
        }
    }

    fn import_texture_dialog(&mut self) {
        if let Some(path) = rfd::FileDialog::new()
            .add_filter("图片文件", &["png", "jpg", "jpeg", "bmp", "tga"])
            .set_title("选择要导入的纹理")
            .pick_file()
        {
            if let Some(ref mut proj) = self.project {
                match proj.import_texture(&path) {
                    Ok(entry) => {
                        tracing::info!("纹理已导入到项目: {}", entry.name);
                    }
                    Err(e) => {
                        tracing::error!("导入纹理失败: {}", e);
                    }
                }
            }
        }
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

    fn apply_project_config(&mut self, proj: &ProjectState) {
        let scene = &proj.project.scene;

        self.camera
            .set_position(Vec3::from_array(scene.camera.position));
        self.camera
            .set_rotation(scene.camera.yaw, scene.camera.pitch);
        self.state.camera_speed = scene.camera.speed;
        self.state.camera_fov = scene.camera.fov;

        self.state.light_direction = scene.light.direction;
        self.state.light_color = egui::Color32::from_rgba_premultiplied(
            scene.light.color[0],
            scene.light.color[1],
            scene.light.color[2],
            scene.light.color[3],
        );
        self.state.light_changed = true;

        self.state.rendering_mode_index = match scene.render.mode.as_str() {
            "PerTriangle" => 0,
            "TileBased" => 1,
            "Deferred" => 2,
            "TileBasedDeferred" => 3,
            _ => 1,
        };
        self.state.rendering_mode_changed = true;
        self.state.tile_size = scene.render.tile_size;
        self.state.tile_size_changed = true;
        self.state.early_z = scene.render.early_z;
        self.state.early_z_changed = true;
        self.state.shininess = scene.render.shininess;
        self.state.shininess_changed = true;

        if let Some(entry) = proj.project.assets.models.first() {
            let model_path = proj.resolve_model_path(entry);
            if model_path.exists() {
                self.render_bridge
                    .send(RenderCommand::LoadModel(model_path));

                if !self.model_loaded {
                    self.scene_entities.insert(0, format!("🧊 {}", entry.name));
                } else {
                    self.scene_entities[0] = format!("🧊 {}", entry.name);
                }
                self.model_loaded = true;
            }
        }
    }

    fn sync_state_to_project(state: &EditorState, proj: &mut ProjectState) {
        let mode_names = ["PerTriangle", "TileBased", "Deferred", "TileBasedDeferred"];
        proj.project.scene.render.mode = mode_names[state.rendering_mode_index].to_string();
        proj.project.scene.render.tile_size = state.tile_size;
        proj.project.scene.render.early_z = state.early_z;
        proj.project.scene.render.shininess = state.shininess;
        proj.project.scene.light.direction = state.light_direction;
        proj.project.scene.light.color = [
            state.light_color.r(),
            state.light_color.g(),
            state.light_color.b(),
            state.light_color.a(),
        ];
        proj.project.scene.camera.fov = state.camera_fov;
        proj.project.scene.camera.speed = state.camera_speed;
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

        if let Some(resp) = viewport_response {
            if resp.dragged_by(egui::PointerButton::Secondary) {
                let delta = resp.drag_delta();
                self.camera.process_mouse(delta.x, delta.y);
            }
        }

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
        ctx.request_repaint();

        self.update_fps();

        if self.render_bridge.try_read_frame() {
            let w = self.render_bridge.width();
            let h = self.render_bridge.height();
            let pixels = self.render_bridge.front_buffer();
            panels::viewport::upload_frame(ctx, &mut self.texture, pixels, w, h);
        }

        self.handle_menu_bar(ctx);

        let render_time_ms = self.render_bridge.frame_time_us() as f32 / 1000.0;
        let mode_names = ["PerTriangle", "TileBased", "Deferred", "TileBasedDeferred"];
        let mode_name = mode_names[self.state.rendering_mode_index];
        let project_name = self
            .project
            .as_ref()
            .map(|p| p.name().to_string())
            .unwrap_or_default();
        let project_dirty = self.project.as_ref().map(|p| p.dirty).unwrap_or(false);
        egui::TopBottomPanel::bottom("status_bar")
            .exact_height(24.0)
            .show(ctx, |ui| {
                panels::status_bar::show(
                    ui,
                    self.display_fps,
                    render_time_ms,
                    mode_name,
                    self.model_loaded,
                    &project_name,
                    project_dirty,
                );
            });

        if self.show_console {
            egui::TopBottomPanel::bottom("console_panel")
                .resizable(true)
                .default_height(160.0)
                .min_height(80.0)
                .show(ctx, |ui| {
                    panels::console::show(ui, &self.log_buffer, &mut self.console_state);
                });
        }

        egui::SidePanel::left("scene_tree")
            .default_width(180.0)
            .show(ctx, |ui| {
                panels::scene_tree::show(ui, &self.scene_entities, &mut self.selected_entity);
            });

        egui::SidePanel::right("properties")
            .default_width(240.0)
            .show(ctx, |ui| {
                panels::properties::show(ui, &mut self.state);
            });

        let mut viewport_response = None;
        egui::CentralPanel::default().show(ctx, |ui| {
            viewport_response = panels::viewport::show(ui, &self.texture);
        });

        self.handle_camera_input(ctx, viewport_response.as_ref());
        self.send_state_changes();
    }
}
