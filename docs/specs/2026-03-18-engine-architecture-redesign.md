# SimpleEngine 全面架构重设计

**日期**: 2026-03-18
**状态**: 已审阅 (Oracle + Deep 双模型审阅完成)
**范围**: 全面架构重设计——从教育渲染器演进为模块化游戏引擎
**定位**: 教育 + 实用混合（代码可读性优先，但架构对标现代引擎）

## 1. 设计目标

### 1.1 核心目标

1. **模块化**: 每个子系统是独立 crate，通过 trait 接口连接，用户可按需引入
2. **教育性**: 代码可读，每个模块可独立学习；软件渲染保留为教学参考实现
3. **实用性**: 架构对标 Bevy/Godot 级别，支持制作真实游戏
4. **可扩展**: Plugin 系统支持第三方扩展，渲染后端可切换

### 1.2 非目标

- 不追求 AAA 级渲染质量
- 不追求 bevy_ecs 级别的调度器性能
- 不追求 100% Bevy/Fyrox 功能对等

## 2. 架构总览

### 2.1 分层架构

```
Layer 4 — Application
  engine_app          App Builder, 主循环, Plugin 注册
  engine_editor       egui 编辑器 GUI

Layer 3 — Feature (全部可选)
  engine_physics      rapier 物理引擎集成
  engine_audio        kira 游戏音频集成
  engine_animation    骨骼动画 + 关键帧 + 状态机
  engine_script       rhai 脚本系统
  engine_network      可靠 UDP 网络层 (renet) (审阅修复 #6: quinn→renet)
  engine_2d           Sprite + TileMap + 2D 渲染

Layer 2 — Infrastructure
  engine_scene        Scene 序列化 (RON) + AssetManager + Prefab
  engine_render_sw    软件渲染后端 (现有代码迁移)
  engine_render_wgpu  wgpu GPU 渲染后端

Layer 1 — Foundation
  engine_core         ECS (hecs) + Event Bus + Plugin Trait + Schedule + Time
                      + AssetId/Handle<T>/AssetLoader trait (审阅修复 #1)
                      + DespawnQueue 事件机制 (审阅修复 #4)
  engine_render_api   RenderBackend trait + FrameData + RenderGraph 类型定义
  engine_input        统一输入状态管理 (键盘/鼠标/手柄) (审阅修复 #3)
  engine_math         glam re-export + AABB/Ray/Frustum 工具
```

### 2.2 分层规则（严格执行）

1. 上层可依赖下层，**绝不反向依赖**
2. 同层 crate **互不依赖**（engine_render_sw 和 engine_render_wgpu 互不知晓）
3. Layer 3 的所有 Feature crate 都是**可选的**（cargo feature gate）
4. `engine_render_api` 和 `engine_core` **互不依赖**（渲染抽象与 ECS 解耦）
5. 任何 crate 的公开 API 变更需要更新本文档

### 2.3 Crate 依赖图

```
engine_editor ──┬── engine_app
                ├── engine_scene ──┬── engine_core
                │                  ├── engine_render_api
                │                  └── engine_math
                ├── engine_render_sw ── engine_render_api ── engine_math
                ├── engine_render_wgpu ── engine_render_api ── engine_math
                ├── engine_physics ── engine_core
                ├── engine_audio ── engine_core
                ├── engine_animation ──┬── engine_core
                │                      └── engine_math
                ├── engine_script ── engine_core
                ├── engine_network ── engine_core
                └── engine_2d ──┬── engine_core
                                ├── engine_render_api
                                └── engine_physics (optional)
```

## 3. engine_core — 核心层

### 3.1 ECS: hecs

**选型分析:**

| 维度 | bevy_ecs | hecs | 自写 (现有) |
|------|----------|------|------------|
| 代码量 | ~50K LOC | ~2K LOC | ~200 LOC |
| 存储模型 | Archetype | Archetype | HashMap |
| 并行调度 | ✅ 内置 | ❌ 需自建 | ❌ |
| Query 缓存 | ✅ | ✅ | ❌ |
| 传递依赖 | ~30 crates | ~5 crates | 0 |
| 教育可读性 | ⭐⭐ | ⭐⭐⭐⭐ | ⭐⭐⭐⭐⭐ |
| 生产性能 | ⭐⭐⭐⭐⭐ | ⭐⭐⭐⭐ | ⭐⭐ |
| 教育+实用平衡 | ⭐⭐⭐ | ⭐⭐⭐⭐⭐ | ⭐⭐⭐ |

**决策**: 采用 `hecs`。源码 ~2000 行可通读，Archetype 存储性能好，无宏无框架绑定。

### 3.2 Plugin 系统

```rust
/// 插件 trait — 比 Bevy 简单，但足够实用
pub trait Plugin: Send + Sync + 'static {
    /// 插件名称（用于日志和调试）
    fn name(&self) -> &str;

    /// 插件依赖（审阅修复 #5：保证初始化顺序）
    /// 返回此插件依赖的其他插件名称，AppBuilder 在 build() 前进行拓扑排序
    fn dependencies(&self) -> Vec<&str> { vec![] }

    /// 构建阶段：注册 System、Resource、Event Handler
    fn build(&self, app: &mut AppBuilder);

    /// 清理阶段（可选）：释放资源
    fn cleanup(&self, _app: &mut AppBuilder) {}
}
```

### 3.3 AppBuilder

```rust
pub struct AppBuilder {
    pub world: hecs::World,
    pub resources: ResourceMap,
    pub events: EventBus,
    pub schedule: Schedule,
    plugins: Vec<Box<dyn Plugin>>,
}

impl AppBuilder {
    pub fn new() -> Self;
    pub fn add_plugin(&mut self, plugin: impl Plugin) -> &mut Self;
    pub fn add_system(&mut self, phase: Phase, system: impl System) -> &mut Self;
    pub fn insert_resource<T: Send + Sync + 'static>(&mut self, resource: T) -> &mut Self;
    pub fn run(&mut self);  // 主循环
}
```

