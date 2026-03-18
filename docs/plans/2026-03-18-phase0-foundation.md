# Phase 0: 基础重构 — 实施计划

> **For agentic workers:** REQUIRED: Use superpowers:subagent-driven-development (if subagents available) or superpowers:executing-plans to implement this plan. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 将现有 4-crate 编辑器原型重构为模块化游戏引擎基础架构。替换自写 ECS 为 hecs，新建 engine_math/engine_render_api/engine_input 基础 crate，将 engine_renderer 迁移为 engine_render_sw 并适配 RenderBackend trait，更新 engine_scene 和 engine_editor 适配新 API。

**Architecture:** 分层架构。Layer 1 基础层（engine_core, engine_math, engine_render_api, engine_input）互不依赖；Layer 2 桥接层（engine_render_sw, engine_scene）依赖 Layer 1；Layer 4 应用层（engine_editor）依赖所有下层。上层可依赖下层，绝不反向。

**Tech Stack:** Rust, hecs 0.10, glam 0.29, rayon, eframe/egui 0.31, rfd, tobj, image, thiserror, tracing

**Spec:** `docs/specs/2026-03-18-engine-architecture-redesign.md`

---

## 文件结构

### 新建文件

```
crates/
├── engine_math/
│   ├── Cargo.toml
│   └── src/
│       ├── lib.rs              # glam re-exports + 模块声明
│       ├── aabb.rs             # 轴对齐包围盒
│       ├── ray.rs              # 射线
│       └── frustum.rs          # 视锥体
├── engine_render_api/
│   ├── Cargo.toml
│   └── src/
│       ├── lib.rs              # re-exports
│       ├── backend.rs          # RenderBackend trait
│       ├── frame.rs            # FrameData, DrawCall, CameraData, LightData, FrameOutput
│       ├── mesh.rs             # MeshData, MeshHandle
│       ├── texture.rs          # TextureData, TextureHandle, TextureFormat
│       ├── material.rs         # MaterialData, MaterialHandle
│       ├── color.rs            # Color (统一 RGBA 类型)
│       └── viewport.rs         # Viewport, RenderConfig
├── engine_input/
│   ├── Cargo.toml
│   └── src/
│       ├── lib.rs              # InputPlugin + re-exports
│       ├── keyboard.rs         # KeyboardState
│       ├── mouse.rs            # MouseState
│       └── input_state.rs      # InputState (聚合)
```

### 重命名/重写文件

```
crates/engine_renderer/ → crates/engine_render_sw/    # 目录重命名
crates/engine_render_sw/src/lib.rs                    # 更新 crate 名引用
crates/engine_render_sw/src/adapter.rs                # 新增: RenderBackend 适配器

crates/engine_core/src/lib.rs          # 重写: 新模块声明
crates/engine_core/src/ecs.rs          # 重写: hecs 包装
crates/engine_core/src/time.rs         # 保留, 小调整
crates/engine_core/src/plugin.rs       # 新增: Plugin trait
crates/engine_core/src/app.rs          # 新增: AppBuilder + 主循环
crates/engine_core/src/event.rs        # 新增: EventBus
crates/engine_core/src/schedule.rs     # 新增: Phase + Schedule + System
crates/engine_core/src/resource.rs     # 新增: ResourceMap
crates/engine_core/src/transform.rs    # 新增: Transform + Parent + Children + GlobalTransform
crates/engine_core/src/despawn.rs      # 新增: DespawnQueue
crates/engine_core/src/asset.rs        # 新增: AssetId, Handle<T>, AssetLoader trait

crates/engine_scene/src/lib.rs         # 重写: 适配 hecs
crates/engine_scene/src/scene.rs       # 重写: 适配新 World API
crates/engine_scene/src/asset.rs       # 重写: 使用 engine_core 的 Handle<T>
crates/engine_scene/src/components.rs  # 重写: 移除 Transform(已移至 core), 保留 MeshRenderer 等

crates/engine_editor/src/app.rs        # 更新: 适配新 API
crates/engine_editor/src/render_bridge.rs  # 更新: 使用 engine_render_sw
```

### 保留不变

```
crates/engine_render_sw/src/renderer.rs      # 内部渲染逻辑不变
crates/engine_render_sw/src/renderers/*      # 4 种渲染策略不变
crates/engine_render_sw/src/shader/*         # 着色器不变
crates/engine_render_sw/src/rasterizer.rs    # 光栅化器不变
crates/engine_render_sw/src/model.rs         # 模型加载不变
crates/engine_render_sw/src/buffer.rs        # 缓冲不变
crates/engine_render_sw/src/triple_buffer.rs # 三缓冲不变
crates/engine_render_sw/src/vertex.rs        # 顶点不变
crates/engine_render_sw/src/fragment.rs      # 片段不变
crates/engine_render_sw/src/uniform.rs       # Uniform 不变
crates/engine_render_sw/src/material.rs      # 材质不变
crates/engine_render_sw/src/face.rs          # 面不变
crates/engine_render_sw/src/light.rs         # 光源不变
crates/engine_render_sw/src/math.rs          # 数学不变
crates/engine_render_sw/src/error.rs         # 错误不变
crates/engine_render_sw/src/color.rs         # 颜色不变（engine_render_api 的 Color 是独立类型）
crates/engine_render_sw/tests/*              # 全部 207 个测试保留
system_test/                                 # 保留但从 workspace 移除
```

---

## Chunk 1: engine_math — 数学工具 crate

### Task 1: 创建 engine_math crate

