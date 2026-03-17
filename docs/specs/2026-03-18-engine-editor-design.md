# SimpleRenderer → SimpleEngine: 编辑器原型设计

**日期**: 2026-03-18
**状态**: 待审阅
**范围**: 引擎架构设计 + 可工作的编辑器原型（第一期交付）

## 1. 目标

将 SimpleRenderer 从教育渲染器演进为独立游戏引擎（SimpleEngine），本期交付：
- 引擎核心架构（ECS + 模块化 crate）
- 可工作的编辑器原型（egui，包含场景视口 + 属性面板 + 资产加载 + 参数调整）
- 保持现有渲染器作为引擎的渲染后端，不破坏现有 API

## 2. Workspace 架构

```
SimpleEngine/               (原 SimpleRenderer/)
├── Cargo.toml              # workspace root
├── crates/
│   ├── engine_core/        # ECS + 事件总线 + 时间管理
│   ├── engine_renderer/    # 原 simple_renderer，适配为引擎渲染模块
│   ├── engine_scene/       # 场景图 + 资产管理 + 序列化
│   └── engine_editor/      # egui 编辑器（替代 system_test）
├── assets/                 # 原 obj/ 目录，扩展为通用资产目录
└── docs/
```

无独立数学 crate — 所有 crate 直接依赖 `glam`。如未来需要引擎特定数学工具
（AABB、Frustum、Ray 等），在 `engine_core` 中以 `math` 模块形式添加。

### 2.1 Crate 职责

| Crate | 职责 | 依赖 |
|-------|------|------|
| `engine_core` | ECS World、Component trait、System trait、事件通道、Time | glam |
| `engine_renderer` | 渲染管线（现有 simple_renderer 重构） | engine_core, glam |
| `engine_scene` | Scene、Entity 管理、资产加载/缓存、序列化 | engine_core, engine_renderer |
| `engine_editor` | egui 编辑器 GUI（本期核心交付） | 所有上述 crate, eframe, egui, rfd |

### 2.2 迁移策略

现有 `simple_renderer/` 代码基本不动，只做以下调整：
1. 移动到 `crates/engine_renderer/`
2. `SimpleRenderer` 实现引擎定义的 `RenderBackend` trait
3. 原有单元测试、集成测试、property tests 全部保留
4. `system_test/` 被 `engine_editor/` 完全替代

## 3. ECS 设计

### 3.1 核心类型

```rust
// engine_core/src/ecs.rs
pub type Entity = u64;

pub trait Component: Send + Sync + 'static {}

pub struct World {
    entities: Vec<Entity>,
    storages: HashMap<TypeId, Box<dyn AnyStorage>>,
    // ...
}
```

### 3.2 内置 Component

| Component | Crate | 字段 |
|-----------|-------|------|
| `Transform` | engine_core | position: Vec3, rotation: Quat, scale: Vec3 |
| `MeshRenderer` | engine_renderer | model: Handle<Model>, material: Handle<Material> |
| `Camera` | engine_renderer | fov, near, far, aspect, projection |
| `Light` | engine_renderer | kind: LightKind, color, intensity, direction |
| `Name` | engine_core | name: String |

### 3.3 内置 System

| System | 作用 |
|--------|------|
| `RenderSystem` | 遍历 (Transform, MeshRenderer, Camera, Light) → 调用渲染器 |
| `InputSystem` | 从 egui 事件映射到引擎输入状态 |
| `CameraControlSystem` | FPS 相机控制 |

### 3.4 第一期简化

本期 ECS 目标是**最小可用**，不追求 bevy_ecs 级别的调度器。具体：
- 无并行 System 调度（串行执行）
- 无 Query 缓存（每帧遍历）
- 无 Archetype 存储（HashMap<TypeId, Vec<Box<dyn Any>>>）
- 后续迭代可替换为 hecs 或 bevy_ecs

## 4. 资产管理

### 4.1 Handle 模式

```rust
// engine_scene/src/asset.rs
pub struct Handle<T> {
    id: AssetId,
    _marker: PhantomData<T>,
}

pub struct AssetManager {
    models: HashMap<AssetId, Model>,
    materials: HashMap<AssetId, Material>,
    textures: HashMap<AssetId, Texture>,
}
```

### 4.2 加载流程

1. 用户在编辑器中点击 File → Open Model
2. `rfd::FileDialog` 弹出原生文件选择器
3. 选中 .obj 文件 → `AssetManager::load_model(path)` → 返回 `Handle<Model>`
4. 创建 Entity，附加 `Transform + MeshRenderer(handle)` Component
5. RenderSystem 在下一帧渲染该 Entity

纹理加载同理：File → Load Texture → 选择图片 → 替换当前选中实体的材质纹理。

## 5. 编辑器 GUI 设计

### 5.1 布局