### 3.4 Schedule — Phase-based 调度

```rust
pub enum Phase {
    PreUpdate,      // 输入处理、物理同步到引擎
    Update,         // 游戏逻辑、脚本执行
    PostUpdate,     // 物理结果同步回来、Transform 层级更新
    PreRender,      // 渲染数据收集 (FrameData 构建)
    Render,         // 提交给 RenderBackend
    PostRender,     // 帧统计、截图
}
```

系统在同一 Phase 内按注册顺序**串行执行**。不同 Phase 按固定顺序执行。
这是有意的简化——教育目标下，串行执行更容易理解和调试。

### 3.5 事件总线

```rust
pub struct EventBus {
    channels: HashMap<TypeId, Box<dyn Any>>,
}

impl EventBus {
    /// 发送事件
    pub fn send<E: Event>(&mut self, event: E);

    /// 读取本帧所有事件（下帧自动清空）
    pub fn read<E: Event>(&self) -> impl Iterator<Item = &E>;
}

pub trait Event: Send + Sync + 'static {}
```

### 3.6 Resource 存储

```rust
/// 类型化全局资源 (类似 Bevy 的 Res<T>)
pub struct ResourceMap {
    map: HashMap<TypeId, Box<dyn Any + Send + Sync>>,
}

impl ResourceMap {
    pub fn insert<T: Send + Sync + 'static>(&mut self, resource: T);
    pub fn get<T: Send + Sync + 'static>(&self) -> Option<&T>;
    pub fn get_mut<T: Send + Sync + 'static>(&mut self) -> Option<&mut T>;
}
```

### 3.7 模块结构

```
engine_core/
├── Cargo.toml
└── src/
    ├── lib.rs           # 模块声明 + pub re-exports
    ├── plugin.rs        # Plugin trait + PluginRegistry
    ├── app.rs           # AppBuilder + 主循环
    ├── event.rs         # EventBus<T>
    ├── time.rs          # Time resource (delta, total, frame_count)
    ├── schedule.rs      # Phase enum + Schedule + System trait
    └── resource.rs      # ResourceMap (TypeMap)
```

## 4. engine_render_api — 渲染抽象层

### 4.1 RenderBackend trait

```rust
/// 渲染后端抽象 — 软件渲染和 wgpu 都实现此 trait
pub trait RenderBackend: Send + 'static {
    type Texture: Send + Sync;
    type Mesh: Send + Sync;
    type Material: Send + Sync;

    /// 初始化后端
    fn init(&mut self, config: RenderConfig) -> Result<()>;

    /// 提交一帧的渲染命令
    fn render_frame(&mut self, frame: &FrameData) -> Result<FrameOutput>;

    /// 调整渲染目标大小
    fn resize(&mut self, width: u32, height: u32);

    /// 创建网格资源
    fn create_mesh(&mut self, data: &MeshData) -> Result<Self::Mesh>;

    /// 创建纹理资源
    fn create_texture(&mut self, data: &TextureData) -> Result<Self::Texture>;

    /// 获取帧缓冲像素（用于 egui 显示或截图）
    fn read_pixels(&self) -> &[u8];

    /// 后端名称（用于 UI 显示）
    fn name(&self) -> &str;
}
```

### 4.2 FrameData — 后端无关的渲染描述

```rust
/// 一帧的渲染数据，由 RenderSystem 从 ECS 收集并打包
pub struct FrameData {
    pub camera: CameraData,
    pub lights: Vec<LightData>,
    pub draw_calls: Vec<DrawCall>,
    pub clear_color: Color,
    pub viewport: Viewport,
}

pub struct CameraData {
    pub view: Mat4,
    pub projection: Mat4,
    pub position: Vec3,
    pub fov: f32,
    pub near: f32,
    pub far: f32,
}

pub struct LightData {
    pub kind: LightKind,       // Directional / Point / Spot
    pub position: Vec3,
    pub direction: Vec3,
    pub color: Vec3,
    pub intensity: f32,
}

pub struct DrawCall {
    pub mesh: MeshHandle,
    pub material: MaterialHandle,
    pub transform: Mat4,
}

pub struct FrameOutput {
    pub frame_time_ms: f64,
    pub triangle_count: u32,
    pub draw_call_count: u32,
}
```

**设计原则**: RenderSystem 负责将 ECS 世界中的 (Transform, MeshRenderer, Camera, Light) 组件收集为 `FrameData`，然后提交给 `RenderBackend`。后端只需要处理 `FrameData`，无需知道 ECS 的存在。

### 4.3 模块结构

```
engine_render_api/
├── Cargo.toml
└── src/
    ├── lib.rs           # re-exports
    ├── backend.rs       # RenderBackend trait
    ├── frame.rs         # FrameData, DrawCall, CameraData, LightData
    ├── mesh.rs          # MeshData, MeshHandle, VertexAttribute
    ├── texture.rs       # TextureData, TextureHandle, TextureFormat
    ├── material.rs      # MaterialData, MaterialHandle
    ├── color.rs         # Color (RGBA, 统一颜色类型)
    └── viewport.rs      # Viewport, RenderConfig
```

## 5. engine_render_sw — 软件渲染后端

### 5.1 迁移策略

现有 `engine_renderer` 的全部代码迁移到此 crate，包括：
- 4 种渲染策略（PerTriangle, TileBased, Deferred, TileBasedDeferred）
- 完整的顶点/片段着色器
- 光栅化器
- 双缓冲/三缓冲
- 全部 207 个测试

在此基础上加一层 `RenderBackend` trait 适配器：