**Files:**
- Create: `crates/engine_math/Cargo.toml`
- Create: `crates/engine_math/src/lib.rs`
- Create: `crates/engine_math/src/aabb.rs`
- Create: `crates/engine_math/src/ray.rs`
- Create: `crates/engine_math/src/frustum.rs`
- Modify: `Cargo.toml` (workspace root)

- [ ] **Step 1: 创建 Cargo.toml**

```toml
# crates/engine_math/Cargo.toml
[package]
name = "engine_math"
version = "0.1.0"
edition = "2021"
rust-version = "1.73"

[dependencies]
glam = "0.29"
```

- [ ] **Step 2: 写 AABB 测试**

```rust
// crates/engine_math/src/aabb.rs
#[cfg(test)]
mod tests {
    use super::*;
    use glam::Vec3;

    #[test]
    fn aabb_contains_point() {
        let aabb = AABB::new(Vec3::ZERO, Vec3::ONE);
        assert!(aabb.contains(Vec3::splat(0.5)));
        assert!(!aabb.contains(Vec3::splat(2.0)));
    }

    #[test]
    fn aabb_intersects_other() {
        let a = AABB::new(Vec3::ZERO, Vec3::ONE);
        let b = AABB::new(Vec3::splat(0.5), Vec3::splat(1.5));
        let c = AABB::new(Vec3::splat(2.0), Vec3::splat(3.0));
        assert!(a.intersects(&b));
        assert!(!a.intersects(&c));
    }
}
```

- [ ] **Step 3: 实现 AABB**

```rust
// crates/engine_math/src/aabb.rs
use glam::Vec3;

/// 轴对齐包围盒
#[derive(Debug, Clone, Copy, PartialEq)]
pub struct AABB {
    pub min: Vec3,
    pub max: Vec3,
}

impl AABB {
    pub fn new(min: Vec3, max: Vec3) -> Self { Self { min, max } }
    pub fn contains(&self, point: Vec3) -> bool { ... }
    pub fn intersects(&self, other: &AABB) -> bool { ... }
    pub fn center(&self) -> Vec3 { (self.min + self.max) * 0.5 }
    pub fn size(&self) -> Vec3 { self.max - self.min }
}
```

- [ ] **Step 4: 写 Ray 测试并实现**

```rust
// crates/engine_math/src/ray.rs
use glam::Vec3;
use crate::aabb::AABB;

/// 射线
#[derive(Debug, Clone, Copy)]
pub struct Ray {
    pub origin: Vec3,
    pub direction: Vec3,
}

impl Ray {
    pub fn new(origin: Vec3, direction: Vec3) -> Self { ... }
    pub fn point_at(&self, t: f32) -> Vec3 { self.origin + self.direction * t }
    pub fn intersects_aabb(&self, aabb: &AABB) -> Option<f32> { ... }  // slab method
}
```

- [ ] **Step 5: 写 Frustum 骨架并实现**

```rust
// crates/engine_math/src/frustum.rs
use glam::{Vec3, Vec4, Mat4};
use crate::aabb::AABB;

/// 平面 (法线 + 距离)
#[derive(Debug, Clone, Copy)]
pub struct Plane {
    pub normal: Vec3,
    pub distance: f32,
}

/// 视锥体 (6 个平面)
#[derive(Debug, Clone)]
pub struct Frustum {
    pub planes: [Plane; 6],
}

impl Frustum {
    pub fn from_view_projection(vp: &Mat4) -> Self { ... }
    pub fn contains_point(&self, point: Vec3) -> bool { ... }
    pub fn intersects_aabb(&self, aabb: &AABB) -> bool { ... }
}
```

- [ ] **Step 6: 写 lib.rs re-exports**

```rust
// crates/engine_math/src/lib.rs
// Copyright (c) Simple-XX/SimpleRenderer
// glam 数学库 re-export + 引擎几何工具
pub use glam::*;

pub mod aabb;
pub mod ray;
pub mod frustum;

pub use aabb::AABB;
pub use ray::Ray;
pub use frustum::{Frustum, Plane};
```

- [ ] **Step 7: 添加到 workspace 并验证**

在 workspace root `Cargo.toml` 的 members 中添加 `"crates/engine_math"`。

```bash
cargo test -p engine_math
cargo clippy -p engine_math -- -D warnings
```

Expected: 全部测试通过，零 clippy 警告。

- [ ] **Step 8: Commit**

---

## Chunk 2: engine_core — 核心层重写

### Task 2: 添加 hecs 依赖，重写 ECS 层

**Files:**
- Modify: `crates/engine_core/Cargo.toml`
- Rewrite: `crates/engine_core/src/ecs.rs`

- [ ] **Step 1: 更新 Cargo.toml**

```toml
[package]
name = "engine_core"
version = "0.1.0"
edition = "2021"
rust-version = "1.73"

[dependencies]
glam = "0.29"
hecs = "0.10"
```

- [ ] **Step 2: 写 hecs 包装的测试**

复用现有测试模式，但适配 hecs API。hecs 不需要 `Component` marker trait（hecs 组件只需 `Send + Sync + 'static`），但我们保留一个空 marker trait 以保持教育可读性和 API 一致性。

```rust
#[cfg(test)]
mod tests {
    use super::*;

    #[derive(Debug, PartialEq)]
    struct Position { x: f32, y: f32 }

    #[derive(Debug, PartialEq)]
    struct Velocity { dx: f32, dy: f32 }

    #[test]
    fn spawn_and_get_component() {
        let mut world = World::new();
        let e = world.spawn((Position { x: 1.0, y: 2.0 },));
        let pos = world.get::<&Position>(e).unwrap();
        assert_eq!(pos.x, 1.0);
    }

    #[test]
    fn query_components() {
        let mut world = World::new();
        world.spawn((Position { x: 0.0, y: 0.0 },));
        world.spawn((Position { x: 1.0, y: 1.0 }, Velocity { dx: 1.0, dy: 0.0 }));
        let count = world.query::<&Position>().iter().count();
        assert_eq!(count, 2);
    }

    #[test]
    fn despawn_entity() {
        let mut world = World::new();
        let e = world.spawn((Position { x: 0.0, y: 0.0 },));
        world.despawn(e).unwrap();
        assert!(world.get::<&Position>(e).is_err());
    }
}
```

