# PROJECT KNOWLEDGE BASE

**Generated:** 2026-03-18
**Branch:** refactor-rust

## OVERVIEW

Modular game engine built on a software renderer mimicking OpenGL's GPU pipeline in **Rust**. Cargo workspace with 7 crates organized in a layered architecture:

- **Layer 1 (Foundation):** `engine_core` (hecs ECS + Plugin/Schedule/EventBus/ResourceMap), `engine_math` (glam re-export + AABB/Ray/Frustum), `engine_render_api` (RenderBackend trait + data types), `engine_input` (keyboard/mouse state)
- **Layer 2 (Infrastructure):** `engine_render_sw` (software rendering backend), `engine_scene` (assets + components)
- **Layer 4 (Application):** `engine_editor` (egui GUI)

Uses hecs ECS, glam math, tobj model loading, rayon parallelism, eframe/egui editor GUI. Multi-threaded: dedicated render thread communicates with editor via lock-free triple buffer.

## STRUCTURE

```
SimpleGameEngine/
├── Cargo.toml                    # Workspace root (resolver=2, dev opt-level=2)
├── crates/
│   ├── engine_math/              # Layer 1: Math utilities
│   │   └── src/
│   │       ├── lib.rs            # glam re-export (pub use glam::*)
│   │       ├── aabb.rs           # AABB (contains, intersects, center, size)
│   │       ├── ray.rs            # Ray (point_at, intersects_aabb slab method)
│   │       └── frustum.rs        # Plane + Frustum (from_view_projection, contains_point, intersects_aabb)
│   ├── engine_core/              # Layer 1: ECS + Plugin + Schedule + Events + Resources
│   │   └── src/
│   │       ├── lib.rs            # Module declarations + pub re-exports
│   │       ├── ecs.rs            # hecs re-exports (Entity, World)
│   │       ├── time.rs           # Time (delta, total, frame_count)
│   │       ├── resource.rs       # ResourceMap (TypeMap pattern)
│   │       ├── event.rs          # Event trait + EventBus
│   │       ├── schedule.rs       # Phase enum (6 phases) + Schedule + SystemContext
│   │       ├── plugin.rs         # Plugin trait (name, dependencies, build, cleanup)
│   │       ├── app.rs            # AppBuilder (world + resources + events + schedule + plugins)
│   │       ├── transform.rs      # Transform, Parent, Children, GlobalTransform
│   │       ├── despawn.rs        # DespawnQueue (mark/drain for entity cleanup)
│   │       └── asset.rs          # AssetId, Handle<T>, AssetLoader trait
│   ├── engine_render_api/        # Layer 1: Render backend abstraction
│   │   └── src/
│   │       ├── lib.rs            # re-exports
│   │       ├── backend.rs        # RenderBackend trait + RenderError
│   │       ├── frame.rs          # FrameData, DrawCall, CameraData, LightData, FrameOutput
│   │       ├── mesh.rs           # MeshData, MeshHandle
│   │       ├── texture.rs        # TextureData, TextureHandle, TextureFormat
│   │       ├── material.rs       # MaterialData, MaterialHandle
│   │       ├── color.rs          # Color (f32 RGBA)
│   │       └── viewport.rs       # Viewport, RenderConfig
│   ├── engine_input/             # Layer 1: Input state management
│   │   └── src/
│   │       ├── lib.rs            # re-exports
│   │       ├── keyboard.rs       # KeyCode enum + KeyboardState (press/release/just_pressed)
│   │       ├── mouse.rs          # MouseButton + MouseState (position/delta/scroll)
│   │       └── input_state.rs    # InputState (aggregates keyboard + mouse)
│   ├── engine_render_sw/         # Layer 2: Software renderer (原 engine_renderer)
│   │   ├── src/
│   │   │   ├── lib.rs            # Module declarations + pub re-exports
│   │   │   ├── renderer.rs       # SimpleRenderer: draw_model(&mut self)
│   │   │   ├── renderers/        # 4 rendering strategies
│   │   │   ├── shader/           # Shader module (mod + vertex + fragment + specular_lut)
│   │   │   ├── rasterizer.rs     # Barycentric interpolation
│   │   │   ├── model.rs          # tobj OBJ loader
│   │   │   ├── buffer.rs         # Double-buffered framebuffer
│   │   │   ├── triple_buffer.rs  # Lock-free triple buffer
│   │   │   ├── color.rs          # 32-bit RGBA (little-endian only)
│   │   │   ├── vertex.rs         # Vertex (AoS) + VertexSoA
│   │   │   ├── fragment.rs       # Fragment data
│   │   │   ├── uniform.rs        # UniformBuffer + uniform::names constants
│   │   │   ├── material.rs       # Material + Texture
│   │   │   ├── face.rs           # Face (3 indices + Arc<Material>)
│   │   │   ├── light.rs          # Light (position, direction, color)
│   │   │   ├── math.rs           # Re-exports glam
│   │   │   └── error.rs          # RendererError + Result<T>
│   │   └── tests/
│   │       ├── integration_test.rs
│   │       └── property_tests.rs
│   ├── engine_scene/             # Layer 2: Scene management + Assets
│   │   └── src/
│   │       ├── lib.rs
│   │       ├── asset.rs          # AssetManager (uses engine_core::Handle<T>)
│   │       ├── components.rs     # Name, MeshRenderer, CameraComponent, LightComponent
│   │       └── scene.rs          # Scene (hecs::World + AssetManager)
│   └── engine_editor/            # Layer 4: egui editor GUI
│       └── src/
│           ├── main.rs           # eframe entry point
│           ├── app.rs            # EditorApp: eframe::App impl
│           ├── render_bridge.rs  # Render thread + triple buffer + RenderCommand channel
│           ├── camera_control.rs # FPS-style viewport camera
│           └── panels/
│               ├── mod.rs
│               ├── viewport.rs   # Render output as egui texture
│               ├── scene_tree.rs # Entity list (left panel)
│               ├── properties.rs # Parameter sliders (right panel)
│               └── status_bar.rs # FPS/stats (bottom)
├── assets/models/                # Bundled 3D models (.obj/.mtl)
├── docs/specs/                   # Design specs
├── docs/plans/                   # Implementation plans
├── .github/workflows/            # CI: Rust build + test + clippy -D warnings
```

