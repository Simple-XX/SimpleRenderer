# SimpleEngine 编辑器原型实施计划

> **For agentic workers:** REQUIRED: Use superpowers:subagent-driven-development (if subagents available) or superpowers:executing-plans to implement this plan. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 将 SimpleRenderer 重构为模块化游戏引擎 workspace，交付可工作的 egui 编辑器原型（场景视口 + 属性面板 + 资产加载 + 参数调整）。

**Architecture:** 4 个 crate 分层架构。`engine_core`（ECS）和 `engine_renderer`（纯渲染，由 simple_renderer 就地重命名）是并行的底层 crate，互不依赖。`engine_scene` 依赖两者，负责从 ECS 提取数据并喂给渲染器。`engine_editor` 是顶层 egui 应用，依赖所有下层 crate。

**Tech Stack:** Rust, glam, rayon, eframe/egui, rfd, tobj, image, thiserror

**Spec:** `docs/specs/2026-03-18-engine-editor-design.md`

---

## 文件结构

### 新建文件

```
crates/
├── engine_core/
│   ├── Cargo.toml
│   └── src/
│       ├── lib.rs              # 模块声明 + 公开 re-export
│       ├── ecs.rs              # Entity, Component, World
│       ├── system.rs           # System trait, SystemRunner
│       ├── event.rs            # 事件通道 (EventBus)
│       └── time.rs             # Time (delta, total, frame count)
├── engine_scene/
│   ├── Cargo.toml
│   └── src/
│       ├── lib.rs              # 模块声明
│       ├── asset.rs            # AssetId, Handle<T>, AssetManager
│       ├── components.rs       # Transform, MeshRenderer, CameraComponent, LightComponent, Name
│       └── scene.rs            # Scene (world + asset manager + render bridge)
└── engine_editor/
    ├── Cargo.toml
    └── src/
        ├── main.rs             # eframe 入口
        ├── app.rs              # EditorApp: eframe::App 实现
        ├── panels/
        │   ├── mod.rs
        │   ├── viewport.rs     # 渲染视口面板
        │   ├── scene_tree.rs   # 场景树面板 (左侧)
        │   ├── properties.rs   # 属性面板 (右侧)
        │   └── status_bar.rs   # 状态栏 (底部)
        ├── render_bridge.rs    # 渲染线程管理 + triple buffer 通信
        └── camera_control.rs   # 视口内相机控制
```

### 修改文件

```
Cargo.toml                          # workspace members 增加 4 个 crate
simple_renderer/Cargo.toml          # 重命名为 engine_renderer
simple_renderer/ → crates/engine_renderer/  # 目录移动
obj/ → assets/models/               # 资产目录重组
```

### 保留不变

```
crates/engine_renderer/src/*        # 全部源码保留（仅改 Cargo.toml name）
crates/engine_renderer/tests/*      # 223 个测试保留
```

---

## Chunk 1: Workspace 重组 + engine_renderer 就地重命名

### Task 1: 创建 crates 目录结构

**Files:**
- Create: `crates/` directory
- Move: `simple_renderer/` → `crates/engine_renderer/`
- Move: `obj/` → `assets/models/`
- Modify: `Cargo.toml` (workspace root)

- [ ] **Step 1: 创建目录并移动 simple_renderer**

```bash
mkdir -p crates
mv simple_renderer crates/engine_renderer
mkdir -p assets/models
cp -r obj/* assets/models/
# 保留 obj/ 软链接以兼容现有测试
ln -sf assets/models obj
```

- [ ] **Step 2: 重命名 crate**

修改 `crates/engine_renderer/Cargo.toml`：
```toml
[package]
name = "engine_renderer"
```

- [ ] **Step 3: 更新 workspace root Cargo.toml**

```toml
[workspace]
resolver = "2"
members = [
    "crates/engine_renderer",
    "crates/engine_core",
    "crates/engine_scene",
    "crates/engine_editor",
]
```

移除 `system_test` member（暂时保留目录但不编译），移除 `simple_renderer` member。

- [ ] **Step 4: 修复 engine_renderer 内部 crate 引用**

搜索所有 `simple_renderer` 字符串并替换为 `engine_renderer`（lib.rs doc comments, integration tests 等）。

- [ ] **Step 5: 验证编译和测试**

```bash
cargo build -p engine_renderer
cargo test -p engine_renderer
cargo clippy -p engine_renderer -- -D warnings
```