- [ ] **Step 3: 重写 ecs.rs — hecs 薄包装**

```rust
// crates/engine_core/src/ecs.rs
// 保留 Entity 作为 hecs::Entity 的 re-export
pub use hecs::Entity;
pub use hecs::World;
// hecs 的 World 已提供 spawn, despawn, get, query 等方法
// 无需额外包装
```

注意: hecs 的 `Entity` 是一个 opaque 类型（不是 u64），`spawn` 接受组件元组，`query` 返回组件引用。这与现有 API 有显著差异，engine_scene 需要相应更新。

- [ ] **Step 4: 验证 ECS 测试**

```bash
cargo test -p engine_core
```

- [ ] **Step 5: Commit**

---

### Task 3: 实现 ResourceMap + EventBus

**Files:**
- Create: `crates/engine_core/src/resource.rs`
- Create: `crates/engine_core/src/event.rs`

- [ ] **Step 1: 写 ResourceMap 测试**

```rust
#[cfg(test)]
mod tests {
    use super::*;

    struct FpsCounter(f32);

    #[test]
    fn insert_and_get_resource() {
        let mut resources = ResourceMap::new();
        resources.insert(FpsCounter(60.0));
        assert_eq!(resources.get::<FpsCounter>().unwrap().0, 60.0);
    }

    #[test]
    fn get_mut_resource() {
        let mut resources = ResourceMap::new();
        resources.insert(FpsCounter(30.0));
        resources.get_mut::<FpsCounter>().unwrap().0 = 60.0;
        assert_eq!(resources.get::<FpsCounter>().unwrap().0, 60.0);
    }

    #[test]
    fn missing_resource_returns_none() {
        let resources = ResourceMap::new();
        assert!(resources.get::<FpsCounter>().is_none());
    }
}
```

- [ ] **Step 2: 实现 ResourceMap**

```rust
// crates/engine_core/src/resource.rs
use std::any::{Any, TypeId};
use std::collections::HashMap;

pub struct ResourceMap {
    map: HashMap<TypeId, Box<dyn Any + Send + Sync>>,
}

impl ResourceMap {
    pub fn new() -> Self { Self { map: HashMap::new() } }
    pub fn insert<T: Send + Sync + 'static>(&mut self, resource: T) { ... }
    pub fn get<T: Send + Sync + 'static>(&self) -> Option<&T> { ... }
    pub fn get_mut<T: Send + Sync + 'static>(&mut self) -> Option<&mut T> { ... }
    pub fn contains<T: Send + Sync + 'static>(&self) -> bool { ... }
    pub fn remove<T: Send + Sync + 'static>(&mut self) -> Option<T> { ... }
}
```

- [ ] **Step 3: 写 EventBus 测试**

```rust
#[cfg(test)]
mod tests {
    use super::*;

    #[derive(Debug, PartialEq)]
    struct Collision { entity_a: u64, entity_b: u64 }
    impl Event for Collision {}

    #[test]
    fn send_and_read_events() {
        let mut bus = EventBus::new();
        bus.send(Collision { entity_a: 1, entity_b: 2 });
        bus.send(Collision { entity_a: 3, entity_b: 4 });
        let events: Vec<&Collision> = bus.read::<Collision>().collect();
        assert_eq!(events.len(), 2);
    }

    #[test]
    fn clear_removes_events() {
        let mut bus = EventBus::new();
        bus.send(Collision { entity_a: 1, entity_b: 2 });
        bus.clear_all();
        let events: Vec<&Collision> = bus.read::<Collision>().collect();
        assert_eq!(events.len(), 0);
    }
}
```

- [ ] **Step 4: 实现 EventBus**

```rust
// crates/engine_core/src/event.rs
use std::any::{Any, TypeId};
use std::collections::HashMap;

pub trait Event: Send + Sync + 'static {}

pub struct EventBus {
    channels: HashMap<TypeId, Box<dyn Any + Send + Sync>>,
}

impl EventBus {
    pub fn new() -> Self { ... }
    pub fn send<E: Event>(&mut self, event: E) { ... }
    pub fn read<E: Event>(&self) -> impl Iterator<Item = &E> { ... }
    pub fn clear_all(&mut self) { ... }
}
```

- [ ] **Step 5: 验证测试**

```bash
cargo test -p engine_core
```

- [ ] **Step 6: Commit**

---

### Task 4: 实现 Schedule + System trait + Phase

**Files:**
- Create: `crates/engine_core/src/schedule.rs`

- [ ] **Step 1: 写 Schedule 测试**

```rust
#[cfg(test)]
mod tests {
    use super::*;
    use std::sync::{Arc, Mutex};

    #[test]
    fn systems_run_in_phase_order() {
        let log = Arc::new(Mutex::new(Vec::new()));
        let mut schedule = Schedule::new();

        let log1 = log.clone();
        schedule.add_system(Phase::Update, move |_ctx: &mut SystemContext| {
            log1.lock().unwrap().push("update");
        });
        let log2 = log.clone();
        schedule.add_system(Phase::PreUpdate, move |_ctx: &mut SystemContext| {
            log2.lock().unwrap().push("pre_update");
        });

        let mut world = hecs::World::new();
        let mut resources = ResourceMap::new();
        let mut events = EventBus::new();
        schedule.run(&mut world, &mut resources, &mut events);

        let result = log.lock().unwrap();
        assert_eq!(*result, vec!["pre_update", "update"]);
    }
}
```