```rust
pub struct SoftwareRenderer {
    inner: SimpleRenderer,  // 现有渲染器
}

impl RenderBackend for SoftwareRenderer {
    type Texture = SoftwareTexture;
    type Mesh = Model;                // 现有 Model 类型
    type Material = SoftwareMaterial;

    fn render_frame(&mut self, frame: &FrameData) -> Result<FrameOutput> {
        // 将 FrameData 转换为现有渲染器的输入格式
        // 调用 self.inner.draw_model()
        // 返回 FrameOutput
    }

    fn read_pixels(&self) -> &[u8] {
        self.inner.framebuffer()
    }

    fn name(&self) -> &str { "Software Renderer" }
}
```

### 5.2 不变量

- 现有渲染管线内部逻辑**不修改**
- 所有 207 个测试**保留并通过**
- 渲染结果与重构前**逐像素一致**

## 6. engine_render_wgpu — GPU 渲染后端

### 6.1 架构

```rust
pub struct WgpuRenderer {
    instance: wgpu::Instance,
    device: wgpu::Device,
    queue: wgpu::Queue,
    surface: Option<wgpu::Surface>,
    // 管线状态
    forward_pipeline: wgpu::RenderPipeline,
    depth_texture: wgpu::Texture,
    // 资源管理
    mesh_pool: HashMap<MeshHandle, GpuMesh>,
    texture_pool: HashMap<TextureHandle, wgpu::Texture>,
    // 帧缓冲回读 (用于 egui 显示)
    readback_buffer: wgpu::Buffer,
}

impl RenderBackend for WgpuRenderer {
    type Texture = wgpu::Texture;
    type Mesh = GpuMesh;
    type Material = GpuMaterial;

    fn render_frame(&mut self, frame: &FrameData) -> Result<FrameOutput> {
        let mut encoder = self.device.create_command_encoder(&Default::default());
        // 1. 更新 Uniform Buffer (camera, lights)
        // 2. Begin RenderPass
        // 3. For each DrawCall: set_pipeline, set_bind_group, draw_indexed
        // 4. Submit
        // 5. 可选: 回读像素到 readback_buffer（仅当 egui 需要显示时）
    }
}
```

### 6.2 渲染管线

第一期: **前向渲染 (Forward Rendering)**
- Blinn-Phong 着色（与软件渲染器一致，方便对比验证）
- WGSL 着色器

后续迭代:
- PBR 材质 (metallic-roughness workflow)
- Shadow Mapping
- SSAO
- Deferred Rendering（可选）

## 7. engine_physics — 物理引擎

### 7.1 架构

```
engine_physics/
├── src/
│   ├── lib.rs              # PhysicsPlugin
│   ├── physics_world.rs    # PhysicsWorld resource (包装 rapier)
│   ├── components.rs       # RigidBody, Collider, Joint
│   ├── sync.rs             # ECS Transform ↔ Rapier Transform 双向同步
│   ├── events.rs           # CollisionEvent, ContactForceEvent
│   └── debug_render.rs     # 碰撞体线框可视化（教育用途）
```

### 7.2 System 执行顺序

```
PreUpdate:  SyncToPhysics    — 将 ECS Transform 写入 Rapier body
Update:     StepPhysics      — rapier_pipeline.step()
PostUpdate: SyncFromPhysics  — 将 Rapier body 位置写回 ECS Transform
PostUpdate: PublishCollisions — 碰撞事件发送到 EventBus
```

### 7.3 依赖

```toml
[dependencies]
rapier3d = { version = "0.22", features = ["simd-stable"] }
engine_core = { path = "../engine_core" }
engine_math = { path = "../engine_math" }
```

## 8. engine_audio — 音频系统

### 8.1 架构

```
engine_audio/
├── src/
│   ├── lib.rs              # AudioPlugin
│   ├── audio_engine.rs     # AudioEngine resource (包装 kira::AudioManager)
│   ├── components.rs       # AudioSource, AudioListener, SpatialEmitter
│   └── events.rs           # PlayAudio, StopAudio, SetVolume 事件
```

### 8.2 选型依据

| crate | 特点 | 评分 |
|-------|------|------|
| **kira** ⭐ | 游戏专用、音效链、空间音频、低延迟、流式播放 | ⭐⭐⭐⭐⭐ |
| rodio | 通用音频、API 简单、无空间音频 | ⭐⭐⭐ |
| cpal | 底层设备抽象、需自建混音器 | ⭐⭐ |
| oddio | 轻量游戏音频、社区较小 | ⭐⭐⭐ |

### 8.3 依赖

```toml
[dependencies]
kira = "0.9"
engine_core = { path = "../engine_core" }
```

## 9. engine_animation — 动画系统

### 9.1 架构

```
engine_animation/
├── src/
│   ├── lib.rs              # AnimationPlugin
│   ├── clip.rs             # AnimationClip (关键帧序列)
│   ├── player.rs           # AnimationPlayer component
│   ├── skeleton.rs         # Skeleton, Bone, Joint 层级
│   ├── skinning.rs         # 蒙皮 (CPU for SW, GPU for wgpu)
│   ├── blend.rs            # BlendTree (动画混合)
│   ├── state_machine.rs    # AnimationStateMachine (FSM)
│   └── interpolation.rs    # lerp, slerp, cubic 插值
```

### 9.2 设计要点

- `AnimationClip` 存储在 AssetManager 中（Handle 模式）
- `AnimationPlayer` 是 ECS Component，挂在 Entity 上
- Skinning 分两条路径:
  - 软件渲染: CPU skinning（在 `engine_render_sw` 中处理）
  - wgpu: Compute Shader skinning（在 `engine_render_wgpu` 中处理）
- 纯 Rust 实现，无第三方依赖（Rust 生态中无成熟的独立动画库）
- 参考 Bevy `bevy_animation` 和 glTF 动画规范设计

## 10. engine_script — 脚本系统

