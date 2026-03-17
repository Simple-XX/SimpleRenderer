# PROJECT KNOWLEDGE BASE

**Generated:** 2026-03-17
**Commit:** e7de7b6
**Branch:** refactor-rust

## OVERVIEW

Educational software renderer mimicking OpenGL's GPU pipeline in **Rust**. Workspace with `simple_renderer` library crate + `system_test` binary crate. Uses glam math, tobj model loading, rayon parallelism, minifb display. Multi-threaded: dedicated render thread communicates with main (input/display) thread via lock-free triple buffer.

## STRUCTURE

```
SimpleRenderer/
├── Cargo.toml              # Workspace root (resolver=2, dev opt-level=2)
├── simple_renderer/        # Core library crate (rust-version 1.73)
│   ├── src/
│   │   ├── lib.rs          # Module declarations + pub re-exports
│   │   ├── renderer.rs     # SimpleRenderer: mode selection + draw_model(&mut self)
│   │   ├── renderers/      # 4 rendering strategies (see renderers/AGENTS.md)
│   │   ├── shader/         # Shader module (split into sub-files)
│   │   │   ├── mod.rs      # Shader struct, uniform management, Clone, Default
│   │   │   ├── vertex.rs   # vertex_shader(&self), VertexUniformCache, matrix caching
│   │   │   ├── fragment.rs # fragment_shader(&self), FragmentUniformCache, Blinn-Phong
│   │   │   └── specular_lut.rs # SpecularLut, evaluate_specular, RwLock-based caching
│   │   ├── rasterizer.rs   # Barycentric interpolation, perspective correction
│   │   ├── model.rs        # tobj OBJ loader with texture cache
│   │   ├── buffer.rs       # Double-buffered framebuffer (flag-swap, no copy)
│   │   ├── triple_buffer.rs # Lock-free triple buffer (Writer+Reader, AtomicU8 state)
│   │   ├── color.rs        # 32-bit RGBA, little-endian only (compile_error on big-endian)
│   │   ├── vertex.rs       # AoS Vertex + SoA VertexSoA for tile renderers
│   │   ├── fragment.rs     # Fragment (no material — passed separately for thread safety)
│   │   ├── uniform.rs      # UniformBuffer + uniform::names constants for type-safe keys
│   │   ├── material.rs     # Material + Texture (image crate loading)
│   │   ├── math.rs         # Re-exports glam
│   │   ├── light.rs        # Light (name, position, direction, color)
│   │   ├── face.rs         # Face (3 vertex indices + Arc<Material>)
│   │   └── error.rs        # RendererError (thiserror) + Result<T> alias
│   └── tests/
│       ├── integration_test.rs  # Renders teapot in all 4 modes
│       └── property_tests.rs    # proptest: Color, Vertex, Shader, Buffer properties
├── system_test/            # Visual demo binary
│   └── src/
│       ├── main.rs         # Multi-threaded: main (input/display) + render thread via triple buffer
│       ├── camera.rs       # FPS-style free camera (Euler angles)
│       └── display.rs      # minifb window + input → RenderingMode/Camera/VSync/BufferMode
├── obj/                    # Bundled 3D models (.obj/.mtl)
├── docs/plans/             # Implementation plans
├── .github/workflows/      # CI: Rust build + test + clippy -D warnings
├── LICENSE, README.md, README-cn.md
```

## WHERE TO LOOK

| Task | Location | Notes |
|------|----------|-------|
| Add rendering algorithm | `simple_renderer/src/renderers/` | Implement `Renderer` trait (`&mut self`), register in `renderer.rs::create_renderer` |
| Change vertex shader | `simple_renderer/src/shader/vertex.rs` | `vertex_shader(&self)` → clip space, caches MVP/normal matrices |
| Change fragment shader | `simple_renderer/src/shader/fragment.rs` | `fragment_shader(&self)` → Blinn-Phong, specular LUT in `specular_lut.rs` |
| Add uniform | `simple_renderer/src/uniform.rs` | Add variant to `UniformValue`, add `From<T>` impl, add getter, add constant to `uniform::names` |
| Modify rasterization | `simple_renderer/src/rasterizer.rs` | Used by `PerTriangle` + `Deferred`; tile renderers have inline edge-function rasterization |
| Add model format | `simple_renderer/src/model.rs` | Currently tobj-only (OBJ/MTL) |
| Change display/input | `system_test/src/display.rs` + `camera.rs` | minifb window, keyboard/mouse handling |
| Add material property | `simple_renderer/src/material.rs` | Update `Material` struct + `model.rs` loader |
| Run the app | `system_test/src/main.rs` | `cargo run -p system_test -- ./obj` |
| Add integration tests | `simple_renderer/tests/` | `cargo test --test integration_test` |
| Add property tests | `simple_renderer/tests/property_tests.rs` | proptest-based, covers Color/Vertex/Shader/Buffer |
| Add unit tests | Inline `#[cfg(test)]` modules | Every source file has them |

## CODE MAP