- [ ] **Step 2: 实现 Phase + Schedule + System**

```rust
// crates/engine_core/src/schedule.rs

#[derive(Debug, Clone, Copy, PartialEq, Eq, PartialOrd, Ord, Hash)]
pub enum Phase {
    PreUpdate,
    Update,
    PostUpdate,
    PreRender,
    Render,
    PostRender,
}

pub struct SystemContext<'a> {
    pub world: &'a mut hecs::World,
    pub resources: &'a mut ResourceMap,
    pub events: &'a mut EventBus,
}

// System 使用闭包，FnMut(&mut SystemContext)
type BoxedSystem = Box<dyn FnMut(&mut SystemContext) + Send>;

pub struct Schedule {
    systems: Vec<(Phase, BoxedSystem)>,
}

impl Schedule {
    pub fn new() -> Self { ... }
    pub fn add_system(&mut self, phase: Phase, system: impl FnMut(&mut SystemContext) + Send + 'static) { ... }
    pub fn run(&mut self, world: &mut hecs::World, resources: &mut ResourceMap, events: &mut EventBus) {
        // 按 Phase 排序，同 Phase 内按注册顺序串行执行
    }
}
```

- [ ] **Step 3: 验证测试**

```bash
cargo test -p engine_core
```

- [ ] **Step 4: Commit**

---

### Task 5: 实现 Plugin trait + AppBuilder

**Files:**
- Create: `crates/engine_core/src/plugin.rs`
- Create: `crates/engine_core/src/app.rs`

- [ ] **Step 1: 写 Plugin + AppBuilder 测试**

```rust
#[cfg(test)]
mod tests {
    use super::*;

    struct TestResource(i32);

    struct TestPlugin;
    impl Plugin for TestPlugin {
        fn name(&self) -> &str { "test" }
        fn build(&self, app: &mut AppBuilder) {
            app.insert_resource(TestResource(42));
        }
    }

    #[test]
    fn plugin_registers_resource() {
        let mut app = AppBuilder::new();
        app.add_plugin(TestPlugin);
        app.build_plugins();
        assert_eq!(app.resources.get::<TestResource>().unwrap().0, 42);
    }
}
```

- [ ] **Step 2: 实现 Plugin trait**

```rust
// crates/engine_core/src/plugin.rs
pub trait Plugin: Send + Sync + 'static {
    fn name(&self) -> &str;
    fn dependencies(&self) -> Vec<&str> { vec![] }
    fn build(&self, app: &mut AppBuilder);
    fn cleanup(&self, _app: &mut AppBuilder) {}
}
```

- [ ] **Step 3: 实现 AppBuilder**

```rust
// crates/engine_core/src/app.rs
pub struct AppBuilder {
    pub world: hecs::World,
    pub resources: ResourceMap,
    pub events: EventBus,
    pub schedule: Schedule,
    plugins: Vec<Box<dyn Plugin>>,
}

impl AppBuilder {
    pub fn new() -> Self { ... }
    pub fn add_plugin(&mut self, plugin: impl Plugin) -> &mut Self { ... }
    pub fn add_system(&mut self, phase: Phase, system: impl FnMut(&mut SystemContext) + Send + 'static) -> &mut Self { ... }
    pub fn insert_resource<T: Send + Sync + 'static>(&mut self, resource: T) -> &mut Self { ... }
    pub fn build_plugins(&mut self) { ... }  // 拓扑排序 + 调用 build
    pub fn run_once(&mut self) { ... }  // 执行一帧
}
```

- [ ] **Step 4: 验证测试**

```bash
cargo test -p engine_core
```

- [ ] **Step 5: Commit**

---

### Task 6: 实现 Transform + DespawnQueue + Asset traits

**Files:**
- Create: `crates/engine_core/src/transform.rs`
- Create: `crates/engine_core/src/despawn.rs`
- Create: `crates/engine_core/src/asset.rs`

- [ ] **Step 1: 实现 Transform 组件 (移自 engine_scene)**

```rust
// crates/engine_core/src/transform.rs
use glam::{Vec3, Quat, Mat4};
use hecs::Entity;

#[derive(Debug, Clone)]
pub struct Transform {
    pub translation: Vec3,
    pub rotation: Quat,
    pub scale: Vec3,
}

impl Default for Transform {
    fn default() -> Self {
        Self { translation: Vec3::ZERO, rotation: Quat::IDENTITY, scale: Vec3::ONE }
    }
}

impl Transform {
    pub fn to_mat4(&self) -> Mat4 {
        Mat4::from_scale_rotation_translation(self.scale, self.rotation, self.translation)
    }
}

#[derive(Debug, Clone)]
pub struct Parent(pub Entity);

#[derive(Debug, Clone, Default)]
pub struct Children(pub Vec<Entity>);

#[derive(Debug, Clone, Default)]
pub struct GlobalTransform(pub Mat4);
```

- [ ] **Step 2: 实现 DespawnQueue**

```rust
// crates/engine_core/src/despawn.rs
use hecs::Entity;

pub struct DespawnQueue {
    pending: Vec<Entity>,
}

impl DespawnQueue {
    pub fn new() -> Self { Self { pending: Vec::new() } }
    pub fn mark(&mut self, entity: Entity) { self.pending.push(entity); }
    pub fn drain(&mut self) -> Vec<Entity> { std::mem::take(&mut self.pending) }
    pub fn is_empty(&self) -> bool { self.pending.is_empty() }
}
```

