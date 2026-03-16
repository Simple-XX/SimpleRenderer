# PROJECT KNOWLEDGE BASE

**Generated:** 2026-03-11
**Commit:** 6872f7b
**Branch:** refactor-rust

## OVERVIEW

Educational software renderer mimicking OpenGL's GPU pipeline in **Rust**. Workspace with `simple_renderer` library crate + `system_test` binary crate. Uses glam math, tobj model loading, rayon parallelism, minifb display. Multi-threaded: dedicated render thread communicates with main (input/display) thread via lock-free triple buffer.

## STRUCTURE

```
SimpleRenderer/
├── Cargo.toml              # Workspace root (resolver=2, dev opt-level=2)
├── simple_renderer/        # Core library crate
│   ├── src/
│   │   ├── lib.rs          # Module declarations + pub re-exports
│   │   ├── renderer.rs     # SimpleRenderer: mode selection + draw_model entry
│   │   ├── renderers/      # 4 rendering strategies (see renderers/AGENTS.md)
│   │   ├── shader.rs       # Vertex/Fragment shaders, uniform caching (970 lines)
│   │   ├── rasterizer.rs   # Barycentric interpolation, perspective correction
│   │   ├── model.rs        # tobj OBJ loader with texture cache
│   │   ├── buffer.rs       # Double-buffered framebuffer (flag-swap, no copy)
│   │   ├── triple_buffer.rs # Lock-free triple buffer (Writer+Reader, AtomicU8 state)
│   │   ├── color.rs        # 32-bit RGBA, little-endian u32 compatible
│   │   ├── vertex.rs       # AoS Vertex + SoA VertexSoA for tile renderers
│   │   ├── fragment.rs     # Fragment (no material — passed separately for thread safety)
│   │   ├── uniform.rs      # HashMap-based UniformBuffer with typed From<T> impls
│   │   ├── material.rs     # Material + Texture (image crate loading)
│   │   ├── math.rs         # Re-exports glam; perspective_rh_gl, look_at_rh
│   │   ├── light.rs        # Light (name, position, direction, color)
│   │   ├── face.rs         # Face (3 vertex indices + Arc<Material>)
│   │   └── error.rs        # RendererError (thiserror) + Result<T> alias
│   └── tests/
│       └── integration_test.rs  # Renders teapot in all 4 modes
├── system_test/            # Visual demo binary
│   └── src/
│       ├── main.rs         # Multi-threaded: main (input/display) + render thread via triple buffer
│       ├── camera.rs       # FPS-style free camera (Euler angles)
│       └── display.rs      # minifb window + input → RenderingMode/Camera/VSync/BufferMode
├── obj/                    # Bundled 3D models (.obj/.mtl)
├── docs/plans/             # Implementation plans
├── .github/workflows/      # CI (currently C++-focused, needs Rust update)
├── LICENSE, README.md, README-cn.md
```

## WHERE TO LOOK

| Task | Location | Notes |
|------|----------|-------|
| Add rendering algorithm | `simple_renderer/src/renderers/` | Implement `Renderer` trait, register in `renderer.rs::create_renderer` |
| Change shader logic | `simple_renderer/src/shader.rs` | `vertex_shader` (→ clip space), `fragment_shader` (Phong) |
| Modify rasterization | `simple_renderer/src/rasterizer.rs` | Used by `PerTriangle` + `Deferred`; tile renderers have inline edge-function rasterization |
| Add model format | `simple_renderer/src/model.rs` | Currently tobj-only (OBJ/MTL) |
| Change display/input | `system_test/src/display.rs` + `camera.rs` | minifb window, keyboard/mouse handling |
| Add uniforms | `simple_renderer/src/uniform.rs` | Add variant to `UniformValue`, add `From<T>` impl, add getter on `UniformBuffer` |
| Add material property | `simple_renderer/src/material.rs` | Update `Material` struct + `model.rs` loader |
| Run the app | `system_test/src/main.rs` | `cargo run -p system_test -- ./obj` |
| Add integration tests | `simple_renderer/tests/` | `cargo test --test integration_test` |
| Add unit tests | Inline `#[cfg(test)]` modules | Every source file has them |

## CODE MAP