```
┌─ 菜单栏 ─────────────────────────────────────┐
│ 文件 | 视图 | 帮助                             │
├──────────┬────────────────────────┬───────────┤
│ 场景树    │                        │ 属性面板   │
│          │     渲染视口             │           │
│ Entity   │  (软渲染 → Texture)     │ Transform │
│ 列表     │                        │ Material  │
│          │  鼠标控制相机            │ Light     │
│          │                        │ Camera    │
│          │                        │ Renderer  │
├──────────┴────────────────────────┴───────────┤
│ 状态栏: FPS | 渲染耗时 | Entity 数 | 渲染模式   │
└───────────────────────────────────────────────┘
```

### 5.2 面板功能

**菜单栏**:
| 菜单 | 功能 |
|------|------|
| 文件 → 打开模型 | rfd 文件对话框，选 .obj 文件，创建 Entity 并加载 |
| 文件 → 加载贴图 | rfd 对话框，选图片文件，替换选中实体的材质纹理 |
| 文件 → 退出 | 退出 |

**场景树**（左侧）:
- 列出所有 Entity（显示 Name component）
- 点击选中 → 右侧属性面板显示该 Entity 的 Component
- 右键菜单：删除 Entity

**属性面板**（右侧）:
| 区块 | 控件 |
|------|------|
| Transform | position/rotation/scale 各 3 个 DragValue |
| MeshRenderer | 模型路径（只读）+ 材质参数 |
| 材质 | shininess slider [1..256] + ambient/specular 系数 slider [0..1] |
| Light | direction 3x slider + color picker + intensity slider |
| Camera | FOV slider [30..120] + near/far + 移动速度 |

**渲染设置**（属性面板底部，全局）:
| 控件 | 说明 |
|------|------|
| 渲染模式 | ComboBox：PerTriangle / TileBased / Deferred / TBDR |
| Tile 大小 | slider [16..128]，仅 TileBased/TBDR 模式可用 |
| Early-Z | checkbox，仅 TileBased 模式可用 |
| VSync | checkbox |
| 缓冲模式 | Radio: 双缓冲 / 三缓冲 |

**状态栏**（底部）:
- 实时 FPS（来自渲染线程计时）
- 渲染耗时（ms）
- Entity 总数
- 当前渲染模式名称

### 5.3 渲染视口

- 渲染线程生产像素 → triple buffer → 编辑器线程读取 → 上传为 `egui::TextureHandle`
- 视口区域内：右键拖拽旋转相机、WASD 移动（需 egui 焦点在视口上时响应）
- 滚轮调整移动速度
- 视口大小随窗口缩放 → 通知渲染线程更新分辨率

### 5.4 渲染线程通信

```
EditorApp (主线程/GUI)
    │
    ├─ 写 → RenderCommand channel ─→ RenderThread
    │       (模型变更、相机更新、参数变更)
    │
    └─ 读 ← TripleBuffer ←─────── RenderThread
            (像素数据 + 帧统计)
```

`RenderCommand` 枚举：
```rust
enum RenderCommand {
    LoadModel(PathBuf),
    SetCamera { view: Mat4, projection: Mat4, pos: Vec3 },
    SetRenderingMode(RenderingMode),
    SetTileSize(usize),
    SetEarlyZ(bool),
    SetLights(Vec<Light>),
    SetMaterial { entity: Entity, material: Material },
    Resize { width: usize, height: usize },
    Shutdown,
}
```

## 6. 新增依赖

| Crate | 版本 | 用途 |
|-------|------|------|
| eframe | 0.31 | egui 原生窗口后端 |
| egui | 0.31 | 即时模式 GUI |
| rfd | 0.15 | 跨平台原生文件对话框 |

## 7. 不变量

- `engine_renderer`（原 simple_renderer）的公开 API 不破坏
- 原有 223 个测试全部保留并通过
- 渲染结果和之前一致（同样的模型 + 参数 = 同样的像素输出）

## 8. 后续迭代（不在本期范围）

| 迭代 | 内容 |
|------|------|
| 第二期 | 场景序列化（JSON/RON）、多 Entity 场景、Transform 层级 |
| 第三期 | 替换自写 ECS 为 hecs/bevy_ecs、并行 System 调度 |
| 第四期 | 物理引擎集成（rapier）、碰撞体 Component |
| 第五期 | 脚本系统（Lua 或 WASM）|
| 第六期 | GPU 渲染后端（wgpu）作为可选替代软渲染 |
| 第七期 | 音频系统、UI 系统（游戏内 UI）|

## 9. 成功标准

本期交付完成的判定标准：
1. `cargo build --workspace` 编译通过
2. 原有 223 个测试全部通过
3. 编辑器启动后显示默认场景（犹他茶壶）
4. 可通过菜单加载新的 .obj 模型和贴图
5. 可通过属性面板实时调整所有参数（光照、材质、相机、渲染模式、Tile 大小等）
6. 视口内相机控制流畅
7. FPS 和渲染耗时正确显示
8. `cargo clippy --workspace -- -D warnings` 通过