- [ ] **Step 3: 实现 Asset 基础 traits**

```rust
// crates/engine_core/src/asset.rs
use std::marker::PhantomData;
use std::path::Path;

pub type AssetId = u64;

#[derive(Debug, Clone, Copy, PartialEq, Eq, Hash)]
pub struct Handle<T> {
    id: AssetId,
    _marker: PhantomData<T>,
}

impl<T> Handle<T> {
    pub fn new(id: AssetId) -> Self { Self { id, _marker: PhantomData } }
    pub fn id(&self) -> AssetId { self.id }
}

/// 资产加载器 trait
pub trait AssetLoader: Send + Sync + 'static {
    type Asset: Send + Sync + 'static;
    fn extensions(&self) -> &[&str];
    fn load(&self, path: &Path) -> Result<Self::Asset, Box<dyn std::error::Error + Send + Sync>>;
}
```

- [ ] **Step 4: 更新 lib.rs re-exports**

```rust
// crates/engine_core/src/lib.rs
pub mod ecs;
pub mod time;
pub mod resource;
pub mod event;
pub mod schedule;
pub mod plugin;
pub mod app;
pub mod transform;
pub mod despawn;
pub mod asset;

// 核心 re-exports
pub use hecs::{Entity, World};
pub use time::Time;
pub use resource::ResourceMap;
pub use event::{Event, EventBus};
pub use schedule::{Phase, Schedule, SystemContext};
pub use plugin::Plugin;
pub use app::AppBuilder;
pub use transform::{Transform, Parent, Children, GlobalTransform};
pub use despawn::DespawnQueue;
pub use asset::{AssetId, Handle, AssetLoader};
```

- [ ] **Step 5: 验证全部 engine_core 测试**

```bash
cargo test -p engine_core
cargo clippy -p engine_core -- -D warnings
```

- [ ] **Step 6: Commit**

---

## Chunk 3: engine_render_api — 渲染抽象层

### Task 7: 创建 engine_render_api crate

**Files:**
- Create: `crates/engine_render_api/Cargo.toml`
- Create: `crates/engine_render_api/src/lib.rs`
- Create: `crates/engine_render_api/src/backend.rs`
- Create: `crates/engine_render_api/src/frame.rs`
- Create: `crates/engine_render_api/src/mesh.rs`
- Create: `crates/engine_render_api/src/texture.rs`
- Create: `crates/engine_render_api/src/material.rs`
- Create: `crates/engine_render_api/src/color.rs`
- Create: `crates/engine_render_api/src/viewport.rs`

- [ ] **Step 1: 创建 Cargo.toml**

```toml
[package]
name = "engine_render_api"
version = "0.1.0"
edition = "2021"
rust-version = "1.73"

[dependencies]
glam = "0.29"
thiserror = "2"
```

- [ ] **Step 2: 实现 Color**

```rust
// crates/engine_render_api/src/color.rs
/// 统一 RGBA 颜色类型 (渲染 API 层)
#[derive(Debug, Clone, Copy, PartialEq)]
pub struct Color {
    pub r: f32, pub g: f32, pub b: f32, pub a: f32,
}

impl Color {
    pub const WHITE: Self = Self { r: 1.0, g: 1.0, b: 1.0, a: 1.0 };
    pub const BLACK: Self = Self { r: 0.0, g: 0.0, b: 0.0, a: 1.0 };
    pub fn new(r: f32, g: f32, b: f32, a: f32) -> Self { Self { r, g, b, a } }
    pub fn to_rgba8(&self) -> [u8; 4] { ... }
}

impl Default for Color {
    fn default() -> Self { Self::BLACK }
}
```

- [ ] **Step 3: 实现 Viewport + RenderConfig**

```rust
// crates/engine_render_api/src/viewport.rs
pub struct Viewport {
    pub x: u32, pub y: u32, pub width: u32, pub height: u32,
}

pub struct RenderConfig {
    pub width: u32,
    pub height: u32,
    pub vsync: bool,
}
```

- [ ] **Step 4: 实现 mesh/texture/material 句柄类型**

```rust
// mesh.rs
#[derive(Debug, Clone, Copy, PartialEq, Eq, Hash)]
pub struct MeshHandle(pub u64);

pub struct MeshData {
    pub positions: Vec<glam::Vec3>,
    pub normals: Vec<glam::Vec3>,
    pub tex_coords: Vec<glam::Vec2>,
    pub indices: Vec<u32>,
}

// texture.rs
#[derive(Debug, Clone, Copy, PartialEq, Eq, Hash)]
pub struct TextureHandle(pub u64);

pub enum TextureFormat { Rgba8, Rgb8 }

pub struct TextureData {
    pub width: u32,
    pub height: u32,
    pub format: TextureFormat,
    pub data: Vec<u8>,
}

// material.rs
#[derive(Debug, Clone, Copy, PartialEq, Eq, Hash)]
pub struct MaterialHandle(pub u64);

pub struct MaterialData {
    pub diffuse_color: glam::Vec3,
    pub specular_color: glam::Vec3,
    pub ambient_color: glam::Vec3,
    pub shininess: f32,
    pub diffuse_texture: Option<TextureHandle>,
}
```

- [ ] **Step 5: 实现 FrameData + DrawCall**

