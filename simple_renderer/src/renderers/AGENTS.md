# RENDERERS — Rendering Strategy Implementations

4 strategies implementing `Renderer` trait, sharing the same vertex/fragment shader pipeline but differing in triangle scheduling, rasterization, and shading order.

## STRUCTURE

```
renderers/
├── mod.rs                  # Renderer trait definition (Send bound)
├── base.rs                 # Shared: perspective_division, viewport_transform
├── tile_common.rs          # Shared tile types: TileGridContext, TileTriangleRef, vertex_transform_soa, bin_triangles
├── per_triangle.rs         # Forward: chunk-parallel over faces, Rasterizer-based
├── tile_based.rs           # Tile-based forward: SoA + edge functions + optional Early-Z
├── deferred.rs             # Deferred: collect all fragments → shade depth winners only
└── tile_based_deferred.rs  # TBDR: tile binning + 2-pass (Z-prepass → shade winners)
```

## RENDERER COMPARISON

| Renderer | Rasterization | Shading | Parallelism | Key Tradeoff |
|----------|--------------|---------|-------------|--------------|
| `PerTriangle` | `Rasterizer` (barycentric) | Immediate per-fragment | `par_chunks` over faces | Simple but shades occluded pixels |
| `TileBased` | Edge functions (inline) | Immediate per-pixel | `par_iter` over tiles | Cache-friendly, optional Early-Z reduces overdraw |
| `Deferred` | `Rasterizer` (barycentric) | Deferred (winners only) | `par_chunks` + pixel-parallel merge | Zero wasted shading, higher memory |
| `TileBasedDeferred` | Edge functions (inline) | 2-pass deferred | `par_iter` over tiles | Best of both: tiling + zero overdraw |

## WHERE TO LOOK

| Task | File | Notes |
|------|------|-------|
| Add new renderer | New file + `mod.rs` + `renderer.rs::create_renderer` | Implement `Renderer` trait |
| Modify shared vertex pipeline | `base.rs` | `perspective_division`, `viewport_transform` |
| Change tile binning/grid | `tile_common.rs` | `TileGridContext`, `bin_triangles`, `vertex_transform_soa` |
| Adjust tile size / lane width | `tile_common.rs` | `DEFAULT_TILE_SIZE=64`, `K_LANE=8` |
| Fix rasterization in tile renderers | `tile_based.rs` or `tile_based_deferred.rs` | Inline edge-function rasterization (not `Rasterizer`) |
| Fix rasterization in non-tile renderers | `per_triangle.rs` / `deferred.rs` | Uses `Rasterizer` from `rasterizer.rs` |

## CONVENTIONS

- **Renderer trait**: `fn render(&self, model, shader, buffer, w, h) -> bool` — `&self` because renderers use local buffers for thread safety
- **Two rasterization paths**: `Rasterizer` struct (barycentric, used by PerTriangle/Deferred) vs inline edge functions (used by tile renderers)
- **SoA layout**: Tile renderers convert vertices to `VertexSoA` for cache-friendly access during tile rasterization
- **Edge function lanes**: `K_LANE=8` pixels processed per step in tile renderers — SIMD-style but scalar
- **Per-thread local buffers**: All renderers allocate thread-local depth/color buffers, then merge — avoids synchronization
- **Backface culling**: PerTriangle/TileBased do screen-space culling; Deferred/TBDR skip it (collect all, resolve at depth test)
- **Material thread safety**: `Arc<Material>` on faces, material passed separately to `fragment_shader` (not stored in Fragment)

## ANTI-PATTERNS

- Do NOT call `Rasterizer` from tile-based renderers — they use inline edge functions for cache locality
- Do NOT store mutable state in `Renderer` struct — use thread-local buffers instead
- Do NOT skip `shader.prepare_caches()` before rendering — caches must be valid for vertex/fragment shaders