## WHERE TO LOOK

| Task | Location | Notes |
|------|----------|-------|
| Add ECS component | `crates/engine_scene/src/components.rs` | hecs components = any Send+Sync+'static type |
| Add rendering algorithm | `crates/engine_render_sw/src/renderers/` | Impl `Renderer` trait (`&mut self`) |
| Change vertex shader | `crates/engine_render_sw/src/shader/vertex.rs` | `vertex_shader(&self)` |
| Change fragment shader | `crates/engine_render_sw/src/shader/fragment.rs` | `fragment_shader(&self)` Blinn-Phong |
| Add uniform | `crates/engine_render_sw/src/uniform.rs` | Add variant + constant to `uniform::names` |
| Add editor panel | `crates/engine_editor/src/panels/` | New panel file + integrate in `app.rs` |
| Modify editor menu | `crates/engine_editor/src/app.rs` | `egui::menu::bar` in `update()` |
| Add asset type | `crates/engine_scene/src/asset.rs` | Uses engine_core::Handle<T> |
| Add plugin | `crates/engine_core/src/plugin.rs` | Impl Plugin trait (name, build, dependencies) |
| Add system | `crates/engine_core/src/schedule.rs` | Use Phase enum + Schedule::add_system |
| Add resource | `crates/engine_core/src/resource.rs` | ResourceMap::insert/get |
| Add event | `crates/engine_core/src/event.rs` | Impl Event trait, use EventBus::send/read |
| Add math utility | `crates/engine_math/src/` | AABB, Ray, Frustum already available |
| Add input binding | `crates/engine_input/src/` | KeyCode, MouseButton enums |
| Define render backend | `crates/engine_render_api/src/backend.rs` | Impl RenderBackend trait |
| Run editor | `cargo run -p engine_editor` | egui window with render viewport |

## CODE MAP

| Symbol | Type | Crate | Role |
|--------|------|-------|------|
| `World` | Struct (hecs) | engine_core | ECS: archetype entity storage |
| `Entity` | Struct (hecs) | engine_core | Opaque entity handle |
| `AppBuilder` | Struct | engine_core | Plugin registration + main loop |
| `Plugin` | Trait | engine_core | Modular engine extension point |
| `Schedule` | Struct | engine_core | Phase-based system execution |
| `Phase` | Enum | engine_core | PreUpdate/Update/PostUpdate/PreRender/Render/PostRender |
| `SystemContext` | Struct | engine_core | World + Resources + Events access for systems |
| `ResourceMap` | Struct | engine_core | Type-keyed global resource storage |
| `EventBus` | Struct | engine_core | Type-keyed per-frame event channels |
| `Transform` | Struct | engine_core | Translation/rotation/scale component |
| `DespawnQueue` | Struct | engine_core | Deferred entity destruction |
| `Handle<T>` | Struct | engine_core | Type-safe asset reference |
| `AssetLoader` | Trait | engine_core | Extensible asset loading |
| `Time` | Struct | engine_core | Frame timing |
| `AABB` | Struct | engine_math | Axis-aligned bounding box |
| `Ray` | Struct | engine_math | Ray for intersection tests |
| `Frustum` | Struct | engine_math | View frustum culling |
| `RenderBackend` | Trait | engine_render_api | Backend-agnostic render interface |
| `FrameData` | Struct | engine_render_api | Per-frame render data |
| `DrawCall` | Struct | engine_render_api | Single draw command |
| `InputState` | Struct | engine_input | Unified keyboard + mouse state |
| `KeyCode` | Enum | engine_input | Keyboard key identifiers |
| `Scene` | Struct | engine_scene | World + AssetManager bundle |
| `AssetManager` | Struct | engine_scene | Model loading + Handle<T> |
| `SimpleRenderer` | Struct | engine_render_sw | Mode dispatch → `Box<dyn Renderer>` |
| `Renderer` | Trait | engine_render_sw | `fn render(&mut self, ...)` |
| `Shader` | Struct | engine_render_sw | Vertex/fragment shader + uniform caching |
| `EditorApp` | Struct | engine_editor | eframe::App impl, orchestrates GUI |
| `RenderBridge` | Struct | engine_editor | Render thread + triple buffer communication |
| `EditorCamera` | Struct | engine_editor | FPS-style viewport camera control |
| `RenderCommand` | Enum | engine_editor | Commands sent to render thread |

