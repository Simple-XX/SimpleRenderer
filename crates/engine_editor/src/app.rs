use std::path::PathBuf;
use std::time::Instant;

use egui::TextureHandle;
use engine_render_sw::{Color, Light, RenderingMode};
use glam::Vec3;

use crate::camera_control::EditorCamera;
use crate::commands::CommandHistory;
use crate::gizmo::GizmoState;
use crate::logger::LogBuffer;
use crate::panels;
use crate::panels::console::ConsoleState;
use crate::panels::scene_tree::SceneTreeState;
use crate::project::ProjectState;
use crate::render_bridge::{DrawCallData, RenderBridge, RenderCommand};
use crate::selection::SelectionState;

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
    #[allow(dead_code)]
    pub show_grid: bool,
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
            show_grid: true,
        }
    }
}

pub struct EditorApp {
    render_bridge: RenderBridge,
    texture: Option<TextureHandle>,
    camera: EditorCamera,
    state: EditorState,
    scene: engine_scene::Scene,
    selection: SelectionState,
    gizmo: GizmoState,
    commands: CommandHistory,
    show_scene_tree: bool,
    show_properties: bool,
    display_fps: f32,
    frame_count: u32,
    fps_timer: Instant,
    last_frame: Instant,

    log_buffer: LogBuffer,
    console_state: ConsoleState,
    scene_tree_state: SceneTreeState,
    project: Option<ProjectState>,
    show_console: bool,

    shortcuts_help_state: crate::panels::shortcuts_help::ShortcutsHelpState,
    toast_manager: crate::panels::toast::ToastManager,
}