### 10.1 架构

```
engine_script/
├── src/
│   ├── lib.rs              # ScriptPlugin
│   ├── runtime.rs          # ScriptRuntime resource (rhai::Engine)
│   ├── bindings.rs         # 引擎 API → rhai 函数绑定
│   ├── component.rs        # ScriptComponent (附加脚本到 Entity)
│   └── hot_reload.rs       # 文件监听 + 脚本热重载
```

### 10.2 选型依据

| crate | 语言 | 特点 | 教育友好度 |
|-------|------|------|-----------|
| **rhai** ⭐ | Rhai (类 JS/Rust) | 纯 Rust、无 unsafe、类型安全绑定、沙箱 | ⭐⭐⭐⭐⭐ |
| mlua | Lua | 成熟生态、C FFI 绑定 | ⭐⭐⭐ |
| wasmtime | WASM | 最安全沙箱、多语言 | ⭐⭐ |

**决策**: rhai — 纯 Rust 实现，语法对 Rust 开发者友好，沙箱安全，API 绑定简洁。

### 10.3 依赖

```toml
[dependencies]
rhai = "1"
engine_core = { path = "../engine_core" }
notify = "7"  # 文件变更监听（热重载用）
```

## 11. engine_network — 网络系统

### 11.1 架构

```
engine_network/
├── src/
│   ├── lib.rs              # NetworkPlugin
│   ├── transport.rs        # Transport trait (协议抽象)
│   ├── quic_transport.rs   # QUIC 实现 (quinn)
│   ├── client.rs           # NetworkClient
│   ├── server.rs           # NetworkServer
│   ├── replication.rs      # Component 状态同步
│   └── protocol.rs         # 消息序列化 (bincode)
```

### 11.2 选型依据

| crate | 协议 | 特点 |
|-------|------|------|
| **renet** ⭐ | UDP (可靠+不可靠) | 同步 API、专为游戏设计、无 async 依赖、支持可靠/不可靠通道 |
| quinn | QUIC | 功能强但依赖 tokio async runtime，与同步游戏循环冲突 |
| laminar | UDP | 已停止维护 (2022) |
| naia | 自定义 | Bevy 绑定强但独立性差 |

**决策**: `renet` — 同步 API 完美适配 Phase-based 游戏循环，无需 tokio 桥接。
（审阅修复 #6：quinn 的 async 运行时与同步调度器存在根本冲突）

### 11.3 依赖

```toml
[dependencies]
renet = "0.0.16"
bincode = "1"
serde = { version = "1", features = ["derive"] }
engine_core = { path = "../engine_core" }
```

## 12. engine_2d — 2D 渲染与游戏支持

### 12.1 架构

```
engine_2d/
├── src/
│   ├── lib.rs              # Plugin2D
│   ├── sprite.rs           # Sprite, SpriteSheet, SpriteAnimation
│   ├── tilemap.rs          # TileMap, TileSet, Tile
│   ├── camera_2d.rs        # 2D 正交相机
│   ├── batch.rs            # Sprite 批处理渲染
│   └── physics_2d.rs       # rapier2d 桥接 (可选)
```

### 12.2 依赖

```toml
[dependencies]
engine_core = { path = "../engine_core" }
engine_render_api = { path = "../engine_render_api" }
engine_math = { path = "../engine_math" }

[dependencies.engine_physics]
path = "../engine_physics"
optional = true
```

## 13. engine_scene — 场景 + 资产管理

### 13.1 架构

```
engine_scene/
├── src/
│   ├── lib.rs
│   ├── scene.rs            # Scene (World 快照 + 元数据)
│   ├── asset.rs            # AssetManager + Handle<T> + AssetLoader trait
│   ├── loader/
│   │   ├── mod.rs
│   │   ├── obj_loader.rs   # OBJ/MTL (tobj)
│   │   ├── gltf_loader.rs  # glTF 2.0 (gltf crate)
│   │   └── texture_loader.rs
│   ├── serialize.rs        # RON 序列化/反序列化
│   ├── prefab.rs           # Prefab 系统 (可复用 Entity 模板)
│   └── transform.rs        # Transform 层级 (parent-child)
```

### 13.2 AssetLoader trait

```rust
/// 资产加载器 trait — 支持注册自定义加载器
pub trait AssetLoader: Send + Sync + 'static {
    type Asset: Send + Sync + 'static;
    fn extensions(&self) -> &[&str];  // e.g., &["obj", "OBJ"]
    fn load(&self, path: &Path) -> Result<Self::Asset>;
}
```

### 13.3 依赖

```toml
[dependencies]
engine_core = { path = "../engine_core" }
engine_render_api = { path = "../engine_render_api" }
engine_math = { path = "../engine_math" }
tobj = "4.0"
gltf = "1"
image = { version = "0.25", default-features = false, features = ["png", "jpeg", "bmp", "tga"] }
ron = "0.8"
serde = { version = "1", features = ["derive"] }
```

## 14. engine_editor — 编辑器（增强设计）

### 14.1 在现有 egui 编辑器基础上扩展

```
engine_editor/
├── src/
│   ├── main.rs
│   ├── app.rs              # EditorApp
│   ├── render_bridge.rs    # 渲染线程管理 (支持后端切换)
│   ├── camera_control.rs   # 视口 FPS 相机
│   ├── panels/
│   │   ├── mod.rs
│   │   ├── viewport.rs     # 渲染视口 (支持切换 SW/wgpu)
│   │   ├── scene_tree.rs   # 场景树 (支持拖拽重排)
│   │   ├── properties.rs   # 属性面板 (反射式自动生成)
│   │   ├── status_bar.rs   # FPS/帧耗时/Entity 数
│   │   ├── asset_browser.rs  # 新增: 资产浏览器
│   │   ├── console.rs        # 新增: 脚本控制台 + 日志输出
│   │   └── animation_editor.rs  # 新增: 动画编辑器 (时间轴)
│   ├── undo.rs             # 新增: Undo/Redo (Command pattern)
│   └── gizmo.rs            # 新增: 3D 变换手柄 (移动/旋转/缩放)
```

