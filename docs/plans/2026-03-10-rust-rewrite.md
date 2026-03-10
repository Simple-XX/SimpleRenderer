# Rust Rewrite Implementation Plan

> **For Claude:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** Rewrite the entire C++ software renderer as idiomatic Rust, with full feature parity across all 4 rendering modes.

**Architecture:** Cargo workspace with a headless library crate (`simple_renderer`) and a binary crate (`system_test`). The renderer trait replaces C++ virtual dispatch. Rayon replaces OpenMP for parallelism. Data types are redesigned as idiomatic Rust with `From`/`Into` conversions, `Result` error handling, and builder patterns where appropriate.

**Tech Stack:** glam (math), tobj (OBJ loading), image (texture loading), minifb (window), rayon (parallelism), log + env_logger (logging)

---

## C++ → Rust Dependency Mapping

| C++ Dependency | Rust Equivalent | Notes |
|---|---|---|
| GLM (glm::vec3, mat4) | glam (Vec2, Vec3, Vec4, Mat3, Mat4) | Near-identical API, SIMD optimized |
| Assimp | tobj | OBJ/MTL only; pure Rust, no C bindings |
| stb_image | image crate | Pure Rust image loading |
| SDL2 | minifb | Minimal framebuffer window, direct u32 pixel array |
| OpenMP | rayon | `par_iter()` replaces `#pragma omp parallel for` |
| spdlog | log + env_logger | Standard Rust logging facade |
| std::variant | Rust enum | `UniformValue` becomes a proper enum |
| virtual dispatch | `dyn Trait` / `Box<dyn Renderer>` | Trait objects replace inheritance |
| std::shared_mutex | `RwLock<HashMap>` | Specular LUT cache |

## C++ → Rust Type Mapping

| C++ Type | Rust Type |
|---|---|
| `Vector2f` (glm::vec2) | `glam::Vec2` |
| `Vector3f` (glm::vec3) | `glam::Vec3` |
| `Vector4f` (glm::vec4) | `glam::Vec4` |
| `Matrix3f` (glm::mat3) | `glam::Mat3` |
| `Matrix4f` (glm::mat4) | `glam::Mat4` |
| `uint32_t*` buffer | `&mut [u32]` slice |
| `std::vector<T>` | `Vec<T>` |
| `std::array<T, N>` | `[T; N]` |
| `std::optional<T>` | `Option<T>` |
| `std::unique_ptr<T>` | `Box<T>` |
| `std::shared_ptr<T>` | `Arc<T>` |
| `size_t` | `usize` |
| `uint8_t` | `u8` |
| `int32_t` | `i32` |
| `float` | `f32` |

## Project Structure

```
rust/
├── Cargo.toml                          # Workspace root
├── simple_renderer/                    # Library crate (headless renderer)
│   ├── Cargo.toml
│   └── src/
│       ├── lib.rs                      # Public API exports
│       ├── color.rs                    # 32-bit RGBA color
│       ├── math.rs                     # Re-exports of glam types + helpers
│       ├── vertex.rs                   # Vertex (AoS) + VertexSoA
│       ├── face.rs                     # Triangle face with indices + material
│       ├── light.rs                    # Light source
│       ├── material.rs                 # Material + Texture
│       ├── model.rs                    # OBJ model loader (tobj)
│       ├── buffer.rs                   # Double-buffered framebuffer
│       ├── fragment.rs                 # Fragment struct (shader input)
│       ├── uniform.rs                  # UniformBuffer + UniformValue enum
│       ├── shader.rs                   # Vertex/fragment shaders + caching
│       ├── rasterizer.rs              # Triangle rasterization
│       ├── renderer.rs                 # SimpleRenderer facade + RenderingMode enum
│       ├── error.rs                    # Error types (thiserror)
│       └── renderers/
│           ├── mod.rs                  # Renderer trait definition
│           ├── base.rs                 # Shared helper functions (perspective division, viewport transform)
│           ├── per_triangle.rs         # AoS forward rendering
│           ├── tile_based.rs           # SoA tile-binned forward rendering
│           ├── deferred.rs             # AoS deferred rendering
│           └── tile_based_deferred.rs  # SoA tile-binned deferred rendering
└── system_test/                        # Binary crate (demo app)
    ├── Cargo.toml
    └── src/
        ├── main.rs                     # Entry point + render loop
        ├── camera.rs                   # Camera with view/projection matrices
        └── display.rs                  # minifb window wrapper
```

---

## Phase 1: Project Setup & Core Types

### Task 1: Scaffold Cargo Workspace

**Files:**
- Create: `rust/Cargo.toml`
- Create: `rust/simple_renderer/Cargo.toml`
- Create: `rust/simple_renderer/src/lib.rs`
- Create: `rust/system_test/Cargo.toml`
- Create: `rust/system_test/src/main.rs`

**Step 1: Create workspace root Cargo.toml**

```toml
[workspace]
members = ["simple_renderer", "system_test"]
resolver = "2"
```

