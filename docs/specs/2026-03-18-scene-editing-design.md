# UE 风格场景编辑系统设计方案

**日期**: 2026-03-18
**状态**: 待审阅
**范围**: 类 Unreal Engine 场景编辑 + 自由相机

---

## 1. 目标

将当前编辑器从「单模型查看器」升级为「多实体场景编辑器」，实现类似 Unreal Engine 的核心编辑体验：

- **多实体场景**：支持在场景中放置多个模型、灯光、相机
- **实体选择**：点击视口中的物体进行选择
- **Gizmo 操控**：平移 / 旋转 / 缩放 gizmo，可视化拖拽编辑 Transform
- **自由相机**：右键+WASD 飞行、Alt+左键 环绕、滚轮缩放、F 聚焦选中物体
- **Transform 层级**：父子关系，Local→Global 传播
- **逐实体属性编辑**：选中实体后在属性面板编辑其组件
- **场景树操作**：添加/删除/复制实体、拖拽改变层级
- **撤销/重做**：基于 Command 模式的操作历史

---

## 2. 现状分析

### 2.1 当前架构缺口

| 模块 | 现状 | 目标状态 |
|------|------|----------|
| 场景数据 | `Vec<String>` 实体名称列表，未使用 `engine_scene::Scene` | 基于 `hecs::World` 的真正 ECS 场景 |
| 渲染 | 渲染线程内 `Option<Model>`，只能画一个模型 | 迭代 World 中所有 `MeshRenderer` 实体 |
| 模型矩阵 | 硬编码茶壶变换 | 每个实体独立的 `Transform` → 模型矩阵 |
| 属性面板 | 编辑全局 `EditorState` | 编辑选中实体的 ECS 组件 |
| 选择系统 | 仅场景树点选 | 视口点击选择 + 场景树选择 |
| 相机 | FPS 飞行（右键+WASD） | UE 多模式相机 |

### 2.2 可复用的基础设施

| 已有 | 位置 | 可复用性 |
|------|------|----------|
| `Transform` + `Parent` + `Children` + `GlobalTransform` | `engine_core/src/transform.rs` | ✅ 直接使用，需添加传播系统 |
| `Scene` + `AssetManager` + `spawn_model/light/camera` | `engine_scene/src/` | ✅ 替换当前 `Vec<String>` |
| `Ray::intersects_aabb` | `engine_math/src/ray.rs` | ✅ 用于射线拾取 |
| `AABB` | `engine_math/src/aabb.rs` | ✅ 用于实体包围盒 |
| `Frustum` | `engine_math/src/frustum.rs` | ✅ 用于视锥剔除 |
| `EditorCamera` (FPS 飞行) | `engine_editor/src/camera_control.rs` | ✅ 扩展为多模式相机 |
| `RenderBridge` + `RenderCommand` | `engine_editor/src/render_bridge.rs` | ⚠️ 需重构为多实体渲染 |
| 坐标变换 (`perspective_division`, `viewport_transform`) | `engine_render_sw/src/renderers/base.rs` | ✅ 反向使用实现 Screen→World |

---

## 3. 系统架构

### 3.1 数据流

```
┌─────────────────── 主线程 (egui) ───────────────────┐
│                                                      │
│  EditorApp                                           │
│  ├── Scene (hecs::World + AssetManager)  ← 真正 ECS │
│  ├── SelectionState { selected: Option<Entity> }     │
│  ├── EditorCamera (多模式)                           │
│  ├── GizmoState { mode, active_axis }                │
│  ├── CommandHistory (Undo/Redo)                      │
│  └── Panels                                          │
│       ├── Viewport → egui-gizmo overlay              │
│       ├── SceneTree → 实体列表 + 右键菜单            │
│       ├── Properties → 逐实体组件编辑                │
│       ├── Console → 日志                             │
│       └── StatusBar → FPS + 项目名                   │
│                                                      │
│  ┌─── 每帧同步 ───┐                                 │
│  │ 收集 DrawCall[] │                                 │
│  │ 发送到渲染线程  │                                 │
│  └────────┬───────┘                                  │
└───────────┼──────────────────────────────────────────┘
            │ RenderCommand::SubmitFrame(Vec<DrawCall>)
            ▼
┌─────────────── 渲染线程 ─────────────────┐
│  for each DrawCall:                       │
│    shader.set_model_matrix(dc.transform)  │
│    renderer.draw_model(dc.model, shader)  │
│                                           │
│  → 输出到 TripleBuffer → 主线程读取      │
└───────────────────────────────────────────┘
```

