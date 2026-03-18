# Editor Interaction Design Implementation Plan

> **For agentic workers:** REQUIRED: Use superpowers:subagent-driven-development (if subagents available) or superpowers:executing-plans to implement this plan. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Implement the full editor interaction redesign from `docs/specs/editor-interaction-design.md`, transforming the editor from a single-model viewer into a multi-entity scene editor with Unity/Unreal-style interactions across 4 phases.

**Architecture:** The editor (`engine_editor`) uses eframe/egui 0.31 with a dedicated render thread communicating via mpsc channels and lock-free triple buffer. The ECS uses hecs with Transform/Parent/Children hierarchy. The current editor uses `Vec<String>` for entities — this plan first migrates to real `engine_scene::Scene` (hecs::World), then builds all interaction systems on top.

**Tech Stack:** Rust, hecs 0.10, glam 0.29, eframe/egui 0.31, egui-gizmo 0.18, engine_scene, engine_core, engine_math

**Spec:** `docs/specs/editor-interaction-design.md` + `docs/specs/2026-03-18-scene-editing-design.md`

---

## File Structure

### New Files

```
crates/engine_editor/src/
├── selection.rs          # SelectionState + ray picking logic
├── commands.rs           # EditorCommand trait + CommandHistory + concrete commands
├── gizmo.rs              # GizmoState + egui-gizmo integration helpers
├── panels/
│   ├── toolbar.rs        # Transform tools (Q/W/E/R), coord system, play controls
│   ├── toast.rs          # Toast notification system
│   └── shortcuts_help.rs # F1 shortcut reference window
crates/engine_scene/src/
└── components.rs         # Add BoundingBox component
```

### Modified Files

```
crates/engine_editor/Cargo.toml              # Add egui-gizmo, engine_math, engine_input deps
crates/engine_editor/src/app.rs              # MAJOR: Scene integration, shortcuts, toolbar, state overhaul
crates/engine_editor/src/camera_control.rs   # Add Orbit/Pan/Dolly/Focus modes, animation
crates/engine_editor/src/render_bridge.rs    # Multi-entity rendering (RegisterModel + SubmitFrame)
crates/engine_editor/src/panels/mod.rs       # Add new module declarations
crates/engine_editor/src/panels/scene_tree.rs  # ECS-based, hierarchy, drag, right-click, search
crates/engine_editor/src/panels/properties.rs  # Per-entity component editing with DragValue
crates/engine_editor/src/panels/viewport.rs    # Gizmo overlay, ray picking, grid, axis indicator, info overlay
crates/engine_editor/src/panels/status_bar.rs  # Selection info, coordinates, undo preview
crates/engine_editor/src/panels/console.rs     # Search, filter, copy
crates/engine_scene/src/components.rs          # Add BoundingBox component
```

---

## Chunk 1: ECS Integration + Multi-Entity Foundation

**Goal:** Replace `Vec<String>` scene entities with real `engine_scene::Scene` (hecs::World). Enable multi-entity rendering. This is the prerequisite for ALL interaction features.

### Task 1: Add Dependencies + New Module Stubs

**Files:**
- Modify: `crates/engine_editor/Cargo.toml`
- Create: `crates/engine_editor/src/selection.rs`
- Create: `crates/engine_editor/src/commands.rs`
- Create: `crates/engine_editor/src/gizmo.rs`
- Create: `crates/engine_editor/src/panels/toolbar.rs`
- Modify: `crates/engine_editor/src/panels/mod.rs`

- [ ] **Step 1: Update Cargo.toml — add egui-gizmo + engine_math + engine_input**

Add to `[dependencies]`:
```toml
egui-gizmo = "0.18"
engine_math = { path = "../engine_math" }
engine_input = { path = "../engine_input" }
```

- [ ] **Step 2: Create selection.rs — SelectionState struct**