**Step 2: Create library crate Cargo.toml**

```toml
[package]
name = "simple_renderer"
version = "0.1.0"
edition = "2021"

[dependencies]
glam = "0.29"
tobj = { version = "4.0", features = ["async"] }
image = { version = "0.25", default-features = false, features = ["png", "jpeg", "bmp", "tga"] }
rayon = "1.10"
log = "0.4"
thiserror = "2"

[dev-dependencies]
env_logger = "0.11"
```

**Step 3: Create binary crate Cargo.toml**

```toml
[package]
name = "system_test"
version = "0.1.0"
edition = "2021"

[dependencies]
simple_renderer = { path = "../simple_renderer" }
minifb = "0.27"
log = "0.4"
env_logger = "0.11"
glam = "0.29"
```

**Step 4: Create minimal lib.rs and main.rs**

`simple_renderer/src/lib.rs`:
```rust
pub mod color;
```

`system_test/src/main.rs`:
```rust
fn main() {
    println!("SimpleRenderer Rust");
}
```

**Step 5: Verify build**

Run: `cargo build` in `rust/`
Expected: Successful compilation

**Step 6: Commit**

```bash
git add rust/
git commit -m "feat: scaffold Rust workspace with lib and bin crates"
```

---

### Task 2: Color Type

**Port from:** `src/include/color.h` + `src/color.cpp`

**Files:**
- Create: `rust/simple_renderer/src/color.rs`
- Modify: `rust/simple_renderer/src/lib.rs`

**Key design decisions:**
- `Color` is `#[derive(Clone, Copy)]` (4 bytes, like C++ version)
- Memory layout: `[r, g, b, a]` as `[u8; 4]` (matches C++ field order)
- Implement `From<u32>`, `Into<u32>`, `ops::Mul<f32>`, `ops::MulAssign<f32>`, `ops::Add`
- Provide named constants: `Color::WHITE`, `Color::BLACK`, `Color::RED`, etc.
- Constructor from floats clamps to [0, 255]

**Step 1: Write tests**

```rust
#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_color_from_rgba() {
        let c = Color::new(255, 128, 0, 255);
        assert_eq!(c.r(), 255);
        assert_eq!(c.g(), 128);
        assert_eq!(c.b(), 0);
        assert_eq!(c.a(), 255);
    }

    #[test]
    fn test_color_from_u32() {
        let c = Color::new(0xAA, 0xBB, 0xCC, 0xFF);
        let val: u32 = c.into();
        let c2 = Color::from(val);
        assert_eq!(c, c2);
    }

    #[test]
    fn test_color_mul_float() {
        let c = Color::new(200, 100, 50, 255);
        let result = c * 0.5;
        assert_eq!(result.r(), 100);
        assert_eq!(result.g(), 50);
        assert_eq!(result.b(), 25);
    }

    #[test]
    fn test_color_add() {
        let c1 = Color::new(100, 50, 25, 255);
        let c2 = Color::new(50, 100, 75, 0);
        let result = c1 + c2;
        assert_eq!(result.r(), 150);
        assert_eq!(result.g(), 150);
        assert_eq!(result.b(), 100);
    }

    #[test]
    fn test_color_constants() {
        assert_eq!(Color::WHITE, Color::new(255, 255, 255, 255));
        assert_eq!(Color::BLACK, Color::new(0, 0, 0, 255));
    }

    #[test]
    fn test_color_from_floats() {
        let c = Color::from_f32(1.0, 0.5, 0.0, 1.0);
        assert_eq!(c.r(), 255);
        assert_eq!(c.g(), 128);  // 0.5 * 255 ≈ 127.5, rounded
        assert_eq!(c.b(), 0);
    }
}
```

**Step 2: Implement Color**

Reference: `src/include/color.h` (lines 30-155) and `src/color.cpp` (lines 1-126).

The C++ Color stores RGBA as 4 separate `uint8_t` fields. The `operator uint32_t()` casts the struct's memory directly to `uint32_t` (little-endian: R in low byte). Reproduce this layout in Rust.

Key methods to implement:
- `new(r, g, b, a)` — from u8 components
- `from_f32(r, g, b, a)` — from [0.0, 1.0] floats (C++ `Color(float, float, float, float)`)
- `r()`, `g()`, `b()`, `a()` — getters
- `From<u32>` / `Into<u32>` — matching C++ `operator uint32_t()`
- `ops::Mul<f32>` — per-channel multiply (C++ `operator*`)
- `ops::Add<Color>` — per-channel add (C++ `operator+`)
- `Index<usize>` — channel access by index (C++ `operator[]`)
- Constants: `WHITE`, `BLACK`, `RED`, `GREEN`, `BLUE`

**Step 3: Run tests**

Run: `cargo test -p simple_renderer -- color`
Expected: All pass

**Step 4: Commit**

---

### Task 3: Math Helpers Module

**Port from:** `src/include/math.hpp`

**Files:**
- Create: `rust/simple_renderer/src/math.rs`
- Modify: `rust/simple_renderer/src/lib.rs`