### 3.2 核心重构：从单模型到多实体渲染

**当前**：渲染线程持有 `Option<Model>`，每帧渲染一个模型。

**目标**：渲染线程接收 `Vec<DrawCall>`，每帧渲染所有可见实体。

```rust
// 新增数据结构
struct DrawCall {
    model_id: AssetId,     // 引用 AssetManager 中的模型
    model_matrix: Mat4,    // 该实体的世界变换
    entity_id: u32,        // 用于 ID Buffer 拾取
}

// 新增 RenderCommand 变体
enum RenderCommand {
    // ... 保留现有的 SetCamera, SetLights, SetRenderingMode 等
    RegisterModel { id: AssetId, model: Model },  // 注册模型到渲染线程
    UnregisterModel { id: AssetId },               // 移除模型
    SubmitFrame(Vec<DrawCall>),                     // 每帧提交所有绘制调用
}
```

**主线程每帧**：

```rust
fn collect_draw_calls(scene: &Scene) -> Vec<DrawCall> {
    let mut calls = Vec::new();
    for (entity, (transform, mesh_renderer)) in
        scene.world.query::<(&Transform, &MeshRenderer)>().iter()
    {
        calls.push(DrawCall {
            model_id: mesh_renderer.model_handle.id(),
            model_matrix: transform.to_mat4(),
            entity_id: entity.to_bits().get() as u32,
        });
    }
    calls
}
```

**渲染线程**：

```rust
// render_loop 内部
RenderCommand::SubmitFrame(draw_calls) => {
    for dc in &draw_calls {
        if let Some(model) = model_registry.get(&dc.model_id) {
            shader.set_uniform(u::MODEL_MATRIX, dc.model_matrix);
            renderer.draw_model(model, &mut shader, buf)?;
        }
    }
}
```

---

## 4. 功能设计

### 4.1 自由相机（UE 风格多模式）

扩展现有 `EditorCamera`，新增交互模式：

| 交互 | 模式 | 行为 |
|------|------|------|
| 右键 + WASD | 飞行 (Fly) | 当前已有，保留 |
| 右键 + 鼠标 | 环视 (Look) | 当前已有，保留 |
| Alt + 左键拖拽 | 环绕 (Orbit) | 以选中实体为轴心旋转 |
| Alt + 右键拖拽 | 推拉 (Dolly) | 沿视线方向前后移动 |
| Alt + 中键拖拽 | 平移 (Pan) | 垂直于视线方向平移 |
| 滚轮 | 缩放 (Zoom) | 向前/向后移动 |
| F 键 | 聚焦 (Focus) | 移动相机到选中实体前方，看向实体中心 |

```rust
pub struct EditorCamera {
    // ... 现有字段
    mode: CameraMode,
    orbit_pivot: Option<Vec3>,  // 环绕轴心（选中实体位置）
}

enum CameraMode {
    Fly,    // 右键 + WASD
    Orbit,  // Alt + 左键
    Pan,    // Alt + 中键
    Dolly,  // Alt + 右键
}
```

**聚焦算法**：

```rust
fn focus_on_entity(&mut self, target_pos: Vec3, target_size: f32) {
    let distance = target_size * 2.5; // 保持适当距离
    let direction = (self.position - target_pos).normalize();
    self.position = target_pos + direction * distance;
    self.look_at(target_pos);
    self.orbit_pivot = Some(target_pos);
}
```

### 4.2 实体选择系统

**方案 A：射线拾取 (Ray Picking)** — 推荐首期实现

1. 用户点击视口
2. 将屏幕坐标反投影为世界射线：
   ```
   Screen (mx, my) → NDC: nx = mx/w*2-1, ny = 1-my/h*2
   Clip Near: (nx, ny, -1, 1)    Clip Far: (nx, ny, 1, 1)
   World = (Proj * View)^-1 * Clip
   Ray = { origin: camera_pos, dir: normalize(world_far - world_near) }
   ```
