# PROJECT KNOWLEDGE BASE

**Generated:** 2026-03-10
**Commit:** 9718b21
**Branch:** main

## OVERVIEW

Educational C++ software renderer (C++23) mimicking OpenGL's GPU pipeline. Builds as a static library (`SimpleRenderer`) with SDL2 display, GLM math, Assimp model loading, spdlog logging, and OpenMP parallelism.

## STRUCTURE

```
SimpleRenderer/
├── src/                    # Core static library (see src/AGENTS.md)
│   ├── include/            # All public headers
│   │   └── renderers/      # Renderer strategy headers
│   └── renderers/          # Renderer strategy implementations
├── test/
│   ├── system_test/        # Visual demo app (ONLY main() lives here)
│   └── unit_test/          # GoogleTest unit tests (*_test.cpp)
├── cmake/                  # Build helpers (deps, compile opts, functions)
├── obj/                    # Bundled 3D models (.obj/.mtl)
├── doc/                    # Doxygen (generated, not committed)
└── tools/                  # cppcheck suppression config
```

## WHERE TO LOOK

| Task | Location | Notes |
|------|----------|-------|
| Add/modify rendering algorithm | `src/renderers/` + `src/include/renderers/` | Subclass `RendererBase`, register in `renderer.cpp` |
| Change shader behavior | `src/shader.cpp` + `src/include/shader.hpp` | Vertex/Fragment shaders, uniform caching, specular LUT |
| Modify rasterization | `src/rasterizer.cpp` + `src/include/rasterizer.hpp` | Barycentric interpolation, perspective correction |
| Add new model format | `src/model.cpp` + `src/include/model.hpp` | Uses Assimp; texture cache in `Model` |
| Change display/input | `test/system_test/display.cpp` + `camera.h` | SDL2 window, keyboard handling |
| Run the app | `test/system_test/main.cpp` | `./build/bin/system_test ./obj` |
| Add unit tests | `test/unit_test/` | Use `*_test.cpp` naming, GoogleTest |
| Modify build deps | `cmake/3rd.cmake` | CPM.cmake package manager |
| Change compile flags | `cmake/compile_config.cmake` | `-Wall -Wextra`, `-Werror` in Release |
| Generated config values | `cmake/config.h.in` → `src/include/config.h` | Thread count, log paths, OBJ path |

## CODE MAP

| Symbol | Type | Location | Role |
|--------|------|----------|------|
| `SimpleRenderer` | Facade class | `renderer.h` | Mode selection + unified `DrawModel` entry point |
| `RendererBase` | Abstract base | `renderers/renderer_base.hpp` | Defines `Render()` interface, shared perspective/viewport transforms |
| `PerTriangleRenderer` | Strategy | `renderers/per_triangle_renderer.hpp` | AoS forward rendering (traditional) |
| `TileBasedRenderer` | Strategy | `renderers/tile_based_renderer.hpp` | SoA + tile binning + optional Early-Z |
| `DeferredRenderer` | Strategy | `renderers/deferred_renderer.hpp` | AoS deferred (collect fragments → shade winners) |
| `TileBasedDeferredRenderer` | Strategy | `renderers/tile_based_deferred_renderer.hpp` | TBDR: tile binning + 2-pass (Z-prepass + deferred shade) |
| `Shader` | Class | `shader.hpp` | Vertex/Fragment shaders, uniform buffer, specular LUT cache |
| `Rasterizer` | Class | `rasterizer.hpp` | Triangle rasterization, barycentric coords, perspective correction |
| `Model` | Class | `model.hpp` | Assimp-based OBJ loader, vertex/face/material storage |
| `Vertex` | Data class | `vertex.hpp` | Position, normal, UV, color, optional clip-space coord |
| `VertexSoA` | Struct | `vertex.hpp` | Structure-of-Arrays layout for tile-based renderers |
| `Color` | Class | `color.h` | 32-bit RGBA, operator overloads for arithmetic |
| `Buffer` | Class | `buffer.hpp` | Double-buffered framebuffer (draw + display swap) |
| `Fragment` | Struct | `shader.hpp` | Fragment shader input (screen coord, normal, UV, depth, material) |

## CONVENTIONS

- **Namespace**: All code in `simple_renderer`
- **Style**: Google C++ style (`.clang-format`), enforced via `clang-format` target
- **Headers**: `.h` for C-compatible / `.hpp` for C++-only (templates, classes with methods)
- **Comments**: Bilingual (Chinese + English) — Chinese for design intent, English for API docs
- **Naming**: `PascalCase` classes/methods, `snake_case_` private members with trailing underscore, `kPascalCase` constants
- **Return style**: Trailing return `auto Foo() -> ReturnType` used in some classes (Color, Rasterizer)
- **Standard**: C++23 (`CMAKE_CXX_STANDARD 23`), no extensions
- **In-source builds**: Prohibited (CMake fatal error)
- **Static analysis**: clang-tidy (see `.clang-tidy` for enabled checks), cppcheck with `tools/cppcheck-suppressions.xml`

## ANTI-PATTERNS (THIS PROJECT)

- **Do NOT** modify `Color` memory layout (marked `@note` in `color.h` — 4-byte RGBA assumed by `uint32_t` cast)
- **Do NOT** build in-source (`mkdir build` required)
- **Do NOT** edit `src/include/config.h` — generated from `cmake/config.h.in` by CMake
- Unit tests `todo1` and `todo2` are placeholder stubs — do not treat as passing tests

## COMMANDS

```bash
# Configure (Linux)
cmake --preset=build

# Configure (macOS)
cmake --preset=build-macos

# Build all
cmake --build build --target all

# Run demo
./build/bin/system_test ./obj

# Unit tests + coverage
cmake --build build --target coverage

# Static analysis
cmake --build build --target clang-tidy
cmake --build build --target cppcheck

# Format code
cmake --build build --target clang-format

# Generate docs
cmake --build build --target doc
```

## DEPENDENCIES (via CPM.cmake)

| Library | Purpose |
|---------|---------|
| SDL2 (2.30.6) | Window/display, keyboard input |
| GLM (1.0.1) | Vector/matrix math (`Vector3f = glm::vec3`, `Matrix4f = glm::mat4`) |
| stb | Image loading (textures) |
| spdlog | Logging (system-installed, not CPM) |
| Assimp | 3D model loading (.obj/.mtl) |
| OpenMP | Parallel rasterization |
| GoogleTest (1.15.2) | Unit testing |
| wqy-zenhei font | CJK font for text rendering |

## NOTES

- **Entry point is in test/**: No `main()` in `src/`. The runnable demo lives at `test/system_test/main.cpp`. The library itself is headless.
- **obj/ is committed**: 3D model assets are tracked in git (teapot, cube, cornell box, etc.)
- **Four rendering modes**: Switch at runtime via `SetRenderingMode()` — PER_TRIANGLE, TILE_BASED, DEFERRED, TILE_BASED_DEFERRED
- **System test hotkeys**: `1` = toggle wireframe, `2` = toggle triangles, `TAB` = switch model
- **CI**: GitHub Actions runs build + coverage on push/PR, deploys Doxygen to gh-pages on release