### 14.2 新增编辑器功能

| 功能 | 实现方式 |
|------|---------|
| 渲染后端切换 | 下拉菜单: Software / wgpu，运行时切换 RenderBackend |
| 资产浏览器 | 文件系统树形视图，拖拽到场景树添加 Entity |
| 脚本控制台 | rhai REPL，直接在编辑器中执行脚本命令 |
| 动画编辑器 | 时间轴 UI，关键帧编辑 |
| Undo/Redo | Command pattern，所有编辑操作可撤销 |
| 3D Gizmo | 视口内拖拽移动/旋转/缩放 Entity |
| 性能面板 | puffin 集成，帧内各阶段耗时可视化 |

## 15. 完整第三方依赖矩阵

### 15.1 运行时依赖

| 子系统 | 推荐 Crate | 版本 | 用途 | 替代方案 |
|--------|-----------|------|------|---------|
| ECS | `hecs` | 0.10 | Archetype ECS | bevy_ecs, specs, legion |
| 数学 | `glam` | 0.29 | 线性代数 (Vec3/Mat4) | nalgebra, ultraviolet |
| GPU 渲染 | `wgpu` | 26 | 跨平台 GPU API | vulkano, ash, glow |
| 窗口管理 | `winit` | 0.30 | 跨平台窗口 (wgpu 需要) | sdl2, minifb |
| 物理 | `rapier3d`/`rapier2d` | 0.22 | 刚体/碰撞/关节 | — (无竞争对手) |
| 音频 | `kira` | 0.9 | 游戏音频引擎 | rodio, oddio |
| 脚本 | `rhai` | 1 | 嵌入式脚本语言 | mlua, wasmtime |
| 网络 | `renet` | 0.0.16 | 同步可靠 UDP (游戏专用) | quinn (需 tokio) |
| 输入 | `gilrs` | 0.11 | 手柄/控制器输入 | — |
| OBJ 加载 | `tobj` | 4.0 | OBJ/MTL 模型加载 | — |
| glTF 加载 | `gltf` | 1 | glTF 2.0 (行业标准) | russimp |
| 纹理加载 | `image` | 0.25 | PNG/JPEG/BMP/TGA | — |
| 场景序列化 | `serde` + `ron` | 1 / 0.8 | RON 格式序列化 | serde_json |
| 网络编码 | `bincode` | 1 | 高效二进制序列化 | rkyv, postcard |
| GUI | `egui` + `eframe` | 0.31 | 编辑器 UI | iced |
| 文件对话框 | `rfd` | 0.15 | 原生文件选择器 | — |
| 并行计算 | `rayon` | 1.10 | 数据并行迭代 | — |
| 日志 | `tracing` | 0.1 | 结构化日志 + span | log (现有, 升级) |
| 日志输出 | `tracing-subscriber` | 0.3 | tracing 输出后端 | env_logger |
| 性能分析 | `puffin` | 0.19 | 帧内性能剖析 | tracy-client |
| 错误处理 | `thiserror` | 2 | 错误类型派生宏 | anyhow |
| 文件监听 | `notify` | 7 | 脚本热重载 | — |

### 15.2 开发依赖 (dev-dependencies)

| crate | 版本 | 用途 |
|-------|------|------|
| `proptest` | 1 | 属性测试（随机化测试） |
| `criterion` | 0.5 | 性能基准测试 |
| `insta` | 1 | 快照测试（渲染结果对比） |

## 16. 实施路线图

### Phase 0: 基础重构（2-3 周）

| # | 任务 | 依赖 | 可并行 |
|---|------|------|--------|
| 0.1 | 创建 `engine_math` (glam re-export + AABB/Ray/Frustum) | 无 | ✅ |
| 0.2 | 重写 `engine_core`: 替换 ECS 为 hecs, 实现 Plugin/Schedule/EventBus/Resource | 无 | ✅ |
| 0.3 | 创建 `engine_render_api` (RenderBackend trait + FrameData) | 无 | ✅ |
| 0.4 | `engine_renderer` → `engine_render_sw`, 实现 RenderBackend 适配 | 0.3 | |
| 0.5 | 更新 `engine_scene`: 适配 hecs World + 新 AssetManager | 0.2 | |
| 0.6 | 更新 `engine_editor`: 适配新 core API | 0.2, 0.4, 0.5 | |
| 0.7 | 升级 `log` → `tracing`, 全 workspace 统一 | 0.1-0.6 | |
| 0.8 | 全量测试 + CI 更新 | 全部 | |

**里程碑**: 编辑器在新架构下正常运行，所有现有功能不退化。

### Phase 1: GPU 渲染 + 场景增强（3-4 周）

| # | 任务 | 依赖 |
|---|------|------|
| 1.1 | 实现 `engine_render_wgpu` (基础前向渲染) | Phase 0 |
| 1.2 | 编辑器渲染后端切换 (SW ↔ wgpu) | 1.1 |
| 1.3 | 场景 RON 序列化 + serde 集成 | Phase 0 |
| 1.4 | glTF 加载器 (gltf crate) | Phase 0 |
| 1.5 | Transform 层级 (parent-child) | Phase 0 |
| 1.6 | Prefab 系统 | 1.3 |

**里程碑**: SW/wgpu 可切换，场景可保存/加载。

### Phase 2: 游戏系统（4-6 周）