```rust
// crates/engine_render_api/src/frame.rs
use glam::{Vec3, Mat4};
use crate::color::Color;
use crate::mesh::MeshHandle;
use crate::material::MaterialHandle;
use crate::viewport::Viewport;

pub struct CameraData {
    pub view: Mat4,
    pub projection: Mat4,
    pub position: Vec3,
    pub fov: f32,
    pub near: f32,
    pub far: f32,
}

#[derive(Debug, Clone, Copy)]
pub enum LightKind { Directional, Point, Spot }

pub struct LightData {
    pub kind: LightKind,
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

pub struct FrameData {
    pub camera: CameraData,
    pub lights: Vec<LightData>,
    pub draw_calls: Vec<DrawCall>,
    pub clear_color: Color,
    pub viewport: Viewport,
}

pub struct FrameOutput {
    pub frame_time_ms: f64,
    pub triangle_count: u32,
    pub draw_call_count: u32,
}
```

- [ ] **Step 6: 实现 RenderBackend trait**

```rust
// crates/engine_render_api/src/backend.rs
use crate::frame::{FrameData, FrameOutput};
use crate::mesh::{MeshData, MeshHandle};
use crate::texture::{TextureData, TextureHandle};
use crate::viewport::RenderConfig;

#[derive(Debug, thiserror::Error)]
pub enum RenderError {
    #[error("渲染初始化失败: {0}")]
    InitFailed(String),
    #[error("渲染错误: {0}")]
    RenderFailed(String),
    #[error("资源创建失败: {0}")]
    ResourceError(String),
}

pub type Result<T> = std::result::Result<T, RenderError>;

/// 渲染后端抽象
pub trait RenderBackend: Send + 'static {
    fn init(&mut self, config: RenderConfig) -> Result<()>;
    fn render_frame(&mut self, frame: &FrameData) -> Result<FrameOutput>;
    fn resize(&mut self, width: u32, height: u32);
    fn create_mesh(&mut self, data: &MeshData) -> Result<MeshHandle>;
    fn create_texture(&mut self, data: &TextureData) -> Result<TextureHandle>;
    fn read_pixels(&self) -> &[u8];
    fn name(&self) -> &str;
}
```

- [ ] **Step 7: 写 lib.rs re-exports**

```rust
// crates/engine_render_api/src/lib.rs
pub mod backend;
pub mod frame;
pub mod mesh;
pub mod texture;
pub mod material;
pub mod color;
pub mod viewport;

pub use backend::{RenderBackend, RenderError, Result};
pub use frame::*;
pub use mesh::{MeshData, MeshHandle};
pub use texture::{TextureData, TextureHandle, TextureFormat};
pub use material::{MaterialData, MaterialHandle};
pub use color::Color;
pub use viewport::{Viewport, RenderConfig};
```

- [ ] **Step 8: 添加到 workspace 并验证**

```bash
cargo build -p engine_render_api
cargo clippy -p engine_render_api -- -D warnings
```

- [ ] **Step 9: Commit**

---

## Chunk 4: engine_input — 输入系统

### Task 8: 创建 engine_input crate

**Files:**
- Create: `crates/engine_input/Cargo.toml`
- Create: `crates/engine_input/src/lib.rs`
- Create: `crates/engine_input/src/keyboard.rs`
- Create: `crates/engine_input/src/mouse.rs`
- Create: `crates/engine_input/src/input_state.rs`

- [ ] **Step 1: 创建 Cargo.toml**

```toml
[package]
name = "engine_input"
version = "0.1.0"
edition = "2021"
rust-version = "1.73"

[dependencies]
glam = "0.29"
```

- [ ] **Step 2: 实现 KeyboardState**

```rust
// crates/engine_input/src/keyboard.rs
use std::collections::HashSet;

#[derive(Debug, Clone, Copy, PartialEq, Eq, Hash)]
pub enum KeyCode {
    W, A, S, D, Q, E, Space, LShift, Escape,
    Up, Down, Left, Right,
    Key1, Key2, Key3, Key4,
    // 按需扩展
}

#[derive(Debug, Default)]
pub struct KeyboardState {
    pressed: HashSet<KeyCode>,
    just_pressed: HashSet<KeyCode>,
    just_released: HashSet<KeyCode>,
}

impl KeyboardState {
    pub fn is_pressed(&self, key: KeyCode) -> bool { self.pressed.contains(&key) }
    pub fn just_pressed(&self, key: KeyCode) -> bool { self.just_pressed.contains(&key) }
    pub fn just_released(&self, key: KeyCode) -> bool { self.just_released.contains(&key) }
    pub fn press(&mut self, key: KeyCode) { ... }
    pub fn release(&mut self, key: KeyCode) { ... }
    pub fn end_frame(&mut self) { ... }  // 清除 just_pressed/just_released
}
```

- [ ] **Step 3: 实现 MouseState**

```rust
// crates/engine_input/src/mouse.rs
#[derive(Debug, Clone, Copy, PartialEq, Eq, Hash)]
pub enum MouseButton { Left, Right, Middle }

#[derive(Debug, Default)]
pub struct MouseState {
    pub position: glam::Vec2,
    pub delta: glam::Vec2,
    pub scroll_delta: f32,
    pressed: std::collections::HashSet<MouseButton>,
    just_pressed: std::collections::HashSet<MouseButton>,
}

impl MouseState {
    pub fn is_pressed(&self, button: MouseButton) -> bool { ... }
    pub fn just_pressed(&self, button: MouseButton) -> bool { ... }
    pub fn press(&mut self, button: MouseButton) { ... }
    pub fn release(&mut self, button: MouseButton) { ... }
    pub fn set_position(&mut self, x: f32, y: f32) { ... }
    pub fn end_frame(&mut self) { ... }
}
```

- [ ] **Step 4: 实现 InputState (聚合)**

