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

An educational C++ software renderer designed to help developers understand the inner workings of rendering pipelines and how OpenGL operates behind the scenes.

## Overview

SimpleRenderer is a software renderer built with the primary goal of educating developers about the fundamentals of 3D rendering and graphics pipelines. By providing a simplified yet functional rendering framework, it demystifies the complex processes involved in rendering graphics, mirroring how OpenGL and other graphics APIs work under the hood.

### Purpose

- **Educational Focus**: Designed to teach the core concepts of rendering, including vertex processing, rasterization, shading, and transformations.
- **Demystifying OpenGL**: Offers insights into how OpenGL and GPU-based rendering pipelines function internally.
- **Hands-On Learning**: Allows developers to experiment with rendering algorithms and observe the effects of various techniques in real-time.

### Key Features

- **Customizable Shaders**: Implemented vertex and fragment shaders to demonstrate how shading works at a fundamental level.
- **Simplified Rendering Pipeline**: Breaks down the rendering process into understandable stages, mirroring the OpenGL pipeline.
- **Cross-Platform Compatibility**: Compatible with Linux and macOS, facilitating learning across different environments.
- **Extensive Documentation**: Provides detailed explanations of each component to aid learning and comprehension.

### Learning Objectives

By exploring SimpleRenderer, you will learn:

- How vertices are transformed from 3D space to 2D screen coordinates.
- The process of assembling primitives (triangles) and performing clipping.
- How rasterization converts vector information into pixels.
- The fundamentals of shading models, including lighting calculations.
- How depth buffering and backface culling optimize rendering.

---

## Getting Started

### Prerequisites

Ensure you have the following dependencies installed:

```bash
sudo apt install doxygen graphviz clang-format clang-tidy cppcheck lcov gcc g++ libsdl2-dev libsdl2-ttf-dev libomp-dev libspdlog-dev cmake libassimp-dev
```

For macOS users, install dependencies using Homebrew:

```bash
brew install doxygen graphviz clang-format clang-tidy cppcheck lcov gcc sdl2 sdl2_ttf libomp spdlog cmake assimp
```

### Building the Project

#### 1. Clone the Repository

```bash
git clone https://github.com/Simple-XX/SimpleRenderer.git
cd SimpleRenderer
```

#### 2. Configure and Build Using CMake Presets

For a standard build:

```bash
cmake --preset=build
cmake --build build --target all
```

For macOS:

```bash
cmake --preset=build-macos
cmake --build build-macos --target all
```

#### 3. Run the Example Application

```bash
./build/bin/system_test ../obj
```

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
     - **Phong Shading Model**: Simulates realistic lighting with ambient, diffuse, and specular components.
     - **Surface Normals**: Determine how light interacts with surfaces.
     - **Light Sources**: Understand different types of lights (directional, point, ambient).

5. **Optimization Techniques**

   - **Objective**: Learn methods to improve rendering efficiency.
   - **Key Concepts**:
     - **Backface Culling**: Eliminates faces not visible to the camera.
     - **Spatial Partitioning**: Organizes objects to reduce rendering workload.

### Code Structure

- **src/rasterizer.cpp**

  Focuses on the rasterization process, converting vector data into raster images. Key learning points include:

  - Implementing barycentric interpolation.
  - Managing depth buffering.
  - Handling edge cases in rasterization.

- **src/renderer.cpp**

  Orchestrates the rendering process. Highlights include:

  - Setting up transformation matrices.
  - Managing the rendering loop.
  - Integrating shaders and handling user input.

- **src/include/**

  Contains header files with detailed comments explaining the purpose and functionality of classes and methods.

---

## Experimentation and Learning

To maximize learning, consider the following steps:

- **Modify Shaders**

  Experiment with the shader code to see how changes affect rendering.

- **Adjust Transformations**

  Play with the model, view, and projection matrices to understand their impact on the scene.

- **Implement New Features**

  Try adding new lighting models, textures, or shading techniques.

---

## Documentation

Generate the documentation to delve deeper into the codebase:

````bash
cmake --build build --target doc
xdg-open doc/html/index.html
````

The documentation provides detailed explanations and diagrams to enhance understanding.

---

## Contributions

Your contributions can help others learn. Feel free to:

- Submit pull requests with improvements or new educational features.
- Report issues or suggest enhancements.
- Share your learning experiences.

---

## License

This project is licensed under the MIT License. See the `LICENSE` file for more information.
