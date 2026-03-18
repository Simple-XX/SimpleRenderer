![workflow](https://github.com/Simple-XX/SimpleGameEngine/actions/workflows/workflow.yml/badge.svg)
![MIT License](https://img.shields.io/github/license/mashape/apistatus.svg)

# SimpleGameEngine

[![cn](https://img.shields.io/badge/language-Chinese-pink.svg)](https://github.com/Simple-XX/SimpleGameEngine/blob/main/README.md)
[![en](https://img.shields.io/badge/language-English-lightblue.svg)](https://github.com/Simple-XX/SimpleGameEngine/blob/main/README_ENG.md)

A modular educational game engine built on a software renderer, helping developers understand rendering pipelines and game engine architecture.

## Overview

SimpleGameEngine is a modular game engine with education as its primary goal. It uses a layered architecture that decouples ECS, rendering, input, and scene management into independent crates. The built-in software renderer fully simulates the GPU rendering pipeline (vertex shading → rasterization → fragment shading → depth testing), helping developers understand 3D graphics from the ground up.

Implemented in minimal `unsafe` Rust as a Cargo workspace with 7 crates. MSRV: Rust 1.73+.

### Design Goals

- **Modular**: Each subsystem is an independent crate connected through trait interfaces
- **Educational**: Readable code; each module can be studied independently
- **Extensible**: Plugin system for third-party extensions; swappable render backends via RenderBackend trait

### Key Features

- **hecs ECS**: Archetype-based entity component system (~2000 lines of readable source)
- **Plugin Architecture**: Plugin trait + AppBuilder + Phase-based Schedule
- **Four Software Rendering Strategies**: PerTriangle / TileBased / Deferred / TileBasedDeferred
- **Blinn-Phong Shading**: Full vertex/fragment shader pipeline
- **RenderBackend Abstraction**: Unified render backend interface for future GPU backends
- **Multi-threaded Rendering**: Dedicated render thread with lock-free triple buffering
- **egui Editor**: Scene tree + properties panel + render viewport + FPS camera
- **Math Utilities**: AABB, Ray, Frustum geometry + glam linear algebra
- **Unified Input**: KeyboardState / MouseState with just_pressed / just_released tracking

---

## Architecture

### Layered Design

```
Layer 4 — Application
  engine_editor       egui editor GUI

Layer 2 — Infrastructure
  engine_scene        Scene + AssetManager + Components
  engine_render_sw    Software rendering backend

Layer 1 — Foundation (no inter-dependencies)
  engine_core         hecs ECS + Plugin + Schedule + EventBus + ResourceMap
  engine_math         glam re-export + AABB / Ray / Frustum
  engine_render_api   RenderBackend trait + FrameData + data types
  engine_input        Keyboard / Mouse state management
```

### Crate Overview

| Crate | Responsibility |
|-------|---------------|
| `engine_core` | hecs ECS, Plugin/Schedule/EventBus/ResourceMap, Transform hierarchy, Handle\<T\> |
| `engine_math` | glam re-export + AABB/Ray/Frustum geometry utilities |
| `engine_render_api` | RenderBackend trait, FrameData/DrawCall, MeshHandle/TextureHandle/MaterialHandle |
| `engine_input` | KeyCode/KeyboardState, MouseButton/MouseState, InputState |
| `engine_render_sw` | Software renderer: 4 rendering modes, Blinn-Phong shading, triple buffering |
| `engine_scene` | Scene (hecs World + AssetManager), MeshRenderer/Camera/Light components |
| `engine_editor` | eframe/egui editor: render viewport, scene tree, properties panel, FPS camera |

---

## Quick Start

### Prerequisites

Rust 1.73+ and Cargo:

```bash
curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sh
```

### Build & Run

```bash
git clone https://github.com/Simple-XX/SimpleGameEngine.git
cd SimpleGameEngine

cargo build --workspace               # Build all crates
cargo run -p engine_editor             # Run egui editor
```

### Controls

| Key / Action | Effect |
|---|---|
| `W` `A` `S` `D` | Move camera |
| Right-click drag | Rotate camera |
| `1` / `2` / `3` / `4` | Switch rendering mode |
| `V` | Toggle VSync simulation |
| `B` | Toggle buffer mode (double / triple) |

---

## Testing

```bash
cargo test --workspace                       # All 363 tests
cargo test -p engine_core                    # ECS + Plugin + Schedule (63)
cargo test -p engine_math                    # AABB / Ray / Frustum (39)
cargo test -p engine_input                   # Input state (20)
cargo test -p engine_render_api              # Render API types (13)
cargo test -p engine_render_sw               # Software renderer (207)
cargo test -p engine_scene                   # Scene management (5)
cargo clippy --workspace -- -D warnings      # Lint (CI requires zero warnings)
```

---

## Contributing

Pull requests, bug reports, and feedback are welcome.

## License

MIT License — see `LICENSE` file for details.