```rust
// crates/engine_editor/src/selection.rs
use engine_core::Entity;

/// 编辑器选择状态
#[derive(Debug, Default)]
pub struct SelectionState {
    /// 当前选中的实体
    pub selected: Option<Entity>,
    /// 上一次选中的实体
    pub previous: Option<Entity>,
}

impl SelectionState {
    pub fn select(&mut self, entity: Entity) {
        self.previous = self.selected;
        self.selected = Some(entity);
    }

    pub fn deselect(&mut self) {
        self.previous = self.selected;
        self.selected = None;
    }

    pub fn is_selected(&self, entity: Entity) -> bool {
        self.selected == Some(entity)
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use engine_core::World;

    #[test]
    fn select_and_deselect() {
        let mut world = World::new();
        let e = world.spawn(());
        let mut sel = SelectionState::default();
        assert!(sel.selected.is_none());

        sel.select(e);
        assert!(sel.is_selected(e));

        sel.deselect();
        assert!(sel.selected.is_none());
        assert_eq!(sel.previous, Some(e));
    }
}
```

- [ ] **Step 3: Create gizmo.rs — GizmoMode + GizmoState**

```rust
// crates/engine_editor/src/gizmo.rs

/// 当前 Gizmo 工具模式
#[derive(Debug, Clone, Copy, PartialEq, Eq, Default)]
pub enum GizmoTool {
    #[default]
    Select,
    Translate,
    Rotate,
    Scale,
}

/// 坐标系
#[derive(Debug, Clone, Copy, PartialEq, Eq, Default)]
pub enum CoordinateSpace {
    #[default]
    Local,
    World,
}

/// Gizmo 状态
#[derive(Debug, Default)]
pub struct GizmoState {
    pub tool: GizmoTool,
    pub space: CoordinateSpace,
}

impl GizmoState {
    pub fn toggle_space(&mut self) {
        self.space = match self.space {
            CoordinateSpace::Local => CoordinateSpace::World,
            CoordinateSpace::World => CoordinateSpace::Local,
        };
    }
}
```

- [ ] **Step 4: Create commands.rs — EditorCommand trait + CommandHistory skeleton**

```rust
// crates/engine_editor/src/commands.rs
use engine_scene::Scene;

/// 可撤销的编辑器命令
pub trait EditorCommand: Send {
    fn execute(&mut self, scene: &mut Scene);
    fn undo(&mut self, scene: &mut Scene);
    fn description(&self) -> &str;
}

/// 命令历史栈
pub struct CommandHistory {
    undo_stack: Vec<Box<dyn EditorCommand>>,
    redo_stack: Vec<Box<dyn EditorCommand>>,
    max_history: usize,
}

impl Default for CommandHistory {
    fn default() -> Self {
        Self {
            undo_stack: Vec::new(),
            redo_stack: Vec::new(),
            max_history: 100,
        }
    }
}

impl CommandHistory {
    pub fn execute(&mut self, mut cmd: Box<dyn EditorCommand>, scene: &mut Scene) {
        cmd.execute(scene);
        self.undo_stack.push(cmd);
        self.redo_stack.clear();
        if self.undo_stack.len() > self.max_history {
            self.undo_stack.remove(0);
        }
    }

    pub fn undo(&mut self, scene: &mut Scene) {
        if let Some(mut cmd) = self.undo_stack.pop() {
            cmd.undo(scene);
            self.redo_stack.push(cmd);
        }
    }

    pub fn redo(&mut self, scene: &mut Scene) {
        if let Some(mut cmd) = self.redo_stack.pop() {
            cmd.execute(scene);
            self.undo_stack.push(cmd);
        }
    }

    pub fn can_undo(&self) -> bool {
        !self.undo_stack.is_empty()
    }

    pub fn can_redo(&self) -> bool {
        !self.redo_stack.is_empty()
    }

    pub fn undo_description(&self) -> Option<&str> {
        self.undo_stack.last().map(|c| c.description())
    }

    pub fn redo_description(&self) -> Option<&str> {
        self.redo_stack.last().map(|c| c.description())
    }
}
```

- [ ] **Step 5: Create toolbar.rs stub**