**Design:** The C++ `math.hpp` defines type aliases (`Vector3f = glm::vec3`, etc.) and spdlog formatters. In Rust, re-export glam types and add helper functions.

```rust
// Re-export glam types for consistent naming
pub use glam::{Mat3, Mat4, Vec2, Vec3, Vec4};

/// Helper to create a perspective projection matrix
pub fn perspective(fov_y_radians: f32, aspect_ratio: f32, near: f32, far: f32) -> Mat4 {
    Mat4::perspective_rh_gl(fov_y_radians, aspect_ratio, near, far)
}

/// Helper to create a look-at view matrix
pub fn look_at(eye: Vec3, center: Vec3, up: Vec3) -> Mat4 {
    Mat4::look_at_rh(eye, center, up)
}
```

Note: glam uses right-handed coordinate system by default. The C++ uses GLM which also defaults to right-handed. Use `_rh_gl` variants to match OpenGL conventions.

**Step: Implement and verify**

Run: `cargo test -p simple_renderer -- math`

---

### Task 4: Light Type

**Port from:** `src/include/light.h` + `src/light.cpp`

**Files:**
- Create: `rust/simple_renderer/src/light.rs`

**Design:** Simple struct with public fields (matches C++ public members).

```rust
use crate::color::Color;
use crate::math::Vec3;

#[derive(Debug, Clone)]
pub struct Light {
    pub name: String,
    pub position: Vec3,
    pub direction: Vec3,
    pub color: Color,
}

impl Default for Light {
    fn default() -> Self {
        Self {
            name: "default light".to_string(),
            position: Vec3::new(0.0, 0.0, 0.0),
            direction: Vec3::new(0.0, 0.0, 1.0), // C++ kDefaultDir
            color: Color::WHITE,
        }
    }
}
```

Reference: `src/include/light.h` (lines 31-65), `src/light.cpp` (lines 1-33).

---

### Task 5: Material & Texture Types

**Port from:** `src/include/material.hpp` + `src/material.cpp`

**Files:**
- Create: `rust/simple_renderer/src/material.rs`

**Key design decisions:**
- `Texture` owns its pixel data as `Vec<u8>` (C++ uses raw `uint8_t*` from stb_image)
- `Texture::load_from_file()` uses the `image` crate instead of stb_image
- `Texture::get_pixel(x, y) -> Color` for texture sampling
- `Material` is a plain struct with optional textures via `Option<Texture>`

Reference: `src/include/material.hpp` (lines 11-105), `src/material.cpp` (lines 1-52).

C++ `Material` uses boolean flags (`has_ambient_texture`, etc.) + always-present Texture objects. In Rust, use `Option<Texture>` (more idiomatic, eliminates flags).

Tests:
- Load a test texture file, verify dimensions and pixel sampling
- Create default Material, verify fields

---

### Task 6: Vertex Types

**Port from:** `src/include/vertex.hpp`

**Files:**
- Create: `rust/simple_renderer/src/vertex.rs`

**Design:**

```rust
/// AoS vertex (used by PerTriangle and Deferred renderers)
#[derive(Debug, Clone, Copy)]
pub struct Vertex {
    pub position: Vec4,
    pub normal: Vec3,
    pub tex_coords: Vec2,
    pub color: Color,
    pub clip_position: Option<Vec4>,
}

/// SoA vertex layout (used by TileBased renderers for cache efficiency)
#[derive(Debug, Clone, Default)]
pub struct VertexSoA {
    pub pos_screen: Vec<Vec4>,
    pub pos_clip: Vec<Vec4>,
    pub normal: Vec<Vec3>,
    pub uv: Vec<Vec2>,
    pub color: Vec<Color>,
}
```

Reference: `src/include/vertex.hpp` (lines 13-93).

Key differences from C++:
- No operator overloading for `Matrix4f * Vertex`; use a method `Vertex::transform(&self, mat: &Mat4) -> Vertex` instead
- `clip_position` is `Option<Vec4>` (was `std::optional<Vector4f>`)
- Fields are public (Rust convention for data structs)

---

### Task 7: Face Type

**Port from:** `src/include/face.hpp`

**Files:**
- Create: `rust/simple_renderer/src/face.rs`

```rust
use crate::material::Material;

#[derive(Debug, Clone)]
pub struct Face {
    pub indices: [usize; 3],
    pub material: Material,
}
```

Reference: `src/include/face.hpp` (lines 11-49).

---

### Task 8: Fragment Type

**Port from:** `src/include/shader.hpp` (Fragment struct)

**Files:**
- Create: `rust/simple_renderer/src/fragment.rs`

```rust
#[derive(Debug, Clone)]
pub struct Fragment {
    pub screen_coord: [i32; 2],
    pub normal: Vec3,
    pub uv: Vec2,
    pub color: Color,
    pub depth: f32,
    pub material_index: usize, // Index into face materials (avoid raw pointer)
}
```

