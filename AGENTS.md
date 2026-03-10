# PROJECT KNOWLEDGE BASE

**Generated:** 2026-03-11
**Branch:** main

## OVERVIEW

Educational software renderer mimicking OpenGL's GPU pipeline. The primary implementation is in **Rust**, with the original C++ version preserved under `cpp/`.

- **Rust** (primary): Workspace with `simple_renderer` library crate + `system_test` binary crate. Uses glam math, tobj model loading, rayon parallelism, minifb display.
- **C++** (legacy, in `cpp/`): C++23 static library with SDL2 display, GLM math, Assimp model loading, spdlog logging, OpenMP parallelism.

## STRUCTURE

```
SimpleRenderer/
├── Cargo.toml              # Rust workspace root
├── Cargo.lock
├── simple_renderer/        # Rust core library crate
│   ├── src/
│   │   ├── lib.rs
│   │   ├── renderers/      # Renderer strategy implementations
│   │   ├── shader.rs
│   │   ├── rasterizer.rs
│   │   ├── model.rs
│   │   ├── buffer.rs
│   │   ├── color.rs
│   │   ├── vertex.rs
│   │   ├── fragment.rs
│   │   ├── light.rs
│   │   ├── material.rs
│   │   ├── math.rs
│   │   ├── uniform.rs
│   │   ├── face.rs
│   │   └── error.rs
│   └── tests/              # Integration tests
├── system_test/            # Rust visual demo binary
│   └── src/
│       ├── main.rs
│       ├── camera.rs
│       └── display.rs
├── obj/                    # Bundled 3D models (.obj/.mtl) — shared
├── cpp/                    # Legacy C++ version
│   ├── CMakeLists.txt
│   ├── CMakePresets.json
│   ├── .clang-format
│   ├── .clang-tidy
│   ├── cmake/
│   ├── src/
│   ├── test/
│   ├── doc/
│   └── tools/
├── docs/                   # Plans and documentation
├── .github/                # CI workflows
├── LICENSE
├── README.md
└── README-cn.md
```

## WHERE TO LOOK (Rust)

| Task | Location | Notes |
|------|----------|-------|
| Add/modify rendering algorithm | `simple_renderer/src/renderers/` | Implement renderer trait, add to `mod.rs` |
| Change shader behavior | `simple_renderer/src/shader.rs` | Vertex/Fragment shaders, uniform handling |
| Modify rasterization | `simple_renderer/src/rasterizer.rs` | Barycentric interpolation, perspective correction |
| Add new model format | `simple_renderer/src/model.rs` | Uses tobj for OBJ loading |
| Change display/input | `system_test/src/display.rs` + `camera.rs` | minifb window, keyboard handling |
| Run the app | `system_test/src/main.rs` | `cargo run -p system_test -- ./obj` |
| Add integration tests | `simple_renderer/tests/` | `cargo test --test integration_test` |
| Add unit tests | Inline `#[cfg(test)]` modules | In each source file |
| Modify dependencies | `simple_renderer/Cargo.toml` or `system_test/Cargo.toml` | |

## WHERE TO LOOK (C++ legacy)

| Task | Location | Notes |
|------|----------|-------|
| C++ rendering code | `cpp/src/` + `cpp/src/include/` | Original C++ implementation |
| C++ build system | `cpp/CMakeLists.txt` + `cpp/cmake/` | CMake with CPM.cmake |
| C++ tests | `cpp/test/` | GoogleTest unit tests + system test |

## CODE MAP (Rust)

| Symbol | Type | Location | Role |
|--------|------|----------|------|
| `SimpleRenderer` | Struct | `simple_renderer/src/renderer.rs` | Mode selection + unified `draw_model` entry point |
| `PerTriangleRenderer` | Struct | `renderers/per_triangle.rs` | Forward rendering (traditional) |
| `TileBasedRenderer` | Struct | `renderers/tile_based.rs` | Tile binning + optional Early-Z |
| `DeferredRenderer` | Struct | `renderers/deferred.rs` | Deferred (collect fragments → shade winners) |
| `TileBasedDeferredRenderer` | Struct | `renderers/tile_based_deferred.rs` | TBDR: tile binning + 2-pass |
| `Shader` | Struct | `shader.rs` | Vertex/Fragment shaders, uniform buffer |
| `Rasterizer` | Struct | `rasterizer.rs` | Triangle rasterization, barycentric coords |
| `Model` | Struct | `model.rs` | tobj-based OBJ loader |
| `Vertex` | Struct | `vertex.rs` | Position, normal, UV, color |
| `Color` | Struct | `color.rs` | 32-bit RGBA color |
| `Buffer` | Struct | `buffer.rs` | Double-buffered framebuffer |
| `Fragment` | Struct | `fragment.rs` | Fragment shader input |

## CONVENTIONS (Rust)

- **Style**: Standard `rustfmt` formatting
- **Naming**: `snake_case` functions/variables, `PascalCase` types, `SCREAMING_SNAKE_CASE` constants
- **Error handling**: `thiserror` for custom errors, `Result<T, RendererError>` returns
- **Parallelism**: `rayon` for parallel iteration
- **Math**: `glam` crate (`Vec3`, `Vec4`, `Mat4`)
- **Testing**: `#[cfg(test)]` inline modules + `tests/` integration tests

## COMMANDS

```bash
# Build all (Rust)
cargo build

# Build release
cargo build --release

# Run demo
cargo run -p system_test -- ./obj

# Run all tests
cargo test

# Run integration tests only
cargo test --test integration_test

# Run unit tests for library
cargo test -p simple_renderer

# --- C++ legacy (from cpp/ directory) ---
# cmake --preset=build && cmake --build build --target all
```

## DEPENDENCIES (Rust)

| Crate | Purpose |
|-------|---------|
| glam (0.29) | Vector/matrix math |
| tobj (4.0) | OBJ model loading |
| image (0.25) | Texture loading (PNG, JPEG, BMP, TGA) |
| rayon (1.10) | Parallel rasterization |
| minifb (0.27) | Window/display (system_test) |
| log (0.4) | Logging facade |
| env_logger (0.11) | Logging backend |
| thiserror (2) | Error type derivation |

## NOTES

- **obj/ is shared**: 3D model assets at project root, used by both Rust and C++ versions
- **Four rendering modes**: `PerTriangle`, `TileBased`, `Deferred`, `TileBasedDeferred`
- **C++ version**: Preserved in `cpp/` for reference. See `cpp/` for its own build instructions.
- **CI**: May need updates for new structure