```rust
// crates/engine_editor/src/panels/toolbar.rs
use crate::gizmo::{GizmoTool, GizmoState, CoordinateSpace};

/// 绘制工具栏
pub fn show(ui: &mut egui::Ui, gizmo: &mut GizmoState) {
    ui.horizontal(|ui| {
        // 变换工具组
        let tools = [
            (GizmoTool::Select, "🔲", "选择 (Q)"),
            (GizmoTool::Translate, "↔", "移动 (W)"),
            (GizmoTool::Rotate, "🔄", "旋转 (E)"),
            (GizmoTool::Scale, "📐", "缩放 (R)"),
        ];

        for (tool, icon, tooltip) in &tools {
            let selected = gizmo.tool == *tool;
            if ui.selectable_label(selected, *icon).on_hover_text(*tooltip).clicked() {
                gizmo.tool = *tool;
            }
        }

        ui.separator();

        // 坐标系切换
        let space_label = match gizmo.space {
            CoordinateSpace::Local => "Local",
            CoordinateSpace::World => "World",
        };
        egui::ComboBox::from_id_salt("coord_space")
            .selected_text(space_label)
            .show_ui(ui, |ui| {
                ui.selectable_value(&mut gizmo.space, CoordinateSpace::Local, "Local");
                ui.selectable_value(&mut gizmo.space, CoordinateSpace::World, "World");
            });
    });
}
```

- [ ] **Step 6: Update panels/mod.rs — add new modules**

Add: `pub mod toolbar;` (and later toast, shortcuts_help).

- [ ] **Step 7: Verify build**

```bash
cargo build -p engine_editor
cargo test -p engine_editor
cargo clippy -p engine_editor -- -D warnings
```

- [ ] **Step 8: Commit**

---

### Task 2: Refactor EditorApp — Scene Integration

**Files:**
- Modify: `crates/engine_editor/src/app.rs`
- Modify: `crates/engine_editor/src/render_bridge.rs`
- Modify: `crates/engine_editor/src/panels/scene_tree.rs`

This is the CORE refactoring. Replace `Vec<String>` entity list with `engine_scene::Scene`, add `SelectionState`, `GizmoState`, `CommandHistory` to EditorApp. Update the render bridge to support multi-entity rendering. Update scene tree to query hecs::World.

- [ ] **Step 1: Add RenderCommand variants for multi-entity**

In `render_bridge.rs`, add:
```rust
pub enum RenderCommand {
    // ... existing variants ...
    RegisterModel { id: u64, model: Model },
    UnregisterModel { id: u64 },
    SubmitFrame(Vec<DrawCallData>),
}

pub struct DrawCallData {
    pub model_id: u64,
    pub model_matrix: glam::Mat4,
    pub entity_id: u32,
}
```

Update `render_loop` to handle `SubmitFrame` — iterate `Vec<DrawCallData>`, look up model by id from an internal `HashMap<u64, Model>`, call `renderer.draw_model()` for each.

- [ ] **Step 2: Refactor EditorApp struct**

Replace:
```rust
scene_entities: Vec<String>,
selected_entity: Option<usize>,
model_loaded: bool,
```

With:
```rust
scene: engine_scene::Scene,
selection: crate::selection::SelectionState,
gizmo: crate::gizmo::GizmoState,
commands: crate::commands::CommandHistory,
```

Update `EditorApp::new()`:
- Create `Scene::new()`
- Spawn default entities (teapot + light + camera) using `scene.spawn_model()` / `scene.spawn_light()` / `scene.spawn_camera()`
- Register loaded models with the render bridge via `RegisterModel`
- Each frame: collect `DrawCallData` from world query, send `SubmitFrame`

- [ ] **Step 3: Update scene_tree.rs to query hecs::World**

Change `show()` signature to accept `&Scene` + `&mut SelectionState` instead of `&Vec<String>` + `&mut Option<usize>`.
Query `scene.world` for all entities with `Name` component. Display using `selectable_label`. Update `selection.select(entity)` on click.

- [ ] **Step 4: Update app.rs update() loop**

- Show toolbar panel (top, below menu bar)
- Pass `&scene` + `&mut selection` to scene_tree
- Pass `&mut scene` + `&selection` to properties
- Each frame: collect draw calls from scene, send to render bridge
- Handle gizmo tool shortcuts (Q/W/E/R)