Key difference: C++ uses `const Material*` raw pointer. In Rust, use an index or reference with lifetime. For simplicity and thread safety, use `material_index` + pass materials separately, or use `Arc<Material>`. Decide during implementation based on ergonomics.

---

### Task 9: Error Types

**Files:**
- Create: `rust/simple_renderer/src/error.rs`

```rust
use thiserror::Error;

#[derive(Debug, Error)]
pub enum RendererError {
    #[error("Model loading failed: {0}")]
    ModelLoad(String),
    #[error("Texture loading failed: {0}")]
    TextureLoad(String),
    #[error("Rendering failed: {0}")]
    RenderFailed(String),
    #[error("IO error: {0}")]
    Io(#[from] std::io::Error),
}

pub type Result<T> = std::result::Result<T, RendererError>;
```

---

## Phase 2: Model Loading

### Task 10: Model Loader (tobj)

**Port from:** `src/model.cpp` + `src/include/model.hpp`

**Files:**
- Create: `rust/simple_renderer/src/model.rs`

**Design:**
- `Model::load(path)` returns `Result<Model>`
- Uses `tobj::load_obj()` to parse OBJ + MTL files
- Converts tobj meshes → `Vec<Vertex>` + `Vec<Face>`
- Loads textures via `image` crate → `Texture`
- Texture cache via `HashMap<PathBuf, Texture>`

Reference: `src/model.cpp` (lines 1-207).

The C++ code does:
1. `Assimp::Importer` loads the file
2. `ProcessNode()` recurses through scene graph
3. `ProcessMesh()` extracts vertices (position, normal, UV, color) and face indices
4. `ProcessMaterial()` reads Phong properties (ambient/diffuse/specular/shininess) and textures

tobj equivalent:
1. `tobj::load_obj(path, &tobj::GPU_LOAD_OPTIONS)` returns (models, materials)
2. For each model: mesh positions/normals/texcoords → Vertex structs
3. Face indices from mesh.indices (every 3 = one triangle)
4. Materials from material properties (Kd, Ks, Ka, Ns, map_Kd, etc.)

Tests:
- Load `obj/utah-teapot-texture/teapot.obj` and verify vertex count > 0, face count > 0
- Verify materials are loaded with correct properties

---

## Phase 3: Buffer & Rasterizer

### Task 11: Double-Buffered Framebuffer

**Port from:** `src/include/buffer.hpp`

**Files:**
- Create: `rust/simple_renderer/src/buffer.rs`

**Design:**

```rust
pub struct Buffer {
    width: usize,
    height: usize,
    framebuffer_1: Vec<u32>,
    framebuffer_2: Vec<u32>,
    draw_is_first: bool, // tracks which buffer is the draw buffer
}

impl Buffer {
    pub fn new(width: usize, height: usize) -> Self { ... }
    pub fn clear_draw_buffer(&mut self, color: Color) { ... }
    pub fn swap(&mut self) { ... }
    pub fn draw_buffer(&self) -> &[u32] { ... }
    pub fn draw_buffer_mut(&mut self) -> &mut [u32] { ... }
    pub fn display_buffer(&self) -> &[u32] { ... }
    pub fn width(&self) -> usize { ... }
    pub fn height(&self) -> usize { ... }
}
```

Reference: `src/include/buffer.hpp` (lines 11-78).

Key difference: C++ uses raw `uint32_t*` swapping. Rust uses `Vec<u32>` with a boolean flag to track which is draw vs display.

---

### Task 12: Rasterizer

**Port from:** `src/rasterizer.cpp` + `src/include/rasterizer.hpp`

**Files:**
- Create: `rust/simple_renderer/src/rasterizer.rs`

**Design:**

```rust
pub struct Rasterizer {
    width: usize,
    height: usize,
}

impl Rasterizer {
    pub fn new(width: usize, height: usize) -> Self { ... }

    /// Rasterize a triangle into fragments
    pub fn rasterize(&self, v0: &Vertex, v1: &Vertex, v2: &Vertex) -> Vec<Fragment> { ... }
}
```

**Algorithm (port exactly from C++):**

1. Compute screen-space bounding box from v0, v1, v2 positions
2. Clamp bbox to screen dimensions
3. For each pixel (x, y) in bbox:
   a. Compute barycentric coordinates via cross product method (`get_barycentric_coord`)
   b. If not inside triangle → skip
   c. Perform perspective correction (`perform_perspective_correction`):
      - Interpolate 1/w (linear in screen space)
      - Compute corrected barycentric = original_bary * (1/w_i) / interpolated_1/w
      - Interpolate depth z with corrected barycentric
   d. Interpolate normal, UV, color using corrected barycentric
   e. Emit Fragment

Reference: `src/rasterizer.cpp` (lines 1-176).

**Parallelism:** The C++ uses OpenMP to parallelize the pixel loop. In Rust, use `rayon::par_iter()` over the pixel range. Collect fragments from each thread into thread-local Vecs, then flatten.