3. 对场景中所有实体的 AABB 做 `Ray::intersects_aabb`
4. 选择最近的命中实体（最小 t 值）

**方案 B：ID Buffer 拾取** — 后续优化

在渲染时为每个像素写入实体 ID，点击直接读取。实现精确到像素的选择。需要修改渲染器内部，复杂度较高，留作后续迭代。

### 4.3 Gizmo 系统

**方案：使用 `egui-gizmo` crate**

`egui-gizmo` 在 egui 画布上绘制 2D gizmo 覆盖层，不需要修改软件渲染器。

```toml
# engine_editor/Cargo.toml
egui-gizmo = "0.18"
```

```rust
// 在 viewport panel 中
if let Some(entity) = selection.selected {
    if let Ok(transform) = scene.world.get::<&Transform>(entity) {
        let gizmo = egui_gizmo::Gizmo::new("transform_gizmo")
            .view_matrix(camera.view_matrix().to_cols_array_2d().into())
            .projection_matrix(camera.projection_matrix(aspect).to_cols_array_2d().into())
            .model_matrix(transform.to_mat4().to_cols_array_2d().into())
            .mode(gizmo_state.mode); // Translate / Rotate / Scale

        if let Some(response) = gizmo.interact(ui) {
            // 通过 CommandHistory 更新 Transform
            let new_matrix = Mat4::from_cols_array_2d(&response.transform());
            commands.execute(MoveCommand { entity, old, new: new_matrix });
        }
    }
}
```

**Gizmo 模式切换**：

| 快捷键 | 模式 |
|--------|------|
| W | 平移 (Translate) |
| E | 旋转 (Rotate) |
| R | 缩放 (Scale) |

### 4.4 Transform 层级传播

利用已有的 `Parent`、`Children`、`GlobalTransform` 组件：

```rust
fn propagate_transforms(world: &mut hecs::World) {
    // 第 1 步：收集根实体（有 Transform 但没有 Parent）
    let roots: Vec<Entity> = world
        .query::<&Transform>()
        .without::<&Parent>()
        .iter()
        .map(|(e, _)| e)
        .collect();

    // 第 2 步：递归传播
    for root in roots {
        let local_mat = world.get::<&Transform>(root).unwrap().to_mat4();
        let _ = world.insert_one(root, GlobalTransform(local_mat));
        propagate_children(world, root, local_mat);
    }
}

fn propagate_children(world: &hecs::World, parent: Entity, parent_global: Mat4) {
    // 注意：由于 hecs 的借用规则，需要先收集子实体 ID
    let children: Vec<Entity> = world
        .get::<&Children>(parent)
        .map(|c| c.0.clone())
        .unwrap_or_default();

    for child in children {
        if let Ok(local) = world.get::<&Transform>(child) {
            let global_mat = parent_global * local.to_mat4();
            let _ = world.insert_one(child, GlobalTransform(global_mat));
            propagate_children(world, child, global_mat);
        }
    }
}
```

### 4.5 场景树操作

升级 `panels/scene_tree.rs`：

| 操作 | 交互方式 | 实现 |
|------|----------|------|
| 选择实体 | 左键点击 | 设置 `SelectionState.selected` |
| 添加空实体 | 右键菜单 → "添加空实体" | `world.spawn((Name, Transform))` |
| 添加模型实体 | 右键菜单 → "添加模型..." | 文件对话框 → `scene.spawn_model()` |
| 添加灯光 | 右键菜单 → "添加灯光" | `scene.spawn_light()` |
| 添加相机 | 右键菜单 → "添加相机" | `scene.spawn_camera()` |
| 删除实体 | 右键菜单 → "删除" / Delete 键 | `world.despawn(entity)` |
| 复制实体 | Ctrl+D / 右键菜单 | 克隆组件到新实体 |
| 重命名 | 双击名称 | 编辑 `Name` 组件 |

### 4.6 逐实体属性编辑

升级 `panels/properties.rs`：