- [ ] **Step 5: Verify build + test**

```bash
cargo build -p engine_editor
cargo test --workspace
cargo clippy --workspace -- -D warnings
```

- [ ] **Step 6: Commit**

---

## Chunk 2: Multi-Mode Camera + Selection Linkage

### Task 3: Multi-Mode Camera

**Files:**
- Modify: `crates/engine_editor/src/camera_control.rs`

Extend EditorCamera with Orbit/Pan/Dolly/Focus modes per spec Section 5.

- [ ] **Step 1: Add CameraMode enum + orbit fields**

```rust
#[derive(Debug, Clone, Copy, PartialEq, Eq, Default)]
pub enum CameraMode {
    #[default]
    Idle,
    Fly,
    Orbit,
    Pan,
    Dolly,
}
```

Add to EditorCamera:
```rust
pub mode: CameraMode,
pub orbit_pivot: Vec3,
fly_speed: f32,  // adjustable 0.5–50.0
```

- [ ] **Step 2: Implement orbit()**

```rust
pub fn orbit(&mut self, dx: f32, dy: f32) {
    let radius = (self.position - self.orbit_pivot).length();
    self.yaw += dx * self.mouse_sensitivity;
    self.pitch += dy * self.mouse_sensitivity;
    self.pitch = self.pitch.clamp(-89.0, 89.0);
    self.update_vectors();
    self.position = self.orbit_pivot - self.front * radius;
}
```

- [ ] **Step 3: Implement pan() and dolly()**

```rust
pub fn pan(&mut self, dx: f32, dy: f32) {
    let speed = 0.01;
    self.position += self.right * (-dx * speed) + self.up * (dy * speed);
    self.orbit_pivot += self.right * (-dx * speed) + self.up * (dy * speed);
}

pub fn dolly(&mut self, delta: f32) {
    let movement = self.front * delta * 0.05;
    self.position += movement;
}
```

- [ ] **Step 4: Implement focus_on()**

```rust
pub fn focus_on(&mut self, target: Vec3, size: f32) {
    let distance = (size * 2.5).max(1.0);
    let direction = (self.position - target).normalize_or_else(|| Vec3::new(0.0, 0.0, 1.0));
    self.position = target + direction * distance;
    self.look_at(target);
    self.orbit_pivot = target;
}

fn look_at(&mut self, target: Vec3) {
    let dir = (target - self.position).normalize();
    self.pitch = dir.y.asin().to_degrees();
    self.yaw = dir.z.atan2(dir.x).to_degrees();
    self.update_vectors();
}
```

- [ ] **Step 5: Update handle_camera_input() in app.rs**

Add mode detection:
- Right-click held → Fly mode (existing)
- Alt + Left-click drag → Orbit mode
- Alt + Middle-click drag → Pan mode
- Alt + Right-click drag → Dolly mode
- Scroll wheel → Zoom (dolly)
- F key → Focus on selected entity

- [ ] **Step 6: Verify build + test**

```bash
cargo build -p engine_editor
cargo clippy -p engine_editor -- -D warnings
```

- [ ] **Step 7: Commit**

---

### Task 4: Per-Entity Properties Panel

**Files:**
- Modify: `crates/engine_editor/src/panels/properties.rs`

Rewrite properties panel to edit selected entity's components (Transform, Name, MeshRenderer, LightComponent, CameraComponent). When no entity selected, show global render/environment settings.

- [ ] **Step 1: Change show() signature**

```rust
pub fn show(
    ui: &mut egui::Ui,
    scene: &mut engine_scene::Scene,
    selection: &crate::selection::SelectionState,
    state: &mut EditorState,
)
```

- [ ] **Step 2: Implement per-entity editing**

If `selection.selected` is Some:
- Query entity for Transform → DragValue for translation.x/y/z, rotation (euler), scale
- Query for Name → TextEdit
- Query for MeshRenderer → display model handle
- Query for LightComponent → direction DragValue + color picker + intensity slider
- Query for CameraComponent → fov/near/far sliders

Each section in a CollapsingHeader.