Tests:
- Rasterize a known triangle, verify fragments are inside triangle bounds
- Verify barycentric coordinates sum to ~1.0
- Test degenerate triangle (zero area) returns empty fragments

---

## Phase 4: Shader System

### Task 13: Uniform Buffer

**Port from:** `src/include/shader.hpp` (UniformBuffer, UniformValue)

**Files:**
- Create: `rust/simple_renderer/src/uniform.rs`

**Design:**

```rust
/// Typed uniform value (replaces C++ std::variant)
#[derive(Debug, Clone)]
pub enum UniformValue {
    Int(i32),
    Float(f32),
    Vec2(Vec2),
    Vec3(Vec3),
    Vec4(Vec4),
    Mat3(Mat3),
    Mat4(Mat4),
    Material(Material),
    Light(Light),
    Lights(Vec<Light>),
}

pub struct UniformBuffer {
    uniforms: HashMap<String, UniformValue>,
}

impl UniformBuffer {
    pub fn set<T: Into<UniformValue>>(&mut self, name: &str, value: T) { ... }
    pub fn get<T: TryFrom<UniformValue>>(&self, name: &str) -> Option<T> { ... }
    pub fn has(&self, name: &str) -> bool { ... }
}
```

Implement `From<T>` for `UniformValue` for each supported type. Implement `TryFrom<UniformValue>` for each type for extraction.

Reference: `src/include/shader.hpp` (lines 17-49).

---

### Task 14: Shader (Vertex + Fragment)

**Port from:** `src/shader.cpp` + `src/include/shader.hpp`

**Files:**
- Create: `rust/simple_renderer/src/shader.rs`

**Design:**

```rust
pub struct Shader {
    uniform_buffer: UniformBuffer,
    shared_data: SharedDataInShader,
    vertex_cache: VertexUniformCache,
    fragment_cache: FragmentUniformCache,
    specular_lut_cache: RwLock<HashMap<u32, SpecularLut>>,
}
```

**Key methods:**

1. `vertex_shader(&mut self, vertex: &Vertex) -> Vertex`
   - Reference: `src/shader.cpp` lines 54-101
   - Applies MVP transform, computes clip position, transforms normal
   - Uses cached matrices when available

2. `fragment_shader(&self, fragment: &Fragment, material: &Material) -> Color`
   - Reference: `src/shader.cpp` lines 289-365
   - Phong shading: ambient + diffuse + specular
   - Texture sampling when available
   - Specular LUT for pow() optimization

3. `set_uniform(&mut self, name: &str, value: impl Into<UniformValue>)`
   - Updates uniform buffer + invalidates caches

4. `prepare_caches(&mut self)`
   - Pre-computes derived matrices and light directions
   - Reference: `src/shader.cpp` lines 183-237

5. Internal: `build_specular_lut(shininess)`, `evaluate_specular(cos_theta, shininess)`
   - Reference: `src/shader.cpp` lines 239-287
   - Thread-safe LUT cache with `RwLock`

6. Internal: `sample_texture(texture, uv) -> Color`
   - Reference: `src/shader.cpp` lines 376-395
   - UV wrapping to [0, 1], pixel sampling

**Note on thread safety:** The C++ uses `mutable shared_mutex` for the specular LUT cache because `FragmentShader` is `const` but needs to mutate the cache. In Rust, use `RwLock<HashMap<u32, SpecularLut>>` — `fragment_shader` takes `&self` and the `RwLock` allows interior mutability.

Tests:
- Vertex shader with identity matrices → position unchanged
- Fragment shader with single light → produces non-zero color
- Specular LUT produces correct values for known inputs
- Texture sampling wraps UV correctly

---

## Phase 5: Renderer Trait & Helpers

### Task 15: Renderer Trait + Base Helpers

**Port from:** `src/include/renderers/renderer_base.hpp` + `src/renderers/renderer_base.cpp`

**Files:**
- Create: `rust/simple_renderer/src/renderers/mod.rs`
- Create: `rust/simple_renderer/src/renderers/base.rs`

**Design:**

```rust
// mod.rs — trait definition
pub trait Renderer: Send {
    fn render(
        &self,
        model: &Model,
        shader: &Shader,
        out_buffer: &mut [u32],
        width: usize,
        height: usize,
    ) -> bool;
}

// base.rs — shared helper functions (not a base class!)
/// Perspective division: clip space → NDC
pub fn perspective_division(vertex: &Vertex) -> Vertex { ... }

/// Viewport transform: NDC → screen coordinates
pub fn viewport_transform(vertex: &Vertex, width: usize, height: usize) -> Vertex { ... }
```

Reference: `src/renderers/renderer_base.cpp` (lines 1-44).

The C++ `RendererBase` is an abstract class with protected methods. In Rust, these become free functions in a `base` module that renderers import.

`PerspectiveDivision` algorithm:
- Divide x, y, z by w (guard against w ≈ 0 with `kMinWValue = 1e-6`)
- Store 1/w in the w component for later perspective correction