Expected: 全部通过，测试数不变。

- [ ] **Step 6: Commit**

```bash
git add -A
git commit -m "refactor: rename simple_renderer to engine_renderer, reorganize workspace"
```

---

### Task 2: 创建 engine_core 骨架

**Files:**
- Create: `crates/engine_core/Cargo.toml`
- Create: `crates/engine_core/src/lib.rs`
- Create: `crates/engine_core/src/ecs.rs`
- Create: `crates/engine_core/src/time.rs`

- [ ] **Step 1: 创建 Cargo.toml**

```toml
[package]
name = "engine_core"
version = "0.1.0"
edition = "2021"
rust-version = "1.73"

[dependencies]
glam = "0.29"
```

- [ ] **Step 2: 实现最小 ECS — 写测试**

`crates/engine_core/src/ecs.rs` 测试：

```rust
#[cfg(test)]
mod tests {
    use super::*;

    struct Position { x: f32, y: f32 }
    impl Component for Position {}

    struct Velocity { dx: f32, dy: f32 }
    impl Component for Velocity {}

    #[test]
    fn create_entity_and_add_component() {
        let mut world = World::new();
        let e = world.spawn();
        world.insert(e, Position { x: 1.0, y: 2.0 });
        let pos = world.get::<Position>(e).unwrap();
        assert_eq!(pos.x, 1.0);
    }

    #[test]
    fn query_entities_with_component() {
        let mut world = World::new();
        let e1 = world.spawn();
        let e2 = world.spawn();
        world.insert(e1, Position { x: 0.0, y: 0.0 });
        world.insert(e2, Position { x: 1.0, y: 1.0 });
        world.insert(e2, Velocity { dx: 1.0, dy: 0.0 });

        let with_pos: Vec<Entity> = world.query::<Position>().collect();
        assert_eq!(with_pos.len(), 2);

        let with_vel: Vec<Entity> = world.query::<Velocity>().collect();
        assert_eq!(with_vel.len(), 1);
    }

    #[test]
    fn remove_entity() {
        let mut world = World::new();
        let e = world.spawn();
        world.insert(e, Position { x: 0.0, y: 0.0 });
        world.despawn(e);
        assert!(world.get::<Position>(e).is_none());
    }
}
```

- [ ] **Step 3: 运行测试确认失败**

```bash
cargo test -p engine_core
```

Expected: 编译失败（类型不存在）

- [ ] **Step 4: 实现 ECS**

```rust
// crates/engine_core/src/ecs.rs
use std::any::{Any, TypeId};
use std::collections::HashMap;

pub type Entity = u64;

pub trait Component: 'static {}

pub struct World {
    next_entity: Entity,
    alive: Vec<Entity>,
    storages: HashMap<TypeId, HashMap<Entity, Box<dyn Any>>>,
}

impl World {
    pub fn new() -> Self { ... }
    pub fn spawn(&mut self) -> Entity { ... }
    pub fn despawn(&mut self, entity: Entity) { ... }
    pub fn insert<C: Component>(&mut self, entity: Entity, component: C) { ... }
    pub fn get<C: Component>(&self, entity: Entity) -> Option<&C> { ... }
    pub fn get_mut<C: Component>(&mut self, entity: Entity) -> Option<&mut C> { ... }
    pub fn query<C: Component>(&self) -> impl Iterator<Item = Entity> + '_ { ... }
}
```

- [ ] **Step 5: 运行测试确认通过**

```bash
cargo test -p engine_core
```

- [ ] **Step 6: 实现 Time**

```rust
// crates/engine_core/src/time.rs
pub struct Time {
    pub delta: f32,
    pub total: f32,
    pub frame_count: u64,
}
```

- [ ] **Step 7: 写 lib.rs re-exports**

```rust
pub mod ecs;
pub mod time;
pub use ecs::{Component, Entity, World};
pub use time::Time;
```

- [ ] **Step 8: Commit**

```bash
git add crates/engine_core
git commit -m "feat(engine_core): add minimal ECS (World, Entity, Component, query)"
```

---

### Task 3: 创建 engine_scene 骨架

**Files:**
- Create: `crates/engine_scene/Cargo.toml`
- Create: `crates/engine_scene/src/lib.rs`
- Create: `crates/engine_scene/src/asset.rs`
- Create: `crates/engine_scene/src/components.rs`
- Create: `crates/engine_scene/src/scene.rs`