If `selection.selected` is None:
- Show global render settings (rendering mode, tile size, Early-Z)
- Show environment settings (sky color, ambient intensity)
- Show editor camera settings (speed, fov, sensitivity)

- [ ] **Step 3: Verify build**
- [ ] **Step 4: Commit**

---

### Task 5: Shortcut Key System

**Files:**
- Modify: `crates/engine_editor/src/app.rs`

Implement global + viewport shortcut dispatching per spec Section 11.

- [ ] **Step 1: Add handle_shortcuts() method to EditorApp**

Process egui input events for:
- Q/W/E/R → GizmoTool switch
- ~ → Toggle coordinate space
- G → Toggle grid visibility
- F → Focus on selected entity
- Delete/Backspace → Delete selected entity
- Ctrl+Z → Undo
- Ctrl+Shift+Z → Redo
- Ctrl+D → Duplicate entity
- Escape → Deselect
- Ctrl+S → Save project

- [ ] **Step 2: Add shortcut annotations to menu items**

Update menu_bar to show shortcut hints (e.g., "保存项目  Ctrl+S").

- [ ] **Step 3: Verify no shortcut conflicts with egui defaults**
- [ ] **Step 4: Commit**

---

## Chunk 3: Viewport Enhancement (Phase 2)

### Task 6: BoundingBox Component + Ray Picking

**Files:**
- Modify: `crates/engine_scene/src/components.rs`
- Modify: `crates/engine_editor/src/selection.rs`
- Modify: `crates/engine_editor/src/panels/viewport.rs`

- [ ] **Step 1: Add BoundingBox component**

```rust
// engine_scene/src/components.rs
use engine_math::AABB;

#[derive(Debug, Clone)]
pub struct BoundingBox(pub AABB);
```

Update `Scene::spawn_model()` to attach a default BoundingBox after model load.

- [ ] **Step 2: Implement screen-to-ray conversion in selection.rs**

```rust
pub fn screen_to_ray(
    screen_pos: egui::Pos2,
    viewport_rect: egui::Rect,
    view: Mat4,
    projection: Mat4,
    camera_pos: Vec3,
) -> engine_math::Ray {
    let nx = ((screen_pos.x - viewport_rect.left()) / viewport_rect.width()) * 2.0 - 1.0;
    let ny = 1.0 - ((screen_pos.y - viewport_rect.top()) / viewport_rect.height()) * 2.0;
    let inv_vp = (projection * view).inverse();
    let near = inv_vp.project_point3(Vec3::new(nx, ny, -1.0));
    let far = inv_vp.project_point3(Vec3::new(nx, ny, 1.0));
    engine_math::Ray::new(camera_pos, (far - near).normalize())
}
```

- [ ] **Step 3: Implement pick_entity()**

```rust
pub fn pick_entity(
    ray: &engine_math::Ray,
    world: &hecs::World,
) -> Option<(Entity, f32)> {
    let mut closest: Option<(Entity, f32)> = None;
    for (entity, (transform, bbox)) in world.query::<(&Transform, &BoundingBox)>().iter() {
        // Transform AABB to world space (simplified: offset by translation)
        let world_aabb = AABB::new(
            bbox.0.min + transform.translation,
            bbox.0.max + transform.translation,
        );
        if let Some(t) = ray.intersects_aabb(&world_aabb) {
            if closest.is_none() || t < closest.unwrap().1 {
                closest = Some((entity, t));
            }
        }
    }
    closest
}
```

- [ ] **Step 4: Wire viewport click → pick_entity → selection.select()**

In viewport.rs, when left-click on viewport (not on gizmo):
1. Convert click position to ray
2. Call pick_entity
3. Update SelectionState

- [ ] **Step 5: Verify + Commit**

---

### Task 7: Gizmo Integration

**Files:**
- Modify: `crates/engine_editor/src/panels/viewport.rs`
- Modify: `crates/engine_editor/src/gizmo.rs`

- [ ] **Step 1: Map GizmoTool to egui_gizmo::GizmoMode**

