# src/ — Core Renderer Library

Static library (`SimpleRenderer`) built from all `.cpp` files here + `renderers/`. Headers live in `include/`.

## ARCHITECTURE

```
SimpleRenderer (Facade)          renderer.h / renderer.cpp
  └─▶ RendererBase (abstract)    renderers/renderer_base.hpp
        ├── PerTriangleRenderer   AoS, per-thread local framebuffer merge
        ├── TileBasedRenderer     SoA, tile binning, optional Early-Z
        ├── DeferredRenderer      AoS, collect-then-shade (GPU pipeline mimic)
        └── TileBasedDeferredRenderer  SoA, 2-pass: Z-prepass + deferred shade
```

`SimpleRenderer::DrawModel()` → delegates to active `RendererBase::Render()`.

Mode switch: `SetRenderingMode()` → reconstructs `renderer_` via `EnsureRenderer()`.

## RENDERING PIPELINE (per Render() call)

1. **Vertex Shader** (`Shader::VertexShader`) — model→clip space via cached MVP
2. **Perspective Division** (`RendererBase::PerspectiveDivision`) — clip→NDC
3. **Viewport Transform** (`RendererBase::ViewportTransformation`) — NDC→screen
4. **Rasterization** (`Rasterizer::Rasterize`) — barycentric interpolation, perspective correction
5. **Fragment Shader** (`Shader::FragmentShader`) — Phong lighting, texture sampling, specular LUT

## WHERE TO LOOK

| Task | Files | Notes |
|------|-------|-------|
| Add new renderer | `renderers/` + `include/renderers/` + `renderer.cpp` | Subclass `RendererBase`, add enum value to `RenderingMode`, register in `EnsureRenderer()` |
| Modify vertex transform | `shader.cpp` (VertexShader) | Uses `VertexUniformCache` for matrix caching |
| Modify fragment shading | `shader.cpp` (FragmentShader) | Phong model, multi-light, specular LUT with `shared_mutex` |
| Change rasterization | `rasterizer.cpp` | `Rasterize()` returns `vector<Fragment>` |
| Modify model loading | `model.cpp` | Assimp-based; `ProcessNode`→`ProcessMesh`→`ProcessMaterial` |
| Add new data type | `include/` | Follow `.h`/`.hpp` convention |

## KEY PATTERNS

- **AoS vs SoA**: `PerTriangleRenderer`/`DeferredRenderer` use `Vertex` (AoS). `TileBasedRenderer`/`TileBasedDeferredRenderer` convert to `VertexSoA` for cache-friendly tile processing.
- **Uniform caching**: `Shader` caches derived matrices (`mvp`, `normal`) in `VertexUniformCache` and light dirs in `FragmentUniformCache`. Call `PrepareUniformCaches()` before render loop or rely on auto-invalidation via `SetUniform()`.
- **Specular LUT**: `Shader` maintains a thread-safe `unordered_map<uint32_t, SpecularLUT>` keyed by `bit_cast<uint32_t>(shininess)`. Guarded by `shared_mutex` (read-shared, write-exclusive).
- **Tile binning**: Two-pass approach — count pass (size reservation) then fill pass. `TileTriangleRef` stores SoA indices + material pointer.
- **OpenMP parallelism**: Renderers parallelize over triangles (PerTriangle) or tiles (TileBased). Thread-local buffers merged post-loop.

## CONVENTIONS (src-specific)

- **Header/source pairing**: `include/foo.hpp` ↔ `foo.cpp`. Renderers: `include/renderers/foo.hpp` ↔ `renderers/foo.cpp`
- **Math aliases**: `Vector3f = glm::vec3`, `Matrix4f = glm::mat4` (defined in `math.hpp`)
- **GLM experimental**: `GLM_ENABLE_EXPERIMENTAL` is `#define`d in `math.hpp` for `glm::to_string()`
- **spdlog formatters**: Custom `fmt::formatter` specializations for `Vector3f`, `Vector4f`, `Matrix4f`, `Color` in `math.hpp`

## ANTI-PATTERNS

- **Do NOT** add `main()` here — library is headless; entry point lives in `test/system_test/`
- **Do NOT** break header convention: `.h` = C-compatible, `.hpp` = C++-only