```rust
fn show_entity_properties(ui: &mut egui::Ui, world: &mut hecs::World, entity: Entity) {
    // Transform 编辑
    if let Ok(mut t) = world.get::<&mut Transform>(entity) {
        egui::CollapsingHeader::new("Transform").default_open(true).show(ui, |ui| {
            ui.horizontal(|ui| {
                ui.label("位置");
                ui.add(egui::DragValue::new(&mut t.translation.x).prefix("X:").speed(0.1));
                ui.add(egui::DragValue::new(&mut t.translation.y).prefix("Y:").speed(0.1));
                ui.add(egui::DragValue::new(&mut t.translation.z).prefix("Z:").speed(0.1));
            });
            // 旋转（欧拉角展示）、缩放同理
        });
    }

    // MeshRenderer 编辑
    if let Ok(mr) = world.get::<&MeshRenderer>(entity) {
        egui::CollapsingHeader::new("网格渲染器").show(ui, |ui| {
            ui.label(format!("模型 ID: {}", mr.model_handle.id()));
        });
    }

    // LightComponent 编辑
    if let Ok(mut light) = world.get::<&mut LightComponent>(entity) {
        egui::CollapsingHeader::new("灯光").show(ui, |ui| {
            // direction, color, intensity sliders
        });
    }

    // CameraComponent 编辑
    if let Ok(mut cam) = world.get::<&mut CameraComponent>(entity) {
        egui::CollapsingHeader::new("相机").show(ui, |ui| {
            // fov, near, far sliders
        });
    }
}
```

### 4.7 撤销/重做系统

基于 Command 模式：

```rust
trait EditorCommand: Send {
    fn execute(&mut self, scene: &mut Scene);
    fn undo(&mut self, scene: &mut Scene);
    fn description(&self) -> &str;
}

struct CommandHistory {
    undo_stack: Vec<Box<dyn EditorCommand>>,
    redo_stack: Vec<Box<dyn EditorCommand>>,
}

impl CommandHistory {
    fn execute(&mut self, mut cmd: Box<dyn EditorCommand>, scene: &mut Scene) {
        cmd.execute(scene);
        self.undo_stack.push(cmd);
        self.redo_stack.clear(); // 新操作清空 redo
    }

    fn undo(&mut self, scene: &mut Scene) {
        if let Some(mut cmd) = self.undo_stack.pop() {
            cmd.undo(scene);
            self.redo_stack.push(cmd);
        }
    }

    fn redo(&mut self, scene: &mut Scene) {
        if let Some(mut cmd) = self.redo_stack.pop() {
            cmd.execute(scene);
            self.undo_stack.push(cmd);
        }
    }
}
```

**预定义命令**：

| 命令 | 撤销行为 |
|------|----------|
| `TransformCommand { entity, old_transform, new_transform }` | 恢复旧 Transform |
| `SpawnEntityCommand { entity, components }` | Despawn 该实体 |
| `DespawnEntityCommand { entity, saved_components }` | 重新 Spawn |
| `RenameCommand { entity, old_name, new_name }` | 恢复旧名称 |

---

## 5. 新增依赖

| Crate | 版本 | 用途 |
|-------|------|------|
| `egui-gizmo` | 0.18 | 视口内 Transform gizmo |

注：`serde`、`toml` 已在前一期添加。

---

## 6. 文件变更清单

### 新增文件

| 文件 | 职责 |
|------|------|
| `crates/engine_editor/src/selection.rs` | SelectionState + 射线拾取逻辑 |
| `crates/engine_editor/src/commands.rs` | EditorCommand trait + CommandHistory + 具体命令 |
| `crates/engine_editor/src/gizmo.rs` | GizmoState + egui-gizmo 集成 |

### 修改文件

| 文件 | 变更内容 |
|------|----------|
| `crates/engine_editor/Cargo.toml` | 添加 `egui-gizmo` 依赖 |
| `crates/engine_editor/src/main.rs` | 添加新模块声明 |
| `crates/engine_editor/src/app.rs` | **核心重构**：用 `Scene` 替换 `Vec<String>`，集成选择/gizmo/命令系统 |
| `crates/engine_editor/src/render_bridge.rs` | 添加 `RegisterModel`/`SubmitFrame` 命令，渲染循环改为多模型 |
| `crates/engine_editor/src/camera_control.rs` | 添加 Orbit/Pan/Dolly/Focus 模式 |
| `crates/engine_editor/src/panels/scene_tree.rs` | 从 `Vec<String>` 改为查询 `hecs::World`，添加右键菜单 |
| `crates/engine_editor/src/panels/properties.rs` | 从全局 `EditorState` 改为逐实体组件编辑 |
| `crates/engine_editor/src/panels/viewport.rs` | 集成 gizmo overlay + 点击选择 |
| `crates/engine_core/src/transform.rs` | 添加 `from_mat4()` 方法（从矩阵分解） |
| `crates/engine_scene/src/components.rs` | 添加 `BoundingBox` 组件（用于拾取） |