```rust
impl GizmoTool {
    pub fn to_egui_mode(&self) -> Option<egui_gizmo::GizmoMode> {
        match self {
            GizmoTool::Select => None,
            GizmoTool::Translate => Some(egui_gizmo::GizmoMode::Translate),
            GizmoTool::Rotate => Some(egui_gizmo::GizmoMode::Rotate),
            GizmoTool::Scale => Some(egui_gizmo::GizmoMode::Scale),
        }
    }
}
```

- [ ] **Step 2: Render gizmo in viewport**

After rendering the 3D image, if there's a selected entity and tool != Select:
```rust
let gizmo = egui_gizmo::Gizmo::new("transform_gizmo")
    .view_matrix(view.to_cols_array_2d())
    .projection_matrix(proj.to_cols_array_2d())
    .model_matrix(model.to_cols_array_2d())
    .mode(mode)
    .orientation(orientation)
    .viewport(viewport_rect);

if let Some(response) = gizmo.interact(ui) {
    // Decompose response.transform() back to Transform
    // Submit TransformCommand to CommandHistory
}
```

- [ ] **Step 3: Verify gizmo renders on top of viewport image**
- [ ] **Step 4: Commit**

---

### Task 8: Grid + Axis Indicator + Viewport Info Overlay

**Files:**
- Modify: `crates/engine_editor/src/panels/viewport.rs`

- [ ] **Step 1: Draw axis indicator (egui Painter)**

In viewport bottom-right corner, draw XYZ axis indicator using `ui.painter()`:
- X axis: Red line
- Y axis: Green line
- Z axis: Blue line
- Rotate based on camera view matrix
- Fixed 60x60 pixel area

- [ ] **Step 2: Draw viewport info overlay**

Top-left of viewport, semi-transparent:
- Current tool name
- Selected entity name
- Selected entity position

- [ ] **Step 3: Add grid visibility toggle state**

Add `show_grid: bool` to EditorApp, toggle with G key.

- [ ] **Step 4: Commit**

---

## Chunk 4: Edit Safety + Scene Management (Phase 3)

### Task 9: Undo/Redo — Concrete Commands

**Files:**
- Modify: `crates/engine_editor/src/commands.rs`
- Modify: `crates/engine_editor/src/app.rs`

- [ ] **Step 1: Implement TransformCommand**

```rust
pub struct TransformCommand {
    entity: Entity,
    old_transform: Transform,
    new_transform: Transform,
}
```

- [ ] **Step 2: Implement SpawnCommand + DespawnCommand**
- [ ] **Step 3: Implement RenameCommand**
- [ ] **Step 4: Wire Ctrl+Z / Ctrl+Shift+Z in app.rs**
- [ ] **Step 5: Verify undo/redo works**
- [ ] **Step 6: Commit**

---

### Task 10: Scene Tree Hierarchy + Drag

**Files:**
- Modify: `crates/engine_editor/src/panels/scene_tree.rs`

- [ ] **Step 1: Recursive tree rendering**

Query entities without Parent → render as roots. For each root, query Children and render recursively with indentation. Add ▼/▶ expand/collapse toggle.

- [ ] **Step 2: Inline rename (double-click)**

On double-click entity name → switch to TextEdit mode. On Enter/Escape → commit/cancel.

- [ ] **Step 3: Visibility toggle (eye icon)**

Add 👁 button per entity. Toggle a `Visible` component.

- [ ] **Step 4: Search/filter bar**

Top of scene tree: search input. Filter entities by name (case-insensitive contains).

- [ ] **Step 5: Commit**

---

### Task 11: Right-Click Context Menus + Entity CRUD

**Files:**
- Modify: `crates/engine_editor/src/panels/scene_tree.rs`
- Modify: `crates/engine_editor/src/panels/viewport.rs`
- Modify: `crates/engine_editor/src/app.rs`

- [ ] **Step 1: Scene tree right-click menu**

On right-click entity in scene tree, show context menu:
- Add empty entity
- Add model entity... (file dialog)
- Add light → submenu (Directional/Point/Spot)
- Add camera
- ---
- Duplicate (Ctrl+D)
- Delete (Delete)
- ---
- Rename (F2)
- Focus (F)

Each action creates appropriate EditorCommand and executes via CommandHistory.