| # | 任务 | 依赖 |
|---|------|------|
| 2.1 | `engine_physics`: rapier 集成 + ECS 同步 + 碰撞事件 | Phase 0 |
| 2.2 | `engine_audio`: kira 集成 + 空间音频 | Phase 0 |
| 2.3 | `engine_animation`: 关键帧 + AnimationPlayer | Phase 0 |
| 2.4 | `engine_animation`: 骨骼动画 + CPU skinning | 2.3, 1.4 |
| 2.5 | `engine_script`: rhai 集成 + 引擎 API 绑定 + 热重载 | Phase 0 |
| 2.6 | `engine_2d`: Sprite + SpriteSheet + 批处理 | Phase 0 |
| 2.7 | 编辑器扩展: 物理调试 + 音频预览 + 脚本控制台 | 2.1-2.6 |

**里程碑**: 可制作完整的简单 3D 游戏。

### Phase 3: 高级功能（3-4 周）

| # | 任务 | 依赖 |
|---|------|------|
| 3.1 | `engine_network`: quinn QUIC + 状态同步 | Phase 0 |
| 3.2 | 动画混合树 + 状态机 | 2.3 |
| 3.3 | wgpu 增强: PBR + Shadow Map + SSAO | 1.1 |
| 3.4 | TileMap + 2D 物理桥接 | 2.1, 2.6 |
| 3.5 | 编辑器: Undo/Redo + 3D Gizmo + 动画编辑器 | Phase 2 |
| 3.6 | Compute Shader skinning (wgpu) | 2.4, 1.1 |

### Phase 4: 打磨 + 文档（2-3 周）

| # | 任务 |
|---|------|
| 4.1 | puffin 性能分析面板集成 |
| 4.2 | 完整 API 文档（中英文） |
| 4.3 | 教程: 从零构建一个简单游戏 |
| 4.4 | 示例项目 (3D FPS + 2D 平台跳跃) |
| 4.5 | CI: 代码覆盖率 + 基准回归测试 |

## 17. 与现有设计的关系

本文档**替代**现有 `2026-03-18-engine-editor-design.md` 中的第二期~第七期路线图。
现有第一期（编辑器原型）的成果**全部保留**，作为 Phase 0 的起点。

## 18. 成功标准

整体重设计完成的判定标准:
1. 所有 crate `cargo build --workspace` 编译通过
2. 现有测试 + 新增测试全部通过
3. `cargo clippy --workspace -- -D warnings` 零警告
4. 编辑器可在 SW/wgpu 间切换渲染
5. 可保存/加载场景
6. 物理、音频、脚本子系统基本功能正常
7. 至少一个完整的示例游戏项目

---

## 附录 A: 架构审阅记录

### A.1 Oracle 审阅（高推理模型）

**审阅日期**: 2026-03-18
**审阅状态**: 已完成

Oracle 识别了 7 个关键架构问题及修复建议：

#### 问题 1: AssetManager 层级依赖违规 [严重]

**问题**: `AssetManager` 在 Layer 2 (`engine_scene`)，但 Layer 3 的 Feature crate（动画、音频）需要加载资产。按分层规则 Layer 3 只能依赖 Layer 1，这导致依赖违规。

**修复**: 将 `AssetManager` 的核心 trait 和 `Handle<T>` 类型定义移至 Layer 1 (`engine_core`)。具体资产加载器（OBJ、glTF）保留在 `engine_scene`。

```
engine_core (Layer 1):
  - AssetId, Handle<T>, AssetLoader trait, AssetManager trait

engine_scene (Layer 2):
  - ObjLoader, GltfLoader 等具体实现
  - 完整的 AssetManager 实现
```

**状态**: ✅ 已采纳 — 需要调整 Section 2.1 和 Section 3

#### 问题 2: RenderBackend FrameData 无状态设计损害 wgpu 性能 [严重]

**问题**: 每帧构建 `FrameData` 的无状态方式适合软件渲染器，但阻止了 wgpu 的 Bind Group 和 Pipeline State 跨帧复用，导致严重性能问题。

**修复**: `FrameData` 中的 `DrawCall` 应使用持久化资源句柄（`MeshHandle`, `MaterialHandle`）而非完整数据。wgpu 后端内部维护资源缓存，仅在句柄首次出现或资源变更时创建 GPU 对象。

```rust
// 修改后的 DrawCall
pub struct DrawCall {
    pub mesh: MeshHandle,        // 持久句柄，非数据
    pub material: MaterialHandle, // 持久句柄，非数据
    pub transform: Mat4,
}

// wgpu 后端内部缓存
struct WgpuRenderer {
    pipeline_cache: HashMap<MaterialHandle, wgpu::RenderPipeline>,
    bind_group_cache: HashMap<MaterialHandle, wgpu::BindGroup>,
    mesh_cache: HashMap<MeshHandle, GpuMesh>,
    // ...
}
```

**状态**: ✅ 已采纳 — DrawCall 设计已使用句柄（原设计本身已正确），需在 Section 6 明确 wgpu 缓存策略

#### 问题 3: 缺少 Input 系统和 Game UI 系统 [中等]

**问题**: 设计中未定义独立的输入系统（键盘/鼠标/手柄状态管理）和游戏内 UI 系统（不同于编辑器的 egui）。

**修复**: 
- 新增 `engine_input` crate (Layer 1)：统一的输入状态管理，支持键盘/鼠标/手柄，与窗口后端解耦
- Game UI 可暂时复用 egui（egui 可同时用于编辑器和游戏内 UI），后续迭代考虑专用方案

**状态**: ✅ 已采纳 — 新增 engine_input 到 Layer 1

#### 问题 4: hecs 缺少 OnRemove 钩子导致资源泄漏 [严重]

**问题**: 当 Entity 被 despawn 时，`hecs` 不会通知外部系统。`rapier` 的 `RigidBodyHandle` 和 `wgpu` 的 GPU Buffer 等外部资源无法被自动清理，导致内存泄漏。