| Symbol | Type | Location | Role |
|--------|------|----------|------|
| `SimpleRenderer` | Struct | `renderer.rs` | Mode enum dispatch → `Box<dyn Renderer>` |
| `Renderer` | Trait | `renderers/mod.rs` | `fn render(&self, model, shader, buffer, w, h) -> bool` |
| `Shader` | Struct | `shader.rs` | Vertex/Fragment shaders, uniform + specular LUT caching |
| `Rasterizer` | Struct | `rasterizer.rs` | Barycentric rasterization (rayon over scanlines) |
| `Model` | Struct | `model.rs` | OBJ loader: vertices, faces, materials, texture cache |
| `Buffer` | Struct | `buffer.rs` | Double-buffer with flag-swap (no memcpy) |
| `TripleBufferWriter` | Struct | `triple_buffer.rs` | Render-thread half of lock-free triple buffer |
| `TripleBufferReader` | Struct | `triple_buffer.rs` | Display-thread half of lock-free triple buffer |
| `Vertex` / `VertexSoA` | Structs | `vertex.rs` | AoS for per-triangle path, SoA for tile-based path |
| `Fragment` | Struct | `fragment.rs` | Rasterizer output → fragment shader input |
| `UniformBuffer` | Struct | `uniform.rs` | `HashMap<String, UniformValue>` with typed getters |
| `Face` | Struct | `face.rs` | 3 vertex indices + `Arc<Material>` (shared ownership) |
| `Color` | Struct | `color.rs` | `[u8; 4]` RGBA, little-endian u32 compatible |
| `RendererError` | Enum | `error.rs` | `ModelLoad`, `TextureLoad`, `RenderFailed`, `Io` |

## ARCHITECTURE

Pipeline mirrors OpenGL: **Vertex Shader → Perspective Division → Viewport Transform → Rasterization → Fragment Shader → Depth Test → Framebuffer**.

- `Shader::vertex_shader(&mut self)` — sequential (writes `frag_pos_varying`)
- `Rasterizer::rasterize(&self)` — parallel over scanlines via rayon
- `Shader::fragment_shader(&self)` — Blinn-Phong with specular LUT (RwLock for thread safety)
- Materials passed separately to fragment shader (not stored in Fragment) for thread safety

Four renderer strategies share this pipeline but differ in scheduling — see `renderers/AGENTS.md`.

## CONVENTIONS

- **Math**: Right-handed coordinate system, OpenGL depth range `[-1, 1]`, Y-flipped in viewport transform
- **Error handling**: `thiserror` → `RendererError`, propagate with `?`, warn on non-fatal (material load failures)
- **Parallelism**: `rayon` everywhere — scanline-parallel rasterizer, chunk-parallel renderers, tile-parallel tile renderers
- **Caching**: Shader caches derived matrices (MVP, normal) and light directions to avoid per-vertex/fragment HashMap lookups
- **Thread safety**: `Renderer: Send`, `RwLock` for specular LUT, `Arc<Material>` for shared face materials, per-thread local buffers in renderers
- **Testing**: Every `.rs` file has inline `#[cfg(test)]` module; integration tests render real teapot model in all 4 modes
- **C++ port**: Comments reference original C++ function names (`Port of C++ Shader::VertexShader`)
- **Chinese**: `system_test/` uses Chinese comments and UI strings

## COMMANDS

```bash
cargo build                              # Debug build
cargo build --release                    # Release build
cargo run -p system_test -- ./obj        # Run demo (teapot)
cargo test                               # All tests
cargo test -p simple_renderer            # Library unit tests only
cargo test --test integration_test       # Integration tests only
```

## DEPENDENCIES

| Crate | Purpose |
|-------|---------|
| glam 0.29 | Vec3, Vec4, Mat4 math |
| tobj 4.0 | OBJ model loading (with `async` feature) |
| image 0.25 | Texture loading (png, jpeg, bmp, tga features) |
| rayon 1.10 | Parallel iteration |
| minifb 0.27 | Window/display (system_test only) |
| thiserror 2 | Error derive macros |
| log 0.4 + env_logger 0.11 | Logging |

## NOTES

- **Dev profile**: `opt-level = 2` globally, `opt-level = 3` for `image` and `tobj` (perf-critical deps)
- **Default mode**: `TileBased` (set in `SimpleRenderer::new`)
- **CI workflow**: `.github/workflows/workflow.yml` still targets C++ build — needs Rust update
- **Integration tests**: Depend on `obj/utah-teapot-texture/teapot.obj` at runtime
- **Minimal `unsafe`**: Only `triple_buffer.rs` uses `unsafe` for `Send`/`Sync` impls on the lock-free buffer. All rendering logic is safe Rust