## ARCHITECTURE

### Layered Architecture

```
Layer 4 — Application
  engine_editor       egui 编辑器 GUI

Layer 2 — Infrastructure
  engine_scene        Scene + AssetManager + Components
  engine_render_sw    Software rendering backend

Layer 1 — Foundation (互不依赖)
  engine_core         hecs ECS + Plugin + Schedule + EventBus + ResourceMap
  engine_math         glam re-export + AABB/Ray/Frustum
  engine_render_api   RenderBackend trait + FrameData + data types
  engine_input        Keyboard/Mouse state management
```

### Dependency Graph

```
engine_editor ──┬── engine_scene ──┬── engine_core
                │                  └── engine_render_sw
                ├── engine_render_sw ── engine_render_api
                └── engine_core
```

Layer 1 crates have NO inter-dependencies. Upper layers depend on lower layers only.

### Render Pipeline

Vertex Shader → Perspective Division → Viewport Transform → Rasterization → Fragment Shader → Depth Test → Framebuffer

### Editor Threading

```
Editor (main thread, egui)
  ├─ writes → mpsc::channel<RenderCommand> → Render Thread
  └─ reads  ← TripleBuffer ←───────────── Render Thread
```

## CONVENTIONS

- **ECS**: hecs archetype storage. Components = any `Send + Sync + 'static` type
- **Plugin system**: Impl `Plugin` trait, register via `AppBuilder::add_plugin`
- **Systems**: `FnMut(&mut SystemContext)` closures, registered to a `Phase`
- **Uniform names**: Always use `uniform::names::*` constants
- **Error handling**: `thiserror` → `RendererError`, propagate with `?`
- **Parallelism**: `rayon` in renderers, parallel merge in PerTriangle
- **Buffer reuse**: Renderer structs own depth/color buffers, reuse with `fill()`
- **Testing**: Every `.rs` file has `#[cfg(test)]` module; proptest for properties
- **Comments**: Chinese (中文注释)
- **License**: MIT, declared in workspace `Cargo.toml` `[workspace.package]`; no per-file copyright headers
- **Platform**: Little-endian only (`compile_error!` guard)
- **MSRV**: Rust 1.73+

## COMMANDS

```bash
cargo build --workspace                  # Build all crates
cargo run -p engine_editor               # Run egui editor
cargo test --workspace                   # All tests (363)
cargo test -p engine_render_sw           # Renderer tests (207)
cargo test -p engine_core                # ECS + Plugin + Schedule tests (63)
cargo test -p engine_math                # Math tests (39)
cargo test -p engine_input               # Input tests (20)
cargo test -p engine_render_api          # Render API tests (13)
cargo test -p engine_scene               # Scene tests (5)
cargo clippy --workspace -- -D warnings  # Lint (CI requires zero warnings)
```

## DEPENDENCIES

| Crate | Purpose | Used by |
|-------|---------|---------|
| hecs 0.10 | Archetype ECS | engine_core |
| glam 0.29 | Vec3, Vec4, Mat4 math | all |
| tobj 4.0 | OBJ model loading | engine_render_sw |
| image 0.25 | Texture loading | engine_render_sw |
| rayon 1.10 | Parallel iteration | engine_render_sw |
| thiserror 2 | Error derive macros | engine_render_sw, engine_render_api |
| log 0.4 | Logging | engine_render_sw, engine_editor |
| env_logger 0.11 | Log output | engine_editor |
| eframe 0.31 | egui native window | engine_editor |
| egui 0.31 | Immediate-mode GUI | engine_editor |
| rfd 0.15 | Native file dialogs | engine_editor |
| proptest 1 (dev) | Property testing | engine_render_sw |
| criterion 0.5 (dev) | Benchmarking | engine_render_sw |