`ViewportTransformation` algorithm:
- x_screen = (x_ndc + 1) * width / 2
- y_screen = (1 - y_ndc) * height / 2 (note: Y is flipped from NDC to screen)
- z_screen = z_ndc (preserved for depth testing)

---

## Phase 6: Per-Triangle Renderer

### Task 16: PerTriangleRenderer

**Port from:** `src/renderers/per_triangle_renderer.cpp`

**Files:**
- Create: `rust/simple_renderer/src/renderers/per_triangle.rs`

**Algorithm (exact port):**

1. **Copy shader** + prepare uniform caches
2. **Vertex transform** (parallel via rayon):
   - For each vertex: `vertex_shader` → `perspective_division` → `viewport_transform`
3. **Allocate per-thread buffers**: depth (`Vec<f32>` filled with `f32::INFINITY`) + color (`Vec<u32>` filled with 0)
4. **Parallel rasterization** (rayon over faces):
   - Backface culling via screen-space cross product (skip if cross > 0)
   - Rasterize triangle → fragments
   - For each fragment: depth test → fragment shader → write to thread-local buffer
5. **Merge thread buffers**: for each pixel, find min depth across all threads → write winner to output

Reference: `src/renderers/per_triangle_renderer.cpp` (lines 1-173).

**Rayon pattern:**

```rust
use rayon::prelude::*;

// Vertex processing
let processed: Vec<Vertex> = model.vertices()
    .par_iter()
    .map(|v| {
        let clip = shader.vertex_shader(v);
        let ndc = base::perspective_division(&clip);
        base::viewport_transform(&ndc, width, height)
    })
    .collect();

// Face rasterization (into thread-local buffers)
// Use rayon's par_iter with thread_local! or par_bridge
```

Tests:
- Render a simple triangle, verify non-zero output pixels
- Verify backface culling (reversed winding produces no output)

---

## Phase 7: Tile-Based Renderer

### Task 17: TileBasedRenderer

**Port from:** `src/renderers/tile_based_renderer.cpp` + `src/include/renderers/tile_based_renderer.hpp`

**Files:**
- Create: `rust/simple_renderer/src/renderers/tile_based.rs`

**Key data structures:**

```rust
struct TileTriangleRef {
    soa_indices: [usize; 3], // Indices into VertexSoA
    material_index: usize,
}

struct TileGridContext {
    soa: VertexSoA,
    tiles_x: usize,
    tiles_y: usize,
    tile_size: usize,
    tile_bins: Vec<Vec<TileTriangleRef>>, // one bin per tile
}
```

**Algorithm:**

