[![codecov](https://codecov.io/gh/Simple-XX/SimpleRenderer/graph/badge.svg?token=J7NKK3SBNJ)](https://codecov.io/gh/Simple-XX/SimpleRenderer)
![workflow](https://github.com/Simple-XX/SimpleRenderer/actions/workflows/workflow.yml/badge.svg)
![commit-activity](https://img.shields.io/github/commit-activity/t/Simple-XX/SimpleRenderer)
![last-commit-main](https://img.shields.io/github/last-commit/Simple-XX/SimpleRenderer/main)
![MIT License](https://img.shields.io/github/license/mashape/apistatus.svg)
[![LICENSE](https://img.shields.io/badge/license-Anti%20996-blue.svg)](https://github.com/996icu/996.ICU/blob/master/LICENSE)
[![996.icu](https://img.shields.io/badge/link-996.icu-red.svg)](https://996.icu)

# SimpleRenderer

[![cn](https://img.shields.io/badge/language-Chinese-pink.svg)](https://github.com/Simple-XX/SimpleRenderer/blob/main/README-cn.md)
[![en](https://img.shields.io/badge/language-English-lightblue.svg)](https://github.com/Simple-XX/SimpleRenderer/blob/main/README.md)

An educational Rust software renderer designed to help developers understand the inner workings of rendering pipelines and how OpenGL operates behind the scenes.

## Overview

SimpleRenderer is a software renderer built with the primary goal of educating developers about the fundamentals of 3D rendering and graphics pipelines. By providing a simplified yet functional rendering framework, it demystifies the complex processes involved in rendering graphics, mirroring how OpenGL and other graphics APIs work under the hood.

The project is implemented in minimal `unsafe` Rust as a Cargo workspace with two crates: `simple_renderer` (the core library) and `system_test` (the interactive demo binary).

### Purpose

- **Educational Focus**: Designed to teach the core concepts of rendering, including vertex processing, rasterization, shading, and transformations.
- **Demystifying OpenGL**: Offers insights into how OpenGL and GPU-based rendering pipelines function internally.
- **Hands-On Learning**: Allows developers to experiment with rendering algorithms and observe the effects of various techniques in real-time.

### Key Features

- **Customizable Shaders**: Implemented vertex and fragment shaders to demonstrate how shading works at a fundamental level.
- **Simplified Rendering Pipeline**: Breaks down the rendering process into understandable stages, mirroring the OpenGL pipeline.
- **Four Rendering Strategies**: Choose between `PerTriangle`, `TileBased`, `Deferred`, and `TileBasedDeferred` rendering modes at runtime.
- **Blinn-Phong Shading**: Realistic lighting with ambient, diffuse, and specular components, including a specular LUT cache.
- **Multi-Buffered Framebuffer**: Lock-free triple buffering with dedicated render thread. Supports runtime switching between double-buffer (GPU-style VSync blocking) and triple-buffer (non-blocking) modes.
- **Parallel Rendering**: Uses [rayon](https://github.com/rayon-rs/rayon) for scanline-parallel rasterization and chunk/tile-parallel rendering strategies.
- **Multi-Threaded Architecture**: Dedicated render thread decoupled from input/display via lock-free triple buffer, simulating real GPU double/triple buffering behavior.
- **Minimal Unsafe**: Only the lock-free triple buffer uses `unsafe` for `Send`/`Sync` impls with well-documented safety invariants. All rendering logic is 100% safe Rust.
- **Cross-Platform Compatibility**: Compatible with Linux and macOS.

### Learning Objectives

By exploring SimpleRenderer, you will learn:

- How vertices are transformed from 3D space to 2D screen coordinates.
- The process of assembling primitives (triangles) and performing clipping.
- How rasterization converts vector information into pixels.
- The fundamentals of shading models, including lighting calculations.
- How depth buffering and backface culling optimize rendering.
- How double and triple buffering decouple rendering from display output.

---

## Getting Started

### Prerequisites

Ensure you have Rust and Cargo installed. The recommended way is via [rustup](https://rustup.rs/):

```bash
curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sh
```

No other system dependencies are required — all library dependencies are managed by Cargo.

### Building the Project

#### 1. Clone the Repository

```bash
git clone https://github.com/Simple-XX/SimpleRenderer.git
cd SimpleRenderer
```

#### 2. Build

```bash
cargo build                              # Debug build
cargo build --release                    # Release build
```

#### 3. Run the Example Application

```bash
cargo run -p system_test -- ./obj        # Run demo (teapot)
```

### Controls

| Key / Action | Effect |
|---|---|
| `1` / `2` / `3` / `4` | Switch rendering mode (PerTriangle / TileBased / Deferred / TileBasedDeferred) |
| `W` `A` `S` `D` | Move camera |
| Right-click drag | Rotate camera |
| `V` | Toggle VSync simulation (60 Hz) |
| `B` | Toggle buffer mode (double / triple) |

---

## Testing

```bash
cargo test                               # All tests
cargo test -p simple_renderer            # Library unit tests only
cargo test --test integration_test       # Integration tests only
```

The test suite includes 178 unit tests and 7 integration tests. Integration tests render the bundled Utah teapot model in all four rendering modes.

---

## Understanding the Renderer

### Core Rendering Pipeline

The rendering pipeline in SimpleRenderer is designed to mirror the stages of a typical GPU-based pipeline, providing a clear view of how each component contributes to the final rendered image.

1. **Vertex Processing and Transformations**

   - **Objective**: Understand how 3D models are projected onto a 2D screen.
   - **Key Concepts**:
     - **Model Matrix**: Positions and orients models in the world.
     - **View Matrix**: Represents the camera's position and orientation.
     - **Projection Matrix**: Defines the camera's lens (field of view, aspect ratio).

2. **Primitive Assembly and Clipping**

   - **Objective**: Learn how individual vertices form triangles and how off-screen parts are handled.
   - **Key Concepts**:
     - **Triangle Assembly**: Grouping vertices into drawable primitives.
     - **Clipping**: Discarding or adjusting primitives outside the view frustum.

3. **Rasterization and Fragment Processing**

   - **Objective**: Discover how triangles are converted into pixel data.
   - **Key Concepts**:
     - **Barycentric Coordinates**: Used for interpolating vertex attributes across a triangle.
     - **Depth Buffering**: Ensures correct rendering of overlapping objects.
     - **Fragment Shaders**: Calculate the color and other attributes of each pixel.

4. **Shading and Lighting Models**

   - **Objective**: Explore how lighting affects the appearance of surfaces.
   - **Key Concepts**:
     - **Blinn-Phong Shading Model**: Simulates realistic lighting with ambient, diffuse, and specular components.
     - **Surface Normals**: Determine how light interacts with surfaces.
     - **Light Sources**: Understand different types of lights (directional, point, ambient).

5. **Optimization Techniques**

   - **Objective**: Learn methods to improve rendering efficiency.
   - **Key Concepts**:
     - **Backface Culling**: Eliminates faces not visible to the camera.
     - **Parallel Rasterization**: Scanlines processed in parallel via rayon.
     - **Tile-Based Rendering**: Divides the screen into tiles for cache-friendly parallel processing.
     - **Multi-Buffering**: Lock-free triple buffer allows the render thread to work independently of the display refresh, avoiding pipeline stalls.

### Code Structure

The project is a Cargo workspace with two crates:

#### `simple_renderer/` — Core Library Crate

| File | Role |
|---|---|
| `src/renderer.rs` | `SimpleRenderer`: mode selection and `draw_model` entry point |
| `src/renderers/` | Four rendering strategies implementing the `Renderer` trait |
| `src/shader.rs` | Vertex and fragment shaders, uniform caching |
| `src/rasterizer.rs` | Barycentric interpolation, perspective-correct rasterization |
| `src/model.rs` | OBJ model loader (via tobj) with texture cache |
| `src/buffer.rs` | Double-buffered framebuffer (flag-swap, no copy) |
| `src/triple_buffer.rs` | Lock-free triple buffer: `TripleBufferWriter` + `TripleBufferReader` for multi-threaded rendering |
| `src/vertex.rs` | `Vertex` (AoS) and `VertexSoA` (SoA for tile renderers) |
| `src/fragment.rs` | Fragment data passed from rasterizer to fragment shader |
| `src/uniform.rs` | `UniformBuffer`: `HashMap`-based typed uniform storage |
| `src/material.rs` | `Material` and `Texture` (loaded via the image crate) |
| `src/math.rs` | Re-exports glam; `perspective_rh_gl`, `look_at_rh` helpers |
| `src/light.rs` | `Light` (name, position, direction, color) |
| `src/face.rs` | `Face` (3 vertex indices + `Arc<Material>`) |
| `src/color.rs` | 32-bit RGBA color, little-endian u32 compatible |
| `src/error.rs` | `RendererError` (thiserror) and `Result<T>` alias |

#### `system_test/` — Interactive Demo Binary

| File | Role |
|---|---|
| `src/main.rs` | Multi-threaded render loop: main thread (input/display) + render thread (shader/rasterizer) |
| `src/camera.rs` | FPS-style free camera (Euler angles) |
| `src/display.rs` | minifb window, keyboard/mouse input, rendering mode and buffer mode switching |

### Dependencies

| Crate | Purpose |
|---|---|
| glam 0.29 | Vec3, Vec4, Mat4 math |
| tobj 4.0 | OBJ model loading |
| image 0.25 | Texture loading (PNG, JPEG, BMP, TGA) |
| rayon 1.10 | Parallel iteration |
| minifb 0.27 | Window and display (system_test only) |
| thiserror 2 | Error derive macros |
| log 0.4 + env_logger 0.11 | Logging |

---

## Experimentation and Learning

To maximize learning, consider the following steps:

- **Modify Shaders**

  Experiment with the shader code in `simple_renderer/src/shader.rs` to see how changes affect rendering.

- **Adjust Transformations**

  Play with the model, view, and projection matrices to understand their impact on the scene.

- **Implement New Features**

  Try adding new lighting models, textures, or shading techniques.

- **Add a Rendering Strategy**

  Implement the `Renderer` trait in `simple_renderer/src/renderers/` and register it in `renderer.rs::create_renderer`.

---

## Contributions

Your contributions can help others learn. Feel free to:

- Submit pull requests with improvements or new educational features.
- Report issues or suggest enhancements.
- Share your learning experiences.

---

## License

This project is licensed under the MIT License. See the `LICENSE` file for more information.