- [ ] **Step 1: Cargo.toml**

```toml
[package]
name = "engine_scene"
version = "0.1.0"
edition = "2021"
rust-version = "1.73"

[dependencies]
engine_core = { path = "../engine_core" }
engine_renderer = { path = "../engine_renderer" }
glam = "0.29"
```

- [ ] **Step 2: 实现资产管理 — 写测试**

```rust
#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn asset_manager_load_and_get() {
        let mut mgr = AssetManager::new();
        let handle = mgr.insert_model(Model::default_for_test());
        assert!(mgr.get_model(handle).is_some());
    }

    #[test]
    fn handle_equality() {
        let mut mgr = AssetManager::new();
        let h1 = mgr.insert_model(Model::default_for_test());
        let h2 = mgr.insert_model(Model::default_for_test());
        assert_ne!(h1.id(), h2.id());
    }
}
```

- [ ] **Step 3: 实现 AssetManager**

```rust
// crates/engine_scene/src/asset.rs
pub type AssetId = u64;

pub struct Handle<T> {
    id: AssetId,
    _marker: std::marker::PhantomData<T>,
}

pub struct AssetManager {
    next_id: AssetId,
    models: HashMap<AssetId, engine_renderer::Model>,
    // textures, materials 后续添加
}
```

- [ ] **Step 4: 实现 Components**

```rust
// crates/engine_scene/src/components.rs
use engine_core::Component;
use glam::{Vec3, Quat};

pub struct Transform {
    pub position: Vec3,
    pub rotation: Quat,
    pub scale: Vec3,
}
impl Component for Transform {}

pub struct Name(pub String);
impl Component for Name {}

pub struct MeshRenderer {
    pub model: Handle<engine_renderer::Model>,
}
impl Component for MeshRenderer {}

// CameraComponent, LightComponent 类似
```

- [ ] **Step 5: 实现 Scene**

```rust
// crates/engine_scene/src/scene.rs
pub struct Scene {
    pub world: World,
    pub assets: AssetManager,
}

impl Scene {
    pub fn new() -> Self { ... }
    pub fn spawn_model(&mut self, path: &str) -> Result<Entity, ...> { ... }
}
```

- [ ] **Step 6: 验证编译**

```bash
cargo build -p engine_scene
cargo test -p engine_scene
```

- [ ] **Step 7: Commit**

```bash
git add crates/engine_scene
git commit -m "feat(engine_scene): add Scene, AssetManager, Transform/MeshRenderer components"
```

---

## Chunk 2: engine_editor — egui 编辑器

### Task 4: 编辑器骨架 + 渲染视口

**Files:**
- Create: `crates/engine_editor/Cargo.toml`
- Create: `crates/engine_editor/src/main.rs`
- Create: `crates/engine_editor/src/app.rs`
- Create: `crates/engine_editor/src/render_bridge.rs`
- Create: `crates/engine_editor/src/panels/mod.rs`
- Create: `crates/engine_editor/src/panels/viewport.rs`

- [ ] **Step 1: Cargo.toml**

```toml
[package]
name = "engine_editor"
version = "0.1.0"
edition = "2021"
rust-version = "1.73"

[dependencies]
engine_core = { path = "../engine_core" }
engine_renderer = { path = "../engine_renderer" }
engine_scene = { path = "../engine_scene" }
eframe = "0.31"
egui = "0.31"
rfd = "0.15"
glam = "0.29"
log = "0.4"
env_logger = "0.11"
```

- [ ] **Step 2: 实现 render_bridge.rs**

启动渲染线程，通过 triple buffer 通信：

```rust
// crates/engine_editor/src/render_bridge.rs
use engine_renderer::{triple_buffer, Shader, SimpleRenderer, Model};
use std::sync::mpsc;
use std::thread;

pub enum RenderCommand {
    SetCamera { view: glam::Mat4, projection: glam::Mat4, pos: glam::Vec3 },
    SetRenderingMode(engine_renderer::RenderingMode),
    SetTileSize(usize),
    SetEarlyZ(bool),
    SetLights(Vec<engine_renderer::Light>),
    Resize { width: usize, height: usize },
    LoadModel(std::path::PathBuf),
    Shutdown,
}

pub struct RenderBridge {
    pub command_tx: mpsc::Sender<RenderCommand>,
    pub reader: triple_buffer::TripleBufferReader,
    pub thread_handle: Option<thread::JoinHandle<()>>,
    pub frame_time_ms: f64,
}

impl RenderBridge {
    pub fn new(width: usize, height: usize, model_path: &str) -> Self { ... }
}
```