**修复**: 在 `engine_core` 中实现 Entity 销毁事件队列：

```rust
// engine_core/src/despawn.rs
pub struct DespawnQueue {
    pending: Vec<Entity>,
}

// 使用方式：每帧在 PostUpdate 之前，各子系统订阅 DespawnEvent 清理资源
// PhysicsPlugin: 移除 RigidBody handle
// RenderPlugin: 释放 GPU buffer
```

**状态**: ✅ 已采纳 — 新增 despawn 事件机制到 engine_core

#### 问题 5: Plugin 初始化顺序缺乏保证 [中等]

**问题**: 多个 Plugin 可能依赖彼此注入的 Resource。当前 Plugin trait 无法表达初始化顺序约束。

**修复**: 增加 `Plugin::dependencies()` 方法：

```rust
pub trait Plugin: Send + Sync + 'static {
    fn name(&self) -> &str;
    fn dependencies(&self) -> Vec<&str> { vec![] }  // 默认无依赖
    fn build(&self, app: &mut AppBuilder);
    fn cleanup(&self, _app: &mut AppBuilder) {}
}
```

AppBuilder 在调用 `build()` 前进行拓扑排序。

**状态**: ✅ 已采纳

#### 问题 6: quinn (QUIC) 与同步游戏循环不兼容 [严重]

**问题**: `quinn` 基于 `tokio` 异步运行时。将 async QUIC 集成到同步的 Phase-based 游戏循环中会引入巨大的复杂度（tokio Runtime 桥接、跨线程状态同步）。

**修复**: 
- 方案 A: 替换 `quinn` 为 `renet`（同步的可靠 UDP 网络库，专为游戏设计）
- 方案 B: 替换为 `std::net::UdpSocket` + 自建可靠层
- 方案 C: 保留 `quinn`，但在独立线程运行 tokio runtime，通过 channel 与游戏循环通信

**推荐方案 A (`renet`)**: 同步 API，专为游戏设计，支持可靠/不可靠通道，无 async 依赖。

**状态**: ✅ 已采纳 — 将 quinn 替换为 renet

#### 问题 7: Phase 2 路线图过大，需拆分 [中等]

**问题**: Phase 2 包含 7 个大任务（物理、音频、动画、骨骼、脚本、2D、编辑器扩展），对单人开发不现实。

**修复**: 
- 将基础物理移到 Phase 1（与 Transform 层级一起验证）
- Phase 2 拆分为 2a（物理+音频）和 2b（动画+脚本+2D）
- 编辑器扩展移到各子系统实现之后的独立步骤

**状态**: ✅ 已采纳

### A.2 Oracle 额外警告

| 警告 | 详情 | 缓解措施 |
|------|------|---------|
| Rhai 脚本性能 | 每 Entity 每帧执行 Rhai 脚本会严重影响帧时间 | 限制脚本用于状态机/事件回调，非逐帧逻辑 |
| 串行调度瓶颈 | CPU skinning 和物理 step 无法并行 | 教育定位可接受，后续迭代可加 rayon 并行 |
| kira 音频锁竞争 | AudioManager 放入 ResourceMap 会导致 Mutex 竞争 | 使用 kira 的 CommandSender 模式，避免直接持有锁 |

### A.3 第二模型审阅 (Deep 深度推理模型)

**审阅模型**: Deep (深度推理模型，不同于 Oracle)
**审阅日期**: 2026-03-18
**审阅状态**: ✅ 已完成

Deep 模型从不同角度提出了 8 个问题，部分与 Oracle 重叠但深度更强：

#### D1: hecs + 自建中间件 ≈ 劣质版 bevy_ecs [中等]