---

## 7. 分期实施计划

### 第 1 期：多实体渲染 + ECS 集成 (基础)

**目标**：场景中可以存在多个独立实体，各自有独立 Transform。

1. 重构 `RenderBridge`：添加 `RegisterModel` + `SubmitFrame` 命令
2. 重构 `render_loop`：从单模型改为迭代 `Vec<DrawCall>`
3. 用 `engine_scene::Scene` 替换 `EditorApp` 中的 `Vec<String>`
4. 场景树从 `Vec<String>` 改为查询 `World` 中的 `Name` 组件
5. 实现 `propagate_transforms()` 系统
6. 属性面板显示选中实体的 Transform（Position/Rotation/Scale DragValue）

**验收标准**：可以在场景中加载多个 .obj 模型，各自独立显示和选择。

### 第 2 期：自由相机 + 实体选择

**目标**：UE 风格的视口交互。

1. 扩展 `EditorCamera`：Orbit / Pan / Dolly / Focus 模式
2. 实现射线拾取（Screen→World 反投影 + Ray-AABB 测试）
3. 视口点击选择实体
4. F 键聚焦选中实体
5. 添加 `BoundingBox` 组件，模型加载时自动计算

**验收标准**：可以点击视口中的模型选中它，Alt+左键环绕，F 聚焦。

### 第 3 期：Gizmo + 撤销/重做

**目标**：可视化编辑 + 操作安全。

1. 集成 `egui-gizmo`
2. W/E/R 快捷键切换 gizmo 模式
3. 实现 `CommandHistory` + `TransformCommand`
4. Ctrl+Z / Ctrl+Shift+Z 撤销/重做
5. 场景树右键菜单（添加/删除/复制实体）

**验收标准**：拖拽 gizmo 修改实体位置/旋转/缩放，可以撤销/重做。

### 第 4 期：场景序列化 + 打磨

**目标**：场景可持久化，编辑体验打磨。

1. 场景序列化/反序列化（与项目系统集成）
2. 实体复制（Ctrl+D）
3. 快捷键系统完善
4. Grid 地面参考线
5. 视锥剔除优化（使用 `Frustum`）

**验收标准**：完整的创建→编辑→保存→重新打开工作流。

---

## 8. 风险与约束

| 风险 | 影响 | 缓解措施 |
|------|------|----------|
| `egui-gizmo` 与 egui 0.31 兼容性 | gizmo 无法渲染 | 检查版本兼容性，必要时 fork 适配 |
| 多模型渲染性能 | 帧率下降 | 视锥剔除 + 限制场景实体数 |
| hecs 借用规则与 UI 交互冲突 | 无法同时查询和修改 World | 使用 `CommandQueue` 延迟执行修改 |
| 渲染线程与主线程模型同步 | 数据竞争 | 模型通过 ID 注册，渲染线程持有独立副本 |

---

## 9. 成功标准

完成所有 4 期后：

1. ✅ 场景中可放置多个模型、灯光、相机实体
2. ✅ 点击视口或场景树可选择实体
3. ✅ Gizmo 可拖拽编辑 Transform（平移/旋转/缩放）
4. ✅ 自由相机支持飞行、环绕、平移、推拉、聚焦
5. ✅ 属性面板显示并编辑选中实体的所有组件
6. ✅ 支持 Ctrl+Z/Ctrl+Shift+Z 撤销/重做
7. ✅ 场景可保存到项目文件并重新加载
8. ✅ `cargo clippy --workspace -- -D warnings` 通过
9. ✅ 现有测试全部通过 + 新增测试覆盖核心逻辑