| Symbol | Type | Location | Role |
|--------|------|----------|------|
| `SimpleRenderer` | Struct | `renderer.rs` | Mode enum dispatch → `Box<dyn Renderer>`, `draw_model(&mut self)` |
| `Renderer` | Trait | `renderers/mod.rs` | `fn render(&mut self, model, shader, buffer, w, h) -> Result<()>` |
| `Shader` | Struct | `shader/mod.rs` | Uniform management, delegates to vertex.rs/fragment.rs/specular_lut.rs |
| `Rasterizer` | Struct | `rasterizer.rs` | Barycentric rasterization, `rasterize_each` (zero-alloc callback) |
| `Model` | Struct | `model.rs` | OBJ loader: vertices, faces, materials, texture cache |
| `Buffer` | Struct | `buffer.rs` | Double-buffer with flag-swap (no memcpy) |
| `TripleBufferWriter` | Struct | `triple_buffer.rs` | Render-thread half of lock-free triple buffer |
| `TripleBufferReader` | Struct | `triple_buffer.rs` | Display-thread half of lock-free triple buffer |
| `Vertex` / `VertexSoA` | Structs | `vertex.rs` | AoS for per-triangle path, SoA for tile-based path |
| `Fragment` | Struct | `fragment.rs` | Rasterizer output → fragment shader input |
| `UniformBuffer` | Struct | `uniform.rs` | `HashMap<String, UniformValue>` with typed getters |
| `uniform::names` | Module | `uniform.rs` | Type-safe string constants (MODEL_MATRIX, VIEW_MATRIX, etc.) |
| `Face` | Struct | `face.rs` | 3 vertex indices + `Arc<Material>` (shared ownership) |
| `Color` | Struct | `color.rs` | `[u8; 4]` RGBA, little-endian u32 compatible |
| `RendererError` | Enum | `error.rs` | `ModelLoad`, `TextureLoad`, `RenderFailed`, `Io` |

## ARCHITECTURE

Pipeline mirrors OpenGL: **Vertex Shader → Perspective Division → Viewport Transform → Rasterization → Fragment Shader → Depth Test → Framebuffer**.

- `Shader::vertex_shader(&self)` — sequential (Shader is not Sync); caches MVP, normal matrix
- `Rasterizer::rasterize_each(&self)` — zero-alloc callback-based, called within parallel outer loop
- `Shader::fragment_shader(&self)` — Blinn-Phong with specular LUT (`RwLock` for thread safety)
- Materials passed separately to fragment shader (not stored in Fragment) for thread safety
- Renderers own reusable buffers (`&mut self`); depth/color buffers reused across frames via `fill()`

Four renderer strategies share this pipeline but differ in scheduling — see `renderers/AGENTS.md`.

## CONVENTIONS

- **Math**: Right-handed coordinate system, OpenGL depth range `[-1, 1]`, Y-flipped in viewport transform
- **Error handling**: `thiserror` → `RendererError`, propagate with `?`, warn on non-fatal (material load failures)
- **Parallelism**: `rayon` everywhere — chunk-parallel renderers, tile-parallel tile renderers, parallel merge in PerTriangle
- **Uniform names**: Always use `uniform::names::*` constants — never raw string literals for uniform keys
- **Caching**: Shader caches derived matrices (MVP, normal) and light directions to avoid per-vertex/fragment HashMap lookups
- **Thread safety**: `Renderer: Send`, `RwLock` for specular LUT, `Arc<Material>` for shared face materials, reusable per-renderer buffers
- **Buffer reuse**: Renderer structs own depth/color buffers, reused each frame with `resize()+fill()` instead of `vec![]` allocation
- **Testing**: Every `.rs` file has inline `#[cfg(test)]` module; integration tests + proptest property tests
- **Platform**: Little-endian only (`color.rs` enforces via `compile_error!`)
- **MSRV**: Rust 1.73+ (uses `div_ceil`)

## COMMANDS

```bash
cargo build                              # Debug build
cargo build --release                    # Release build
cargo run -p system_test -- ./obj        # Run demo (teapot)
cargo test                               # All tests (223: unit + integration + property + system_test + doc)
cargo test -p simple_renderer            # Library unit tests only
cargo test --test integration_test       # Integration tests only
cargo test --test property_tests         # Property-based tests only
cargo clippy --workspace -- -D warnings  # Lint (must pass clean for CI)
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

### Dev Dependencies

| Crate | Purpose |
|-------|---------|
| proptest 1 | Property-based testing |
| criterion 0.5 | Benchmarking (with html_reports) |

## NOTES

- **Dev profile**: `opt-level = 2` globally, `opt-level = 3` for `image` and `tobj` (perf-critical deps)
- **Default mode**: `TileBased` (set in `SimpleRenderer::new`)
- **CI workflow**: `.github/workflows/workflow.yml` — Rust toolchain, build, test, clippy -D warnings
- **Integration tests**: Depend on `obj/utah-teapot-texture/teapot.obj` at runtime
- **Minimal `unsafe`**: Only `triple_buffer.rs` uses `unsafe` for `Send`/`Sync` impls on the lock-free buffer. All rendering logic is safe Rust
- **Endianness**: `Color` ↔ `u32` conversions assume little-endian; enforced by `compile_error!` at crate level