渲染线程内部逻辑与现有 `system_test/src/main.rs` 的渲染线程类似，但通过 `mpsc::Receiver<RenderCommand>` 接收参数变更。

- [ ] **Step 3: 实现 main.rs + app.rs**

```rust
// crates/engine_editor/src/main.rs
fn main() -> eframe::Result<()> {
    env_logger::init();
    let options = eframe::NativeOptions {
        viewport: egui::ViewportBuilder::default()
            .with_inner_size([1280.0, 720.0])
            .with_title("SimpleEngine Editor"),
        ..Default::default()
    };
    eframe::run_native("SimpleEngine", options, Box::new(|cc| {
        Ok(Box::new(app::EditorApp::new(cc)))
    }))
}
```

```rust
// crates/engine_editor/src/app.rs
pub struct EditorApp {
    render_bridge: RenderBridge,
    texture_handle: Option<egui::TextureHandle>,
    // ... 编辑器状态
}

impl eframe::App for EditorApp {
    fn update(&mut self, ctx: &egui::Context, _frame: &mut eframe::Frame) {
        // 1. 读取 triple buffer 最新帧
        // 2. 上传为 egui TextureHandle
        // 3. 绘制 UI (menu bar, panels, viewport)
        ctx.request_repaint(); // 持续重绘
    }
}
```

- [ ] **Step 4: 实现 viewport.rs — 渲染输出显示**

```rust
// crates/engine_editor/src/panels/viewport.rs
pub fn show(ui: &mut egui::Ui, texture: &egui::TextureHandle) -> egui::Response {
    let available = ui.available_size();
    let image = egui::Image::new(texture).fit_to_exact_size(available);
    ui.add(image)
}
```

- [ ] **Step 5: 验证编辑器启动并显示渲染输出**

```bash
cargo run -p engine_editor
```

Expected: 窗口打开，显示犹他茶壶的软渲染输出。

- [ ] **Step 6: Commit**

```bash
git add crates/engine_editor
git commit -m "feat(engine_editor): egui editor with render viewport displaying software renderer output"
```

---

### Task 5: 菜单栏 + 文件对话框

**Files:**
- Modify: `crates/engine_editor/src/app.rs`

- [ ] **Step 1: 添加菜单栏**

在 `EditorApp::update` 中添加 `egui::TopBottomPanel::top`：

```rust
egui::TopBottomPanel::top("menu_bar").show(ctx, |ui| {
    egui::menu::bar(ui, |ui| {
        ui.menu_button("文件", |ui| {
            if ui.button("打开模型...").clicked() {
                if let Some(path) = rfd::FileDialog::new()
                    .add_filter("OBJ", &["obj"])
                    .pick_file()
                {
                    self.render_bridge.command_tx
                        .send(RenderCommand::LoadModel(path))
                        .ok();
                }
                ui.close_menu();
            }
            if ui.button("加载贴图...").clicked() {
                // 类似，选择图片文件
                ui.close_menu();
            }
            ui.separator();
            if ui.button("退出").clicked() {
                ctx.send_viewport_cmd(egui::ViewportCommand::Close);
            }
        });
    });
});
```

- [ ] **Step 2: 在渲染线程处理 LoadModel 命令**

```rust
// render_bridge.rs 渲染线程循环中
RenderCommand::LoadModel(path) => {
    match Model::load(path.to_str().unwrap_or_default()) {
        Ok(new_model) => model = new_model,
        Err(e) => log::error!("模型加载失败: {e}"),
    }
}
```

- [ ] **Step 3: 手动验证**

```bash
cargo run -p engine_editor
```

Expected: 菜单栏出现"文件"菜单，点击"打开模型..."弹出系统文件对话框，选 .obj 后视口刷新。

- [ ] **Step 4: Commit**

```bash
git commit -am "feat(engine_editor): add file menu with model/texture loading via rfd"
```

---

### Task 6: 属性面板 — 渲染设置 + 光源 + 材质 + 相机

**Files:**
- Create: `crates/engine_editor/src/panels/properties.rs`
- Modify: `crates/engine_editor/src/app.rs`