- [ ] **Step 2: Bottom "Add Entity" button**

Below scene tree list, add [+ 添加实体] button with same submenu.

- [ ] **Step 3: Viewport right-click menu** (simplified — Focus, Select All, Deselect)
- [ ] **Step 4: Commit**

---

### Task 12: Toast Notification System

**Files:**
- Create: `crates/engine_editor/src/panels/toast.rs`
- Modify: `crates/engine_editor/src/app.rs`

- [ ] **Step 1: Create Toast struct + ToastManager**

```rust
pub enum ToastLevel { Success, Error, Info }

pub struct Toast {
    pub message: String,
    pub level: ToastLevel,
    pub created: Instant,
    pub duration: Duration,
}

pub struct ToastManager {
    toasts: Vec<Toast>,
}
```

- [ ] **Step 2: Render toasts in viewport bottom-right**

Draw toasts as small floating panels. Remove after duration expires.
- Success: green background, 3s
- Error: red background, 5s
- Info: gray background, 3s

- [ ] **Step 3: Wire toast calls for key operations**

- Model load success/failure
- Project save
- Entity delete

- [ ] **Step 4: Commit**

---

## Chunk 5: Polish & Complete (Phase 4)

### Task 13: Menu System Completion

**Files:**
- Modify: `crates/engine_editor/src/app.rs`

- [ ] **Step 1: Complete File menu** (New/Open/Save/SaveAs/Import/Recent/Exit with shortcuts)
- [ ] **Step 2: Add Edit menu** (Undo/Redo/Duplicate/Delete/SelectAll/Deselect)
- [ ] **Step 3: Expand View menu** (Reset Camera/Focus/View presets/Panel toggles/Grid/Wireframe)
- [ ] **Step 4: Add Entity menu** (Add empty/model/light/camera, Rename/Duplicate/Delete)
- [ ] **Step 5: Add Help menu** (Shortcut reference F1, About, GitHub)
- [ ] **Step 6: Commit**

---

### Task 14: Status Bar Enhancement

**Files:**
- Modify: `crates/engine_editor/src/panels/status_bar.rs`

- [ ] **Step 1: Update show() to accept selection + command info**
- [ ] **Step 2: Display: project name*, FPS, render time, mode, selected entity name, position**
- [ ] **Step 3: Commit**

---

### Task 15: Console Enhancement

**Files:**
- Modify: `crates/engine_editor/src/panels/console.rs`

- [ ] **Step 1: Add search/filter bar at top**
- [ ] **Step 2: Add right-click context menu (copy line, copy all, clear)**
- [ ] **Step 3: Commit**

---

### Task 16: Layout Persistence

**Files:**
- Modify: `crates/engine_editor/src/project.rs`
- Modify: `crates/engine_editor/src/app.rs`

- [ ] **Step 1: Add [editor.layout] section to project TOML**

```toml
[editor.layout]
scene_tree_width = 180
properties_width = 260
console_height = 160
console_visible = true
```

- [ ] **Step 2: Save/load layout on project save/open**
- [ ] **Step 3: Commit**

---

### Task 17: Shortcut Reference Window (F1)

**Files:**
- Create: `crates/engine_editor/src/panels/shortcuts_help.rs`

- [ ] **Step 1: Create modal window listing all shortcuts from spec Section 11**

Organized by category: Global, Viewport, Camera, Panel.
Displayed as a table.

- [ ] **Step 2: Wire F1 to toggle window**
- [ ] **Step 3: Commit**

---

## Verification

After all tasks complete:

```bash
cargo build --workspace
cargo test --workspace
cargo clippy --workspace -- -D warnings
cargo run -p engine_editor  # Manual QA
```

**Manual QA checklist:**
1. Load model → scene tree shows entity
2. Click entity in scene tree → properties shows Transform
3. Edit Transform values → model moves in viewport
4. Q/W/E/R switch tools → toolbar updates
5. Alt+left-click drag → orbit camera
6. F key → focus on selected
7. Ctrl+Z → undo
8. Right-click scene tree → context menu works
9. F1 → shortcut help window
