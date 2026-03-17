# PROJECT KNOWLEDGE BASE

**Generated:** 2026-03-18
**Branch:** refactor-rust

## OVERVIEW

Game engine built on a software renderer mimicking OpenGL's GPU pipeline in **Rust**. Cargo workspace with 5 crates: `engine_core` (ECS), `engine_renderer` (software rendering), `engine_scene` (assets + components), `engine_editor` (egui GUI), and legacy `system_test` (minifb demo). Uses glam math, tobj model loading, rayon parallelism, eframe/egui editor GUI. Multi-threaded: dedicated render thread communicates with editor via lock-free triple buffer.

## STRUCTURE

```
SimpleRenderer/
├── Cargo.toml                    # Workspace root (resolver=2, dev opt-level=2)
├── crates/
│   ├── engine_core/              # ECS + Time
│   │   └── src/
│   │       ├── lib.rs
│   │       ├── ecs.rs            # Entity(u64), Component, World (HashMap storage)
│   │       └── time.rs           # Time (delta, total, frame_count)
│   ├── engine_renderer/          # Software renderer (原 simple_renderer)
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
│   ├── engine_scene/             # Scene management + Assets
│   │   └── src/
│   │       ├── lib.rs
│   │       ├── asset.rs          # AssetId, Handle<T>, AssetManager
│   │       ├── components.rs     # Transform, Name, MeshRenderer, CameraComponent, LightComponent
│   │       └── scene.rs          # Scene (World + AssetManager)
│   └── engine_editor/            # egui editor GUI
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
├── system_test/                  # Legacy minifb demo (retained)
├── assets/models/                # Bundled 3D models (.obj/.mtl)
├── docs/specs/                   # Design specs
├── docs/plans/                   # Implementation plans
├── .github/workflows/            # CI: Rust build + test + clippy -D warnings
```

## WHERE TO LOOK

| Task | Location | Notes |
|------|----------|-------|
| Add ECS component | `crates/engine_scene/src/components.rs` | Impl `engine_core::Component` trait |
| Add rendering algorithm | `crates/engine_renderer/src/renderers/` | Impl `Renderer` trait (`&mut self`) |
| Change vertex shader | `crates/engine_renderer/src/shader/vertex.rs` | `vertex_shader(&self)` |
| Change fragment shader | `crates/engine_renderer/src/shader/fragment.rs` | `fragment_shader(&self)` Blinn-Phong |
| Add uniform | `crates/engine_renderer/src/uniform.rs` | Add variant + constant to `uniform::names` |
| Add editor panel | `crates/engine_editor/src/panels/` | New panel file + integrate in `app.rs` |
| Modify editor menu | `crates/engine_editor/src/app.rs` | `egui::menu::bar` in `update()` |
| Add asset type | `crates/engine_scene/src/asset.rs` | Add HashMap + Handle methods |
| Run editor | `cargo run -p engine_editor` | egui window with render viewport |
| Run legacy demo | `cargo run -p system_test -- ./assets/models` | minifb window |

## CODE MAP

| Symbol | Type | Crate | Role |
|--------|------|-------|------|
| `World` | Struct | engine_core | ECS: entity storage, typed component maps |
| `Entity` | Type alias (u64) | engine_core | Unique entity ID |
| `Component` | Trait | engine_core | Marker for ECS components |
| `Time` | Struct | engine_core | Frame timing |
| `Scene` | Struct | engine_scene | World + AssetManager bundle |
| `AssetManager` | Struct | engine_scene | Model loading + Handle<T> |
| `Transform` | Struct | engine_scene | Position/rotation/scale component |
| `SimpleRenderer` | Struct | engine_renderer | Mode dispatch → `Box<dyn Renderer>` |
| `Renderer` | Trait | engine_renderer | `fn render(&mut self, ...)` |
| `Shader` | Struct | engine_renderer | Vertex/fragment shader + uniform caching |
| `EditorApp` | Struct | engine_editor | eframe::App impl, orchestrates GUI |
| `RenderBridge` | Struct | engine_editor | Render thread + triple buffer communication |
| `EditorCamera` | Struct | engine_editor | FPS-style viewport camera control |
| `RenderCommand` | Enum | engine_editor | Commands sent to render thread |

## ARCHITECTURE

### Engine Layers

```
engine_editor (egui GUI, top-level binary)
    ↓ depends on
engine_scene (Scene, AssetManager, Components)
    ↓ depends on
engine_core (ECS)    engine_renderer (software rendering)
    ↓                    ↓
    glam                 glam, tobj, image, rayon
```

`engine_core` and `engine_renderer` are parallel base crates with NO dependency on each other. `engine_scene` bridges them. `engine_editor` depends on all.

### Render Pipeline

Vertex Shader → Perspective Division → Viewport Transform → Rasterization → Fragment Shader → Depth Test → Framebuffer

### Editor Threading

```
Editor (main thread, egui)
  ├─ writes → mpsc::channel<RenderCommand> → Render Thread
  └─ reads  ← TripleBuffer ←───────────── Render Thread
```

## CONVENTIONS

- **Uniform names**: Always use `uniform::names::*` constants
- **Error handling**: `thiserror` → `RendererError`, propagate with `?`
- **Parallelism**: `rayon` in renderers, parallel merge in PerTriangle
- **Buffer reuse**: Renderer structs own depth/color buffers, reuse with `fill()`
- **ECS**: `HashMap<TypeId, HashMap<Entity, Box<dyn Any>>>` storage (simple, not optimized)
- **Testing**: Every `.rs` file has `#[cfg(test)]` module; proptest for properties
- **Comments**: Chinese (中文注释)
- **Copyright**: Every `.rs` file starts with `// Copyright (c) Simple-XX/SimpleRenderer`
- **Platform**: Little-endian only (`compile_error!` guard)
- **MSRV**: Rust 1.73+

## COMMANDS

```bash
cargo build --workspace                  # Build all crates
cargo run -p engine_editor               # Run egui editor
cargo run -p system_test -- ./assets/models  # Run legacy minifb demo
cargo test --workspace                   # All tests (245)
cargo test -p engine_renderer            # Renderer tests (207)
cargo test -p engine_core                # ECS tests (16)
cargo test -p engine_scene               # Scene tests (6)
cargo clippy --workspace -- -D warnings  # Lint (CI requires zero warnings)
```

## DEPENDENCIES

| Crate | Purpose | Used by |
|-------|---------|---------|
| glam 0.29 | Vec3, Vec4, Mat4 math | all |
| tobj 4.0 | OBJ model loading | engine_renderer |
| image 0.25 | Texture loading | engine_renderer |
| rayon 1.10 | Parallel iteration | engine_renderer |
| thiserror 2 | Error derive macros | engine_renderer |
| log 0.4 | Logging | engine_renderer, engine_editor |
| env_logger 0.11 | Log output | engine_editor |
| eframe 0.31 | egui native window | engine_editor |
| egui 0.31 | Immediate-mode GUI | engine_editor |
| rfd 0.15 | Native file dialogs | engine_editor |
| minifb 0.27 | Window/display | system_test (legacy) |
| proptest 1 (dev) | Property testing | engine_renderer |
| criterion 0.5 (dev) | Benchmarking | engine_renderer |