- [ ] **Step 1: 定义编辑器状态结构**

```rust
// app.rs
pub struct EditorState {
    pub rendering_mode: RenderingMode,
    pub tile_size: usize,
    pub early_z: bool,
    pub vsync: bool,
    pub triple_buffer_mode: bool,
    pub light_direction: [f32; 3],
    pub light_color: [f32; 3],
    pub shininess: f32,
    pub ambient_strength: f32,
    pub specular_strength: f32,
    pub camera_fov: f32,
    pub camera_near: f32,
    pub camera_far: f32,
    pub camera_speed: f32,
}
```

- [ ] **Step 2: 实现 properties.rs**

```rust
pub fn show(ui: &mut egui::Ui, state: &mut EditorState) -> bool {
    let mut changed = false;

    ui.heading("渲染设置");
    // ComboBox 渲染模式
    // Slider tile_size [16..128]
    // Checkbox early_z
    // Checkbox vsync
    // Radio 缓冲模式

    ui.separator();
    ui.heading("光源");
    // 3x DragValue 方向
    // color_edit_button 颜色

    ui.separator();
    ui.heading("材质");
    // Slider shininess [1..256]
    // Slider ambient [0..1]
    // Slider specular [0..1]

    ui.separator();
    ui.heading("相机");
    // Slider FOV [30..120]
    // DragValue near/far
    // Slider speed

    changed
}
```

- [ ] **Step 3: 集成到 app.rs 布局**

```rust
// app.rs update()
egui::SidePanel::right("properties").show(ctx, |ui| {
    if panels::properties::show(ui, &mut self.state) {
        self.send_state_to_renderer();
    }
});
```

- [ ] **Step 4: 实现 send_state_to_renderer()**

将 EditorState 中变更的字段通过 `command_tx` 发送给渲染线程。

- [ ] **Step 5: 手动验证所有 slider/参数生效**

```bash
cargo run -p engine_editor
```

Expected: 右侧面板显示所有参数，拖动 slider 后渲染视口实时更新。

- [ ] **Step 6: Commit**

```bash
git commit -am "feat(engine_editor): add properties panel with rendering/light/material/camera controls"
```

---

### Task 7: 场景树 + 状态栏

**Files:**
- Create: `crates/engine_editor/src/panels/scene_tree.rs`
- Create: `crates/engine_editor/src/panels/status_bar.rs`
- Modify: `crates/engine_editor/src/app.rs`

- [ ] **Step 1: 实现 scene_tree.rs**

```rust
pub fn show(ui: &mut egui::Ui, entities: &[EntityInfo], selected: &mut Option<usize>) {
    ui.heading("场景");
    for (i, entity) in entities.iter().enumerate() {
        let label = egui::SelectableLabel::new(
            *selected == Some(i),
            &entity.name,
        );
        if ui.add(label).clicked() {
            *selected = Some(i);
        }
    }
}
```

- [ ] **Step 2: 实现 status_bar.rs**

```rust
pub fn show(ui: &mut egui::Ui, fps: f64, render_time_ms: f64, entity_count: usize, mode: &str) {
    ui.horizontal(|ui| {
        ui.label(format!("FPS: {fps:.0}"));
        ui.separator();
        ui.label(format!("渲染: {render_time_ms:.1}ms"));
        ui.separator();
        ui.label(format!("实体: {entity_count}"));
        ui.separator();
        ui.label(format!("模式: {mode}"));
    });
}
```

- [ ] **Step 3: 集成到 app.rs**

```rust
egui::SidePanel::left("scene_tree").show(ctx, |ui| {
    panels::scene_tree::show(ui, &self.entities, &mut self.selected_entity);
});

egui::TopBottomPanel::bottom("status").show(ctx, |ui| {
    panels::status_bar::show(ui, self.fps, self.render_time_ms, self.entities.len(), &mode_name);
});
```

- [ ] **Step 4: 手动验证**

```bash
cargo run -p engine_editor
```

Expected: 左侧场景树显示 Entity 列表，底部状态栏显示 FPS 和渲染耗时。

- [ ] **Step 5: Commit**

```bash
git commit -am "feat(engine_editor): add scene tree panel and status bar"
```

---

### Task 8: 视口相机控制

**Files:**
- Create: `crates/engine_editor/src/camera_control.rs`
- Modify: `crates/engine_editor/src/panels/viewport.rs`

