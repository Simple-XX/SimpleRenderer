[![codecov](https://codecov.io/gh/Simple-XX/SimpleRenderer/graph/badge.svg?token=J7NKK3SBNJ)](https://codecov.io/gh/Simple-XX/SimpleRenderer)
![workflow](https://github.com/Simple-XX/SimpleRenderer/actions/workflows/workflow.yml/badge.svg)
![commit-activity](https://img.shields.io/github/commit-activity/t/Simple-XX/SimpleRenderer)
![last-commit-main](https://img.shields.io/github/last-commit/Simple-XX/SimpleRenderer/main)
![MIT License](https://img.shields.io/github/license/mashape/apistatus.svg)
[![LICENSE](https://img.shields.io/badge/license-Anti%20996-blue.svg)](https://github.com/996icu/996.ICU/blob/master/LICENSE)
[![996.icu](https://img.shields.io/badge/link-996.icu-red.svg)](https://996.icu)

# SimpleRenderer

[![cn](https://img.shields.io/badge/language-cn-red.svg)](https://github.com/Simple-XX/SimpleRenderer/blob/main/README-cn.md)
[![en](https://img.shields.io/badge/language-en-blue.svg)]((https://github.com/Simple-XX/SimpleRenderer/blob/main/README.md))

A C++ software renderer designed for rendering 3D models with soft rasterization techniques. 
    
## Overview

SimpleRenderer is a software renderer built with a focus on fine control over the rendering pipeline, including vertex transformations, shading, and pixel rasterization. Its GPU-independent nature makes it suitable for educational purposes, real-time simulation, and platforms where dedicated graphics hardware is unavailable.

### Features

- **Customizable Shaders**: Vertex and fragment shaders are implemented to achieve high control over the rendering process.
- **Cross-platform Compatibility**: Compatible with Linux and macOS, with specific build configurations for each.
- **Realistic Lighting Models**: Supports directional, point, and ambient lighting with Phong shading and other shading models.
- **Extensive Testing**: Includes unit and system tests to ensure robust performance and accurate rendering.

### Core Rendering Pipeline

SimpleRenderer’s rendering pipeline comprises multiple stages, as detailed in `src/rasterizer.cpp` and `src/renderer.cpp`:

1. **Vertex Processing and Transformations**:
   - **Vertex Shader**: Each vertex undergoes transformations in the vertex shader, which projects it from model space to screen space.
   - **Transformation Matrices**:
     - **Model Matrix**: Applies object transformations such as rotation, translation, and scaling.
     - **View Matrix**: Transforms world coordinates into camera coordinates.
     - **Projection Matrix**: Applies perspective projection to simulate depth and perspective.
     - The combination of these matrices forms the **Model-View-Projection (MVP) Matrix**, which is essential for converting 3D coordinates into 2D screen space.

2. **Primitive Assembly and Clipping**:
   - **Triangle Assembly**: Vertices are assembled into triangles, the fundamental drawing primitive in SimpleRenderer.
   - **Frustum Clipping**: Triangles outside the camera's view frustum are clipped to optimize rendering.

3. **Rasterization and Fragment Processing**:
   - **Barycentric Interpolation**: Rasterization interpolates pixel values across the triangle surface using barycentric coordinates to accurately compute colors, textures, and depth.
   - **Depth Buffering (Z-Buffering)**: Manages depth information to correctly render overlapping objects, ensuring only the closest surfaces are visible.
   - **Fragment Shader**: Computes pixel colors based on lighting and surface properties, applying shading models such as Phong shading for realistic lighting.

4. **Lighting and Shading Models**:
   - **Phong Lighting Model**: Simulates realistic lighting by calculating ambient, diffuse, and specular components based on surface normals and light direction.
   - **Multiple Light Sources**: Supports point lights, directional lights, and ambient light, each adding depth and realism to the scene.
   - **Shader Customization**: Shaders can be customized to create various visual effects, including realistic materials and textures.

5. **Optimization Techniques**:
   - **Backface Culling**: Eliminates faces that are not visible to the camera, reducing the number of fragments processed.
   - **Screen-Space Optimization**: Reduces unnecessary computations for off-screen objects.

### Build and Run Instructions

#### Prerequisites

Install the following dependencies on your system:

```bash
sudo apt install doxygen graphviz clang-format clang-tidy cppcheck lcov gcc g++ libsdl2-dev libsdl2-ttf-dev libomp-dev libspdlog-dev cmake libassimp-dev
```

On macOS, use [Homebrew](https://brew.sh/) for dependencies:

```bash
brew install doxygen graphviz clang-format clang-tidy cppcheck lcov gcc sdl2 sdl2_ttf libomp spdlog cmake assimp
```

#### Building the Project

##### 1. Default Build (Linux/Windows)

1. **Clone the Repository:**

   ```bash
   git clone https://github.com/Simple-XX/SimpleRenderer.git
   cd SimpleRenderer
   ```

2. **Configure and Build Using CMake Presets:**

   ```bash
   cmake --preset=build
   cmake --build build --target all
   ```

3. **Run System Tests**:

   ```bash
   ./build/bin/system_test ../obj
   ```

##### 2. macOS Build

Use the macOS-specific preset to include RPATH configurations:

1. **Configure and Build Using macOS Preset**:

   ```bash
   cmake --preset=build-macos
   cmake --build build --target all
   ```

2. **Run System Tests**:

   ```bash
   ./build/bin/system_test ../obj
   ```

### Example Usage and Controls

1. **Run the Renderer**:
   
   ```bash
   ./build/bin/system_test ../obj
   ```

2. **Controls**:
   - **Arrow Keys**: Move the camera.
   - **TAB**: Switch between different models.
   - **Number Keys**: Switch rendering modes (e.g., wireframe, filled triangles).

### Code Structure

- **src/rasterizer.cpp**: Implements core rasterization techniques, including vertex and fragment shading, triangle assembly, and depth buffering.
- **src/renderer.cpp**: Manages the rendering pipeline, including camera setup, model loading, lighting, and shader management.
- **src/include/**: Header files defining the core classes and structures used across the renderer.
- **test/system_test/**: System tests demonstrating rendering with basic models.
- **test/unit_test/**: Unit tests for individual components.

### Generating Documentation

Generate and view the documentation for detailed code insights:

```bash
cmake --build build --target doc
xdg-open doc/html/index.html
```

### Contributions

We welcome contributions! Please feel free to submit issues or pull requests to help improve SimpleRenderer.

### License

This project is licensed under the MIT License. See the `LICENSE` file for details.