```rust
// crates/engine_input/src/input_state.rs
pub struct InputState {
    pub keyboard: super::keyboard::KeyboardState,
    pub mouse: super::mouse::MouseState,
}

impl InputState {
    pub fn new() -> Self { ... }
    pub fn end_frame(&mut self) { ... }  // 调用子模块的 end_frame
}
```

- [ ] **Step 5: 写 lib.rs + 测试 + 验证**

```rust
// crates/engine_input/src/lib.rs
pub mod keyboard;
pub mod mouse;
pub mod input_state;

pub use keyboard::{KeyCode, KeyboardState};
pub use mouse::{MouseButton, MouseState};
pub use input_state::InputState;
```

```bash
cargo test -p engine_input
cargo clippy -p engine_input -- -D warnings
```

- [ ] **Step 6: Commit**

---

## Chunk 5: engine_render_sw — 软件渲染后端迁移

### Task 9: 重命名 engine_renderer → engine_render_sw

**Files:**
- Rename: `crates/engine_renderer/` → `crates/engine_render_sw/`
- Modify: `crates/engine_render_sw/Cargo.toml`
- Modify: `Cargo.toml` (workspace)
- Modify: all internal references to crate name

- [ ] **Step 1: 重命名目录**

```bash
mv crates/engine_renderer crates/engine_render_sw
```

- [ ] **Step 2: 更新 Cargo.toml crate 名称**

```toml
[package]
name = "engine_render_sw"
# ... 其余不变
```

添加 engine_render_api 依赖:
```toml
[dependencies]
engine_render_api = { path = "../engine_render_api" }
# ... 现有依赖保留
```

- [ ] **Step 3: 更新 workspace members**

将 `"crates/engine_renderer"` 替换为 `"crates/engine_render_sw"`。

- [ ] **Step 4: 更新所有内部 crate 名引用**

搜索 `engine_renderer` 字符串，替换为 `engine_render_sw`（注意仅替换 crate 引用，不替换类型名如 `SimpleRenderer`）。

- [ ] **Step 5: 验证重命名后编译和测试通过**

```bash
cargo build -p engine_render_sw
cargo test -p engine_render_sw
```

Expected: 全部 207 个测试通过。

- [ ] **Step 6: Commit**

---

### Task 10: 实现 RenderBackend 适配器

**Files:**
- Create: `crates/engine_render_sw/src/adapter.rs`
- Modify: `crates/engine_render_sw/src/lib.rs`

- [ ] **Step 1: 实现 SoftwareBackend 适配器**

```rust
// crates/engine_render_sw/src/adapter.rs
use engine_render_api::{
    RenderBackend, RenderConfig, RenderError,
    FrameData, FrameOutput,
    MeshData, MeshHandle,
    TextureData, TextureHandle,
};
use crate::{SimpleRenderer, Model, Shader};

pub struct SoftwareBackend {
    renderer: SimpleRenderer,
    shader: Shader,
    width: usize,
    height: usize,
    pixel_buffer: Vec<u8>,
    // 资源存储
    models: Vec<Model>,
}

impl RenderBackend for SoftwareBackend {
    fn init(&mut self, config: RenderConfig) -> engine_render_api::Result<()> { ... }
    fn render_frame(&mut self, frame: &FrameData) -> engine_render_api::Result<FrameOutput> {
        // 将 FrameData 转换为现有渲染器格式
        // 调用 self.renderer.draw_model()
        // 返回 FrameOutput
    }
    fn resize(&mut self, width: u32, height: u32) { ... }
    fn create_mesh(&mut self, data: &MeshData) -> engine_render_api::Result<MeshHandle> { ... }
    fn create_texture(&mut self, data: &TextureData) -> engine_render_api::Result<TextureHandle> { ... }
    fn read_pixels(&self) -> &[u8] { &self.pixel_buffer }
    fn name(&self) -> &str { "Software Renderer" }
}
```

注意: 适配器的主要作用是作为 API 兼容层。当前编辑器的 render_bridge 仍可继续直接使用 SimpleRenderer — 适配器是为未来后端切换做准备。

- [ ] **Step 2: 在 lib.rs 中导出适配器**

```rust
pub mod adapter;
pub use adapter::SoftwareBackend;
```

- [ ] **Step 3: 验证适配器编译**

```bash
cargo build -p engine_render_sw
cargo test -p engine_render_sw
```

Expected: 编译通过，现有 207 个测试不受影响。

- [ ] **Step 4: Commit**

---

## Chunk 6: engine_scene + engine_editor 适配

### Task 11: 更新 engine_scene 适配 hecs + 新 core API

**Files:**
- Modify: `crates/engine_scene/Cargo.toml`
- Rewrite: `crates/engine_scene/src/lib.rs`
- Rewrite: `crates/engine_scene/src/components.rs`
- Rewrite: `crates/engine_scene/src/asset.rs`
- Rewrite: `crates/engine_scene/src/scene.rs`

- [ ] **Step 1: 更新 Cargo.toml**

```toml
[dependencies]
engine_core = { path = "../engine_core" }
engine_render_sw = { path = "../engine_render_sw" }
engine_render_api = { path = "../engine_render_api" }
glam = "0.29"
```

- [ ] **Step 2: 更新 components.rs**

Transform 已移至 engine_core，此处只保留渲染相关组件:

```rust
// crates/engine_scene/src/components.rs
use engine_core::Handle;
use engine_render_sw::Model;

/// 实体名称
#[derive(Debug, Clone)]
pub struct Name(pub String);

/// 网格渲染器组件
#[derive(Debug, Clone)]
pub struct MeshRenderer {
    pub model_handle: Handle<Model>,
}

/// 相机组件
#[derive(Debug, Clone)]
pub struct CameraComponent {
    pub fov: f32,
    pub near: f32,
    pub far: f32,
}

/// 光源组件
#[derive(Debug, Clone)]
pub struct LightComponent {
    pub direction: glam::Vec3,
    pub color: engine_render_sw::Color,
    pub intensity: f32,
}
```