1. **Vertex processing** → `VertexSoA` (SoA layout for cache efficiency)
2. **Tile binning** (two-pass):
   - Pass 1: Count triangles per tile (by computing which tiles each triangle's bbox overlaps)
   - Pass 2: Fill tile bins with triangle references
3. **Parallel tile rendering** (rayon over tiles):
   - For each tile, for each triangle in bin:
     - Optional Early-Z pre-pass (depth-only, no shading)
     - Full rasterization within tile bounds
     - Depth test + fragment shade
4. **Write results** to output buffer

Reference: `src/renderers/tile_based_renderer.cpp` (lines 1-530).

Configuration options:
- `early_z_enabled: bool` (default true)
- `tile_size: usize` (default 64)

---

## Phase 8: Deferred Renderer

### Task 18: DeferredRenderer

**Port from:** `src/renderers/deferred_renderer.cpp`

**Files:**
- Create: `rust/simple_renderer/src/renderers/deferred.rs`

**Algorithm:**

1. **Vertex processing** (same as PerTriangle: AoS)
2. **Fragment collection** (parallel):
   - Rasterize all faces → collect all fragments
   - Per-thread fragment vectors, merged afterward
3. **Depth resolve**:
   - For each pixel, find the fragment with minimum depth
4. **Deferred shading**:
   - Only shade the winning (closest) fragment per pixel
   - Write result to output buffer

Reference: `src/renderers/deferred_renderer.cpp` (lines 1-177).

Key difference from PerTriangle: shading is deferred until after depth testing is complete for ALL triangles. This mimics GPU deferred rendering.

---

## Phase 9: Tile-Based Deferred Renderer

### Task 19: TileBasedDeferredRenderer

**Port from:** `src/renderers/tile_based_deferred_renderer.cpp`

**Files:**
- Create: `rust/simple_renderer/src/renderers/tile_based_deferred.rs`

**Algorithm (2-pass):**

1. **Vertex processing** → `VertexSoA`
2. **Tile binning** (same as TileBased)
3. **Pass 1 — Z-prepass** (parallel over tiles):
   - Rasterize all triangles in tile, record only depth → build depth buffer
4. **Pass 2 — Deferred shade** (parallel over tiles):
   - Re-rasterize, shade only fragments matching depth buffer winners
   - Write final color to output

Reference: `src/renderers/tile_based_deferred_renderer.cpp` (lines 1-435).

This combines tile-based binning with deferred shading, mimicking mobile GPU TBDR architecture.

---

## Phase 10: SimpleRenderer Facade

### Task 20: SimpleRenderer + RenderingMode

**Port from:** `src/renderer.cpp` + `src/include/renderer.h`

**Files:**
- Create: `rust/simple_renderer/src/renderer.rs`
- Modify: `rust/simple_renderer/src/lib.rs` (final public API)

**Design:**

```rust
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum RenderingMode {
    PerTriangle,
    TileBased,
    Deferred,
    TileBasedDeferred,
}

pub struct SimpleRenderer {
    width: usize,
    height: usize,
    mode: RenderingMode,
    renderer: Box<dyn Renderer>,
    // TBR config cache
    early_z_enabled: bool,
    tile_size: usize,
}

impl SimpleRenderer {
    pub fn new(width: usize, height: usize) -> Self { ... }
    pub fn draw_model(&self, model: &Model, shader: &Shader, buffer: &mut [u32]) -> bool { ... }
    pub fn set_rendering_mode(&mut self, mode: RenderingMode) { ... }
    pub fn rendering_mode(&self) -> RenderingMode { ... }
    pub fn set_early_z_enabled(&mut self, enabled: bool) { ... }
    pub fn set_tile_size(&mut self, size: usize) { ... }
}
```

Reference: `src/renderer.cpp` (lines 1-90), `src/include/renderer.h` (lines 38-111).

`set_rendering_mode()` reconstructs the internal `Box<dyn Renderer>` (matches C++ `EnsureRenderer()`).

---

### Task 21: Public API (lib.rs)

Finalize `lib.rs` with all public re-exports:

```rust
pub mod color;
pub mod math;
pub mod light;
pub mod material;
pub mod vertex;
pub mod face;
pub mod fragment;
pub mod uniform;
pub mod shader;
pub mod rasterizer;
pub mod buffer;
pub mod renderer;
pub mod renderers;
pub mod model;
pub mod error;

// Convenience re-exports
pub use color::Color;
pub use renderer::{SimpleRenderer, RenderingMode};
pub use shader::Shader;
pub use model::Model;
pub use buffer::Buffer;
pub use light::Light;
pub use vertex::Vertex;
pub use error::{RendererError, Result};
```

---

## Phase 11: Demo Application

### Task 22: Camera

**Port from:** `test/system_test/camera.h`

**Files:**
- Create: `rust/system_test/src/camera.rs`

```rust
use glam::{Mat4, Vec3};

pub struct Camera {
    position: Vec3,
    front: Vec3,
    up: Vec3,
}

impl Camera {
    pub fn new(position: Vec3) -> Self { ... }
    pub fn view_matrix(&self) -> Mat4 { ... }
    pub fn projection_matrix(&self, fov_deg: f32, aspect: f32, near: f32, far: f32) -> Mat4 { ... }
    pub fn move_forward(&mut self, distance: f32) { ... }
    pub fn move_right(&mut self, distance: f32) { ... }
    pub fn move_up(&mut self, distance: f32) { ... }
    pub fn rotate(&mut self, yaw: f32, pitch: f32) { ... }
    pub fn position(&self) -> Vec3 { ... }
}
```

Reference: `test/system_test/camera.h` (lines 34-109).

---

### Task 23: Display (minifb)

**Port from:** `test/system_test/display.cpp` + `test/system_test/display.h`

**Files:**
- Create: `rust/system_test/src/display.rs`

**Design:**

```rust
use minifb::{Key, Window, WindowOptions};

pub struct Display {
    window: Window,
    width: usize,
    height: usize,
}

impl Display {
    pub fn new(width: usize, height: usize) -> Self { ... }
    pub fn is_open(&self) -> bool { ... }
    pub fn update(&mut self, buffer: &[u32]) { ... }
    pub fn handle_input(&self, camera: &mut Camera) { ... }
}
```

minifb provides a much simpler API than SDL2:
- `Window::new(title, width, height, opts)` creates window
- `window.update_with_buffer(buffer, width, height)` displays pixels
- `window.is_key_down(Key)` checks input

Reference: `test/system_test/display.cpp` (lines 1-127).

Hotkey mapping:
- `Key::Escape` / `Key::Q` → exit
- `Key::Up` / `Key::Down` → `camera.move_up(±1.0)`
- `Key::Left` / `Key::Right` → `camera.move_right(±1.0)`

---

### Task 24: Main Entry Point

**Port from:** `test/system_test/main.cpp`

**Files:**
- Modify: `rust/system_test/src/main.rs`

```rust
fn main() {
    env_logger::init();

    let obj_path = std::env::args().nth(1).expect("Usage: system_test <obj_dir>");

    let width = 800;
    let height = 600;

    let mut buffer = simple_renderer::Buffer::new(width, height);
    let mut renderer = simple_renderer::SimpleRenderer::new(width, height);

    let model = simple_renderer::Model::load(&format!("{}/utah-teapot-texture/teapot.obj", obj_path))
        .expect("Failed to load model");

    // Model matrix: scale + translate + rotate (same as C++)
    let model_matrix = Mat4::from_scale(Vec3::splat(0.02))
        * Mat4::from_translation(Vec3::new(0.0, -5.0, 0.0))
        * Mat4::from_rotation_x((-105.0_f32).to_radians());

    let mut shader = simple_renderer::Shader::new();
    shader.set_uniform("modelMatrix", model_matrix);

    // Multi-light setup (same as C++)
    let lights = vec![
        Light { direction: Vec3::new(1.0, 5.0, 1.0), ..Default::default() },
        Light { direction: Vec3::new(-3.0, -2.0, 2.0), ..Default::default() },
        Light { direction: Vec3::new(2.0, 1.0, -1.0), ..Default::default() },
    ];
    shader.set_lights(&lights);

    let mut camera = Camera::new(Vec3::new(0.0, 0.0, 1.0));
    renderer.set_rendering_mode(RenderingMode::TileBased);

    let mut display = Display::new(width, height);

    while display.is_open() {
        display.handle_input(&mut camera);

        shader.set_uniform("cameraPos", camera.position());
        shader.set_uniform("viewMatrix", camera.view_matrix());
        shader.set_uniform("projectionMatrix",
            camera.projection_matrix(60.0, width as f32 / height as f32, 0.1, 100.0));

        buffer.clear_draw_buffer(Color::BLACK);
        renderer.draw_model(&model, &shader, buffer.draw_buffer_mut());
        buffer.swap();

        display.update(buffer.display_buffer());
    }
}
```

Reference: `test/system_test/main.cpp` (lines 37-118).

---

## Phase 12: Integration & Verification

### Task 25: Integration Tests

**Files:**
- Create: `rust/simple_renderer/tests/integration_test.rs`

Tests:
1. Load teapot model → verify vertex/face count
2. Render one frame with PerTriangle → verify buffer has non-zero pixels
3. Render one frame with each mode → verify all produce output
4. Compare output consistency between modes (same scene should produce similar pixel counts)

### Task 26: Build & Run Verification

```bash
cd rust/
cargo build --release
cargo test
./target/release/system_test ../../obj
```

Verify:
- All tests pass
- Demo window opens and shows rendered teapot
- All 4 rendering modes work (modify code or add CLI arg to switch)
- Camera controls work (arrow keys)
- ESC/Q exits cleanly

### Task 27: Final Commit

```bash
git add rust/
git commit -m "feat: complete Rust rewrite of SimpleRenderer with all 4 rendering modes"
```

---

## Execution Notes

### Order of Implementation

The tasks are ordered to build dependencies bottom-up. Each phase depends on the previous:

1. **Phase 1** (Tasks 1-9): Foundation types — no dependencies
2. **Phase 2** (Task 10): Model loading — depends on Phase 1 types
3. **Phase 3** (Tasks 11-12): Buffer + Rasterizer — depends on Vertex, Fragment, Color
4. **Phase 4** (Tasks 13-14): Shader — depends on all Phase 1 types + Fragment
5. **Phase 5** (Task 15): Renderer trait — depends on Model, Shader
6. **Phases 6-9** (Tasks 16-19): Renderers — depend on everything above, can be done sequentially
7. **Phase 10** (Tasks 20-21): Facade — depends on all renderers
8. **Phase 11** (Tasks 22-24): Demo app — depends on facade
9. **Phase 12** (Tasks 25-27): Verification — depends on everything

### Parallelizable Work

Within each phase, some tasks can be done in parallel:
- Tasks 2-9 (core types) are mostly independent
- Tasks 16-19 (renderers) are independent once the trait is defined

### Key Algorithms to Port Carefully

These contain non-trivial math that must be ported exactly:
1. **Barycentric coordinates** (`rasterizer.cpp:84-109`) — cross product method
2. **Perspective correction** (`rasterizer.cpp:140-162`) — 1/w interpolation
3. **Phong shading** (`shader.cpp:289-365`) — ambient + diffuse + specular with LUT
4. **Specular LUT** (`shader.cpp:239-287`) — thread-safe cache with linear interpolation
5. **Backface culling** (`per_triangle_renderer.cpp:80-94`) — screen-space cross product sign
6. **Tile binning** (`tile_based_renderer.cpp`) — two-pass count+fill approach

### minifb Pixel Format

minifb expects pixels as `u32` in `0RGB` format (bits 23-16 = R, 15-8 = G, 7-0 = B, bits 31-24 ignored). The C++ SDL2 code uses `SDL_PIXELFORMAT_RGBA32` which is `RGBA` byte order. Ensure `Color::into::<u32>()` matches minifb's expected format. May need to swap from RGBA to 0RGB:

```rust
// For minifb: 0x00RRGGBB
fn to_minifb_pixel(c: Color) -> u32 {
    ((c.r() as u32) << 16) | ((c.g() as u32) << 8) | (c.b() as u32)
}
```

This is a critical detail — wrong pixel format will produce garbled colors.