impl EditorApp {
    pub fn new(cc: &eframe::CreationContext<'_>, log_buffer: LogBuffer) -> Self {
        Self::configure_cjk_fonts(&cc.egui_ctx);
        let render_bridge = RenderBridge::new(DEFAULT_RENDER_WIDTH, DEFAULT_RENDER_HEIGHT);

        let mut scene = engine_scene::Scene::new();

        let default_model = PathBuf::from("assets/models/utah-teapot-texture/teapot.obj");
        if default_model.exists() {
            let path_str = default_model.to_string_lossy().to_string();
            match scene.spawn_model("Teapot", &path_str) {
                Ok(entity) => {
                    let model_handle = scene
                        .world
                        .get::<&engine_scene::MeshRenderer>(entity)
                        .unwrap()
                        .model_handle;
                    if let Some(model) = scene.assets.get_model(model_handle) {
                        render_bridge.send(RenderCommand::RegisterModel {
                            id: model_handle.id(),
                            model: model.clone(),
                        });
                    }
                }
                Err(e) => {
                    tracing::warn!("默认模型加载失败: {}", e);
                }
            }
        } else {
            tracing::warn!("默认模型路径不存在，等待用户手动加载");
        }

        scene.spawn_light(
            "主光源",
            Vec3::new(1.0, 5.0, 1.0),
            Color::new(255, 255, 255, 255),
        );
        scene.spawn_camera("编辑器相机");

        tracing::info!("编辑器启动完成");

        Self {
            render_bridge,
            texture: None,
            camera: EditorCamera::default(),
            state: EditorState::default(),
            scene,
            selection: SelectionState::default(),
            gizmo: GizmoState::new(),
            commands: CommandHistory::default(),
            show_scene_tree: true,
            show_properties: true,
            display_fps: 0.0,
            frame_count: 0,
            fps_timer: Instant::now(),
            last_frame: Instant::now(),
            log_buffer,
            console_state: ConsoleState::default(),
            scene_tree_state: SceneTreeState::default(),
            project: None,
            show_console: true,
            shortcuts_help_state: crate::panels::shortcuts_help::ShortcutsHelpState::default(),
            toast_manager: crate::panels::toast::ToastManager::default(),
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

                ui.menu_button("编辑", |ui| {
                    if ui
                        .add_enabled(self.commands.can_undo(), egui::Button::new("↩️ 撤销"))
                        .clicked()
                    {
                        self.commands.undo(&mut self.scene);
                        ui.close_menu();
                    }
                    if ui
                        .add_enabled(self.commands.can_redo(), egui::Button::new("↪️ 重做"))
                        .clicked()
                    {
                        self.commands.redo(&mut self.scene);
                        ui.close_menu();
                    }
                    ui.separator();
                    if ui
                        .add_enabled(
                            self.selection.selected.is_some(),
                            egui::Button::new("🗑 删除"),
                        )
                        .clicked()
                    {
                        if let Some(entity) = self.selection.selected {
                            let _ = self.scene.world.despawn(entity);
                            self.selection.clear();
                        }
                        ui.close_menu();
                    }
                });

                ui.menu_button("实体", |ui| {
                    if ui.button("📦 空实体").clicked() {
                        let name = format!("实体 {}", self.scene.world.len());
                        self.scene
                            .world
                            .spawn((engine_scene::Name(name), engine_core::Transform::default()));
                        ui.close_menu();
                    }
                    if ui.button("💡 灯光").clicked() {
                        self.scene.spawn_light(
                            "灯光",
                            glam::Vec3::new(0.0, -1.0, 0.0),
                            engine_render_sw::Color::WHITE,
                        );
                        ui.close_menu();
                    }
                    if ui.button("📷 相机").clicked() {
                        self.scene.spawn_camera("相机");
                        ui.close_menu();
                    }
                });

                ui.menu_button("视图", |ui| {
                    if ui
                        .selectable_label(self.show_scene_tree, "场景树")
                        .clicked()
                    {
                        self.show_scene_tree = !self.show_scene_tree;
                        ui.close_menu();
                    }
                    if ui
                        .selectable_label(self.show_properties, "属性面板")
                        .clicked()
                    {
                        self.show_properties = !self.show_properties;
                        ui.close_menu();
                    }
                    if ui.selectable_label(self.show_console, "控制台").clicked() {
                        self.show_console = !self.show_console;
                        ui.close_menu();
                    }
                    ui.separator();
                    if ui.button("重置相机").clicked() {
                        self.camera = EditorCamera::default();
                        ui.close_menu();
                    }
                });

                ui.menu_button("帮助", |ui| {
                    if ui.button("⌨ 快捷键参考 (F1)").clicked() {
                        self.shortcuts_help_state.open = true;
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
                        let path_str = model_path.to_string_lossy().to_string();
                        match self.scene.spawn_model(&entry.name, &path_str) {
                            Ok(entity) => {
                                let model_handle = self
                                    .scene
                                    .world
                                    .get::<&engine_scene::MeshRenderer>(entity)
                                    .unwrap()
                                    .model_handle;
                                if let Some(model) = self.scene.assets.get_model(model_handle) {
                                    self.render_bridge.send(RenderCommand::RegisterModel {
                                        id: model_handle.id(),
                                        model: model.clone(),
                                    });
                                }
                            }
                            Err(e) => {
                                tracing::error!("加载导入模型失败: {}", e);
                            }
                        }
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

            let path_str = path.to_string_lossy().to_string();
            match self.scene.spawn_model(&name, &path_str) {
                Ok(entity) => {
                    let model_handle = self
                        .scene
                        .world
                        .get::<&engine_scene::MeshRenderer>(entity)
                        .unwrap()
                        .model_handle;
                    if let Some(model) = self.scene.assets.get_model(model_handle) {
                        self.render_bridge.send(RenderCommand::RegisterModel {
                            id: model_handle.id(),
                            model: model.clone(),
                        });
                    }
                }
                Err(e) => {
                    tracing::error!("模型加载失败: {}", e);
                }
            }
        }
    }

    fn apply_project_config(&mut self, proj: &ProjectState) {
        let scene_cfg = &proj.project.scene;

        self.camera
            .set_position(Vec3::from_array(scene_cfg.camera.position));
        self.camera
            .set_rotation(scene_cfg.camera.yaw, scene_cfg.camera.pitch);
        self.state.camera_speed = scene_cfg.camera.speed;
        self.state.camera_fov = scene_cfg.camera.fov;

        self.state.light_direction = scene_cfg.light.direction;
        self.state.light_color = egui::Color32::from_rgba_premultiplied(
            scene_cfg.light.color[0],
            scene_cfg.light.color[1],
            scene_cfg.light.color[2],
            scene_cfg.light.color[3],
        );
        self.state.light_changed = true;

        self.state.rendering_mode_index = match scene_cfg.render.mode.as_str() {
            "PerTriangle" => 0,
            "TileBased" => 1,
            "Deferred" => 2,
            "TileBasedDeferred" => 3,
            _ => 1,
        };
        self.state.rendering_mode_changed = true;
        self.state.tile_size = scene_cfg.render.tile_size;
        self.state.tile_size_changed = true;
        self.state.early_z = scene_cfg.render.early_z;
        self.state.early_z_changed = true;
        self.state.shininess = scene_cfg.render.shininess;
        self.state.shininess_changed = true;

        if let Some(entry) = proj.project.assets.models.first() {
            let model_path = proj.resolve_model_path(entry);
            if model_path.exists() {
                let path_str = model_path.to_string_lossy().to_string();
                match self.scene.spawn_model(&entry.name, &path_str) {
                    Ok(entity) => {
                        let model_handle = self
                            .scene
                            .world
                            .get::<&engine_scene::MeshRenderer>(entity)
                            .unwrap()
                            .model_handle;
                        if let Some(model) = self.scene.assets.get_model(model_handle) {
                            self.render_bridge.send(RenderCommand::RegisterModel {
                                id: model_handle.id(),
                                model: model.clone(),
                            });
                        }
                    }
                    Err(e) => {
                        tracing::error!("项目模型加载失败: {}", e);
                    }
                }
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

    fn collect_and_submit_frame(&self) {
        use engine_core::Transform;
        use engine_scene::MeshRenderer;

        let mut draw_calls = Vec::new();
        for (entity, (transform, mesh_renderer)) in self
            .scene
            .world
            .query::<(&Transform, &MeshRenderer)>()
            .iter()
        {
            draw_calls.push(DrawCallData {
                model_id: mesh_renderer.model_handle.id(),
                model_matrix: transform.to_mat4(),
                entity_id: entity.to_bits().get() as u32,
            });
        }
        if !draw_calls.is_empty() {
            self.render_bridge
                .send(RenderCommand::SubmitFrame(draw_calls));
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

    fn has_model_loaded(&self) -> bool {
        self.scene
            .world
            .query::<&engine_scene::MeshRenderer>()
            .iter()
            .next()
            .is_some()
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

        egui::TopBottomPanel::top("toolbar").show(ctx, |ui| {
            panels::toolbar::show(ui, &mut self.gizmo);
        });

        let render_time_ms = self.render_bridge.frame_time_us() as f32 / 1000.0;
        let mode_names = ["PerTriangle", "TileBased", "Deferred", "TileBasedDeferred"];
        let mode_name = mode_names[self.state.rendering_mode_index];
        let project_name = self
            .project
            .as_ref()
            .map(|p| p.name().to_string())
            .unwrap_or_default();
        let project_dirty = self.project.as_ref().map(|p| p.dirty).unwrap_or(false);
        let model_loaded = self.has_model_loaded();

        // 提取选中实体信息
        let selected_name = self.selection.selected.and_then(|e| {
            self.scene
                .world
                .get::<&engine_scene::Name>(e)
                .ok()
                .map(|n| n.0.clone())
        });
        let selected_pos = self.selection.selected.and_then(|e| {
            self.scene
                .world
                .get::<&engine_core::Transform>(e)
                .ok()
                .map(|t| t.translation.to_array())
        });
        let undo_desc = self.commands.undo_description().map(|s| s.to_string());
        let redo_desc = self.commands.redo_description().map(|s| s.to_string());

        egui::TopBottomPanel::bottom("status_bar")
            .exact_height(24.0)
            .show(ctx, |ui| {
                panels::status_bar::show(
                    ui,
                    self.display_fps,
                    render_time_ms,
                    mode_name,
                    model_loaded,
                    &project_name,
                    project_dirty,
                    selected_name.as_deref(),
                    selected_pos.as_ref().copied(),
                    undo_desc.as_deref(),
                    redo_desc.as_deref(),
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

        if self.show_scene_tree {
            egui::SidePanel::left("scene_tree")
                .default_width(180.0)
                .show(ctx, |ui| {
                    panels::scene_tree::show(
                        ui,
                        &mut self.scene,
                        &mut self.selection,
                        &mut self.scene_tree_state,
                    );
                });
        }

        if self.show_properties {
            egui::SidePanel::right("properties")
                .default_width(240.0)
                .show(ctx, |ui| {
                    panels::properties::show(ui, &mut self.scene, &self.selection, &mut self.state);
                });
        }

        let mut viewport_response = None;
        egui::CentralPanel::default().show(ctx, |ui| {
            viewport_response = panels::viewport::show(ui, &self.texture);
        });

        self.handle_camera_input(ctx, viewport_response.as_ref());

        // 快捷键处理
        if !ctx.wants_keyboard_input() {
            let input = ctx.input(|i| {
                let q = i.key_pressed(egui::Key::Q);
                let w = i.key_pressed(egui::Key::W);
                let e = i.key_pressed(egui::Key::E);
                let r = i.key_pressed(egui::Key::R);
                let ctrl_z =
                    i.modifiers.command && i.key_pressed(egui::Key::Z) && !i.modifiers.shift;
                let ctrl_shift_z =
                    i.modifiers.command && i.key_pressed(egui::Key::Z) && i.modifiers.shift;
                let delete =
                    i.key_pressed(egui::Key::Delete) || i.key_pressed(egui::Key::Backspace);
                let escape = i.key_pressed(egui::Key::Escape);
                let f1 = i.key_pressed(egui::Key::F1);
                (q, w, e, r, ctrl_z, ctrl_shift_z, delete, escape, f1)
            });

            if input.0 {
                self.gizmo.tool = crate::gizmo::GizmoTool::Select;
            }
            if input.1 {
                self.gizmo.tool = crate::gizmo::GizmoTool::Translate;
            }
            if input.2 {
                self.gizmo.tool = crate::gizmo::GizmoTool::Rotate;
            }
            if input.3 {
                self.gizmo.tool = crate::gizmo::GizmoTool::Scale;
            }
            if input.4 {
                self.commands.undo(&mut self.scene);
            }
            if input.5 {
                self.commands.redo(&mut self.scene);
            }
            if input.6 {
                if let Some(entity) = self.selection.selected {
                    let _ = self.scene.world.despawn(entity);
                    self.selection.clear();
                }
            }
            if input.7 {
                self.selection.clear();
            }
            if input.8 {
                self.shortcuts_help_state.open = true;
            }
        }

        panels::shortcuts_help::show(ctx, &mut self.shortcuts_help_state);
        panels::toast::show(ctx, &mut self.toast_manager);

        self.collect_and_submit_frame();
        self.send_state_changes();
    }
}