- [ ] **Step 1: 实现 camera_control.rs**

从 `system_test/src/camera.rs` 迁移核心逻辑：

```rust
pub struct EditorCamera {
    pub position: Vec3,
    pub yaw: f32,
    pub pitch: f32,
    pub fov: f32,
    pub near: f32,
    pub far: f32,
    pub speed: f32,
}

impl EditorCamera {
    pub fn process_input(&mut self, response: &egui::Response, ctx: &egui::Context) {
        // 右键拖拽 → 旋转 (response.dragged_by(PointerButton::Secondary))
        // WASD → 移动 (ctx.input().key_down)
        // 滚轮 → 调速
    }

    pub fn view_matrix(&self) -> Mat4 { ... }
    pub fn projection_matrix(&self, aspect: f32) -> Mat4 { ... }
}
```

- [ ] **Step 2: 在 viewport.rs 中集成相机控制**

```rust
pub fn show(ui: &mut egui::Ui, texture: &egui::TextureHandle, camera: &mut EditorCamera) -> egui::Response {
    let available = ui.available_size();
    let response = ui.add(egui::Image::new(texture).fit_to_exact_size(available).sense(egui::Sense::click_and_drag()));
    camera.process_input(&response, ui.ctx());
    response
}
```

- [ ] **Step 3: 每帧发送相机状态到渲染线程**

```rust
// app.rs update() 中
self.render_bridge.command_tx.send(RenderCommand::SetCamera {
    view: self.camera.view_matrix(),
    projection: self.camera.projection_matrix(aspect),
    pos: self.camera.position,
}).ok();
```

- [ ] **Step 4: 手动验证相机控制**

```bash
cargo run -p engine_editor
```

Expected: 在视口内右键拖拽旋转，WASD 移动，滚轮调速。

- [ ] **Step 5: Commit**

```bash
git commit -am "feat(engine_editor): add viewport camera control (FPS-style, mouse + keyboard)"
```

---

## Chunk 3: 收尾 + 验证

### Task 9: 清理 system_test + 更新文档

**Files:**
- Remove: `system_test/` (从 workspace 移除，可保留目录作为参考)
- Modify: `Cargo.toml`
- Modify: `README.md`, `README-cn.md`
- Modify: `AGENTS.md`

- [ ] **Step 1: 从 workspace 移除 system_test**

确认 `Cargo.toml` workspace members 不包含 system_test。

- [ ] **Step 2: 更新 AGENTS.md**

更新项目结构、代码映射、命令表等。

- [ ] **Step 3: 更新 README-cn.md**

更新代码结构表、运行命令（`cargo run -p engine_editor`）、依赖表等。

- [ ] **Step 4: Commit**

```bash
git commit -am "docs: update AGENTS.md and README for engine workspace structure"
```

---

### Task 10: 全量验证

- [ ] **Step 1: 编译全部 crate**

```bash
cargo build --workspace
```

Expected: 全部编译通过。

- [ ] **Step 2: 运行全部测试**

```bash
cargo test --workspace
```

Expected: engine_renderer 原有测试全部通过 + engine_core 新测试通过。

- [ ] **Step 3: Clippy 检查**

```bash
cargo clippy --workspace -- -D warnings
```

Expected: 零警告。

- [ ] **Step 4: 手动 QA — 编辑器功能验证**

```bash
cargo run -p engine_editor
```

验证清单：
- [ ] 窗口启动，默认显示犹他茶壶
- [ ] 菜单: 文件 → 打开模型 → 选择 .obj → 模型替换
- [ ] 菜单: 文件 → 加载贴图 → 选择图片 → 贴图应用
- [ ] 属性面板: 渲染模式切换（4 种）生效
- [ ] 属性面板: 光源方向/颜色拖拽 → 渲染实时更新
- [ ] 属性面板: 材质 shininess slider → 高光变化
- [ ] 属性面板: 相机 FOV slider → 视角变化
- [ ] 属性面板: Tile 大小/Early-Z → 渲染变化
- [ ] 状态栏: FPS 和渲染耗时实时更新
- [ ] 视口: 右键拖拽旋转，WASD 移动
- [ ] 场景树: 显示 Entity，点击选中

- [ ] **Step 5: 最终 Commit**

```bash
git commit -am "feat: SimpleEngine editor prototype complete"
```