**分析**: 在 hecs 之上自建 Schedule + EventBus + ResourceMap + Plugin 约需 750-1400 行代码。bevy_ecs 独立使用（不含 Bevy 引擎）约 15K LOC，但提供：
- 变更检测 (`Changed<T>`, `Added<T>`)
- 并行调度器
- `RemovedComponents<T>` (解决问题 #4)
- 经过生产验证的事件系统
- Plugin trait

**权衡**:
- 用 bevy_ecs: 省 ~12K LOC 自建代码，增 ~150KB 二进制大小，降低教育透明度
- 用 hecs: 保留教育价值，但承担自建中间件的 bug 风险

**决策**: 保持 hecs（教育定位核心价值），但接受以下简化:
- `EventBus<T>` 可用 `shrev` crate (200 LOC ring-buffer 事件通道)
- `ResourceMap` 可用 `anymap2` crate (80 LOC)
- 明确文档说明：串行调度是有意限制

#### D2: RenderBackend 需要 Resource Broker 生命周期 API [严重]

**分析**: 比 Oracle 的建议更深入。Deep 提出 **Resource Broker Pattern**:

```rust
pub trait RenderBackend {
    // 生命周期 API（调用一次，返回句柄）
    fn create_mesh(&mut self, data: &MeshData) -> RenderHandle<Self::Mesh>;
    fn create_texture(&mut self, data: &TextureData) -> RenderHandle<Self::Texture>;
    fn create_pipeline(&mut self, desc: &PipelineDesc) -> RenderHandle<Self::Pipeline>;
    fn destroy_mesh(&mut self, handle: RenderHandle<Self::Mesh>);

    // 每帧 API（引用句柄，不传数据）
    fn begin_frame(&mut self);
    fn submit_draw(&mut self, cmd: &DrawCommand);  // DrawCommand 引用句柄
    fn end_frame(&mut self);
}

/// 分代索引 — 软件渲染和 wgpu 通用
pub struct RenderHandle<T> {
    index: u32,
    generation: u32,
    _marker: PhantomData<T>,
}
```

这是 bgfx/sokol 等生产渲染器的标准做法。`FrameData` 应替换为 `DrawCommand` 引用持久句柄。

**状态**: ✅ 已采纳 — 替换原有 FrameData 设计为 Resource Broker Pattern

#### D3: Transform 必须在 Layer 1 [严重]

**分析**: `Transform` 被物理、音频（AudioListener 位置）、动画、渲染、脚本全部使用。它不是"场景"概念，而是核心组件。

**修复**: 将 `Transform` 和 `Parent`/`Children`/`GlobalTransform` 移至 `engine_core`。

```rust
// engine_core/src/transform.rs
pub struct Transform {
    pub translation: Vec3,
    pub rotation: Quat,
    pub scale: Vec3,
}

pub struct Parent(pub Entity);
pub struct Children(pub Vec<Entity>);
pub struct GlobalTransform(pub Mat4);  // 由 propagate_transforms 系统计算
```

**状态**: ✅ 已采纳

#### D4: 范围严重超标 — 需要 30-42 周，只有 14-20 周 [严重]

**Deep 模型的详细工期估算**:

| Crate | 预估周数 |
|-------|---------|
| engine_core | 2-3 |
| engine_math | 0.5 |
| engine_render_api | 1 |
| engine_render_sw (迁移) | 1 |
| engine_render_wgpu | 4-6 |
| engine_scene | 2 |
| engine_physics | 2-3 |
| engine_audio | 1-1.5 |
| engine_animation | 3-4 |
| engine_script | 2 |
| engine_network | 4-6 |
| engine_2d | 2 |
| engine_app | 1 |
| engine_editor | 4-6 |
| **总计** | **30-42** |

**推荐 MVP 裁剪方案** (7 个核心 crate, ~12-16 周):

| 保留 (MVP) | 裁剪 (Post-MVP) | 原因 |
|-----------|----------------|------|
| engine_core | — | 基础 |
| engine_math | — | 简单 |
| engine_render_api | — | 基础 |
| engine_render_sw | — | 已有 |
| engine_scene | — | 必需 |
| engine_app | — | 必需 |
| engine_physics | — | 高演示价值 |
| engine_editor (精简版) | — | 必需但精简 |
| — | engine_render_wgpu | 单独 4-6 周，v2 再加 |
| — | engine_audio | 可用 rodio 20 行替代 |
| — | engine_animation | 骨骼动画是 3-4 周深坑 |
| — | engine_script | 教育目标不强制 |
| — | engine_network | 多人网络等于另一个引擎 |
| — | engine_2d | 聚焦 3D |

**状态**: ⚠️ 建议采纳 — 需用户确认裁剪范围

#### D5: rapier 句柄泄漏的详细修复 [高]

Deep 提供了比 Oracle 更具体的修复方案 — `sync_removals()` 每帧扫描:

```rust
impl PhysicsWorld {
    pub fn sync_removals(&mut self, world: &World) {
        let orphans: Vec<_> = self.entity_to_body.iter()
            .filter(|(entity, _)| !world.contains(**entity))
            .map(|(_, handle)| *handle)
            .collect();

        for handle in orphans {
            self.rapier_bodies.remove(handle, ...);
        }
    }
}
```

10K Entity 场景的开销: ~50μs/帧。可接受。

**状态**: ✅ 已采纳

#### D6: 缺少 Transform 层级系统 [严重]

**分析**: 当前 Transform 是扁平的。没有 Parent-Child 关系，以下功能无法工作:
- 武器挂载到手 → 手连接到手臂（动画）
- 子物体跟随父物体移动（物理）
- 编辑器场景树的层级显示

**修复**: 实现 `propagate_transforms` 系统:
```rust
// PostUpdate 阶段运行
fn propagate_transforms(world: &World) {
    // 遍历 Transform 层级树
    // GlobalTransform = parent.GlobalTransform * local.Transform.to_mat4()
}
```

**状态**: ✅ 已采纳 — 纳入 D3 的 Transform 层级设计

#### D7: 缺少窗口管理抽象 [中等]

**分析**: 谁创建窗口？谁分发事件？`winit` 应由 `engine_app` 拥有，分发事件到 Input、Renderer (resize)、Editor。

**状态**: ✅ 已采纳 — engine_app 负责 winit 事件循环

#### D8: 缺少错误恢复策略 [中等]

**分析**: 模型文件缺失时 panic 不可接受。资产加载应:
- `AssetManager::load` 立即返回 `Handle<T>`（始终成功）
- 后台异步加载
- 失败时返回 fallback 资源（紫色纹理 / 单位立方体网格）
- 记录错误日志，永不 panic

**状态**: ✅ 已采纳

### A.4 两个模型审阅汇总对比

| 问题领域 | Oracle (高推理模型) | Deep (深度推理模型) | 共识 |
|---------|-------------------|-------------------|------|
| AssetManager 层级 | 移至 Layer 1 | 移至 Layer 1 + Trait 反转 | ✅ 一致 |
| RenderBackend | 句柄缓存 | Resource Broker Pattern (更详细) | ✅ Deep 方案更优 |
| ECS 选型 | hecs OK | hecs 存在风险，建议 bevy_ecs | ⚠️ 分歧 — 保留 hecs + 补充 crate |
| 网络选型 | renet 替换 quinn | quinn 可用但建议裁剪 | ⚠️ 分歧 — 采用裁剪方案 |
| 范围可行性 | Phase 2 拆分 | 裁剪到 7 个 crate | ⚠️ Deep 更激进 |
| Plugin 顺序 | dependencies() | 未提及 | Oracle 独有 |
| Transform 层级 | 未提及 | 必须有 Parent/Children | Deep 独有 |
| 错误恢复 | 未提及 | fallback 资源模式 | Deep 独有 |
| 输入系统 | 缺失 | 缺失 (更详细方案) | ✅ 一致 |
| rapier 泄漏 | DespawnQueue | sync_removals() (更详细) | ✅ Deep 方案更具体 |