- [ ] **Step 3: 更新 asset.rs — 使用 engine_core 的 Handle<T>**

```rust
// crates/engine_scene/src/asset.rs
use engine_core::{AssetId, Handle};
use engine_render_sw::Model;
use std::collections::HashMap;

pub struct AssetManager {
    next_id: AssetId,
    models: HashMap<AssetId, Model>,
}

impl AssetManager {
    pub fn new() -> Self { ... }
    pub fn load_model(&mut self, path: &str) -> engine_render_sw::Result<Handle<Model>> { ... }
    pub fn get_model(&self, handle: Handle<Model>) -> Option<&Model> { ... }
}
```

- [ ] **Step 4: 更新 scene.rs — 使用 hecs::World**

```rust
// crates/engine_scene/src/scene.rs
use hecs::World;
use engine_core::Transform;
use crate::asset::AssetManager;
use crate::components::*;

pub struct Scene {
    pub world: World,
    pub assets: AssetManager,
}

impl Scene {
    pub fn new() -> Self {
        Self { world: World::new(), assets: AssetManager::new() }
    }

    pub fn spawn_model(&mut self, name: &str, path: &str) -> engine_render_sw::Result<hecs::Entity> {
        let handle = self.assets.load_model(path)?;
        let entity = self.world.spawn((
            Name(name.to_string()),
            Transform::default(),
            MeshRenderer { model_handle: handle },
        ));
        Ok(entity)
    }

    pub fn spawn_light(&mut self, name: &str, direction: glam::Vec3, color: engine_render_sw::Color) -> hecs::Entity {
        self.world.spawn((
            Name(name.to_string()),
            Transform::default(),
            LightComponent { direction, color, intensity: 1.0 },
        ))
    }

    pub fn spawn_camera(&mut self, name: &str) -> hecs::Entity {
        self.world.spawn((
            Name(name.to_string()),
            Transform::default(),
            CameraComponent { fov: 45.0, near: 0.1, far: 100.0 },
        ))
    }
}
```

- [ ] **Step 5: 更新 lib.rs**

```rust
pub mod asset;
pub mod components;
pub mod scene;

pub use asset::AssetManager;
pub use components::*;
pub use scene::Scene;
```

- [ ] **Step 6: 更新测试并验证**

```bash
cargo test -p engine_scene
cargo clippy -p engine_scene -- -D warnings
```

- [ ] **Step 7: Commit**

---

### Task 12: 更新 engine_editor 适配新 crate 名

**Files:**
- Modify: `crates/engine_editor/Cargo.toml`
- Modify: `crates/engine_editor/src/app.rs`
- Modify: `crates/engine_editor/src/render_bridge.rs`

- [ ] **Step 1: 更新 Cargo.toml 依赖**

```toml
[dependencies]
engine_core = { path = "../engine_core" }
engine_render_sw = { path = "../engine_render_sw" }   # 原 engine_renderer
engine_render_api = { path = "../engine_render_api" }
engine_scene = { path = "../engine_scene" }
engine_input = { path = "../engine_input" }
eframe = "0.31"
egui = "0.31"
rfd = "0.15"
glam = "0.29"
log = "0.4"
env_logger = "0.11"
```

- [ ] **Step 2: 更新所有 `use engine_renderer::` 为 `use engine_render_sw::`**

搜索并替换 `engine_renderer` → `engine_render_sw` 在整个 engine_editor crate 中。

- [ ] **Step 3: 验证编辑器编译**

```bash
cargo build -p engine_editor
```

- [ ] **Step 4: 手动验证编辑器启动**

```bash
cargo run -p engine_editor
```

Expected: 编辑器正常启动，渲染视口显示模型，所有面板功能正常。

- [ ] **Step 5: Commit**

---

## Chunk 7: system_test 适配 + Workspace 清理

### Task 13: 更新 system_test 依赖

**Files:**
- Modify: `system_test/Cargo.toml`

- [ ] **Step 1: 更新依赖名称**

将 `engine_renderer` 替换为 `engine_render_sw`。

- [ ] **Step 2: 更新 system_test 源码中的 use 声明**

搜索并替换 `engine_renderer` → `engine_render_sw`。

- [ ] **Step 3: 验证 system_test 编译**

```bash
cargo build -p system_test
```

- [ ] **Step 4: Commit**

---

### Task 14: 更新 Workspace root Cargo.toml + CI

**Files:**
- Modify: `Cargo.toml`
- Modify: `.github/workflows/workflow.yml`

- [ ] **Step 1: 更新 workspace members**

```toml
[workspace]
members = [
    "crates/engine_math",
    "crates/engine_core",
    "crates/engine_render_api",
    "crates/engine_input",
    "crates/engine_render_sw",
    "crates/engine_scene",
    "crates/engine_editor",
    "system_test",
]
resolver = "2"
```

- [ ] **Step 2: 全量验证**

```bash
cargo build --workspace
cargo test --workspace
cargo clippy --workspace -- -D warnings
```

Expected:
- 编译: 全部 8 个 crate 通过
- 测试: engine_render_sw 207 + engine_core 新测试 + engine_scene 测试 + 新 crate 测试
- Clippy: 零警告

- [ ] **Step 3: 更新 AGENTS.md**

更新项目结构、代码映射表、命令表以反映新的 crate 组织。

- [ ] **Step 4: Commit**
