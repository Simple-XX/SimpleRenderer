[![codecov](https://codecov.io/gh/Simple-XX/SimpleRenderer/graph/badge.svg?token=J7NKK3SBNJ)](https://codecov.io/gh/Simple-XX/SimpleRenderer)
![workflow](https://github.com/Simple-XX/SimpleRenderer/actions/workflows/workflow.yml/badge.svg)
![commit-activity](https://img.shields.io/github/commit-activity/t/Simple-XX/SimpleRenderer)
![last-commit-main](https://img.shields.io/github/last-commit/Simple-XX/SimpleRenderer/main)
![MIT License](https://img.shields.io/github/license/mashape/apistatus.svg)
[![LICENSE](https://img.shields.io/badge/license-Anti%20996-blue.svg)](https://github.com/996icu/996.ICU/blob/master/LICENSE)
[![996.icu](https://img.shields.io/badge/link-996.icu-red.svg)](https://996.icu)

# SimpleRenderer

[![cn](https://img.shields.io/badge/language-cn-red.svg)](https://github.com/Simple-XX/SimpleRenderer/blob/main/README.md)
[![en](https://img.shields.io/badge/language-en-blue.svg)]((https://github.com/Simple-XX/SimpleRenderer/blob/main/README.en.md))


## 简介

SimpleRenderer 是一个专注于渲染管道细致控制的软件渲染器，包括顶点变换、着色和像素光栅化。由于不依赖 GPU，它非常适合教育用途、实时仿真以及不具备专用图形硬件的平台。

### 功能

- **可定制的着色器**：实现了顶点和片段着色器，以实现对渲染过程的高控制力。
- **跨平台兼容**：兼容 Linux 和 macOS，并为每个平台提供了特定的构建配置。
- **真实感的光照模型**：支持方向光、点光源和环境光，并包含 Phong 着色和其他着色模型。
- **广泛测试**：包括单元测试和系统测试，以确保稳定的性能和精确的渲染。

### 核心渲染管道

SimpleRenderer 的渲染管道包括多个阶段，如 `src/rasterizer.cpp` 和 `src/renderer.cpp` 中详细描述：

1. **顶点处理和变换**：
   - **顶点着色器**：每个顶点在顶点着色器中进行变换，从模型空间投影到屏幕空间。
   - **变换矩阵**：
     - **模型矩阵**：应用旋转、平移和缩放等对象变换。
     - **视图矩阵**：将世界坐标转换为相机坐标。
     - **投影矩阵**：应用透视投影以模拟深度和视角。
     - 这些矩阵的组合形成了**模型视图投影（MVP）矩阵**，这是将 3D 坐标转换为 2D 屏幕空间的关键。

2. **图元组装和裁剪**：
   - **三角形组装**：顶点被组装成三角形，这是 SimpleRenderer 中的基本绘图图元。
   - **视锥体裁剪**：裁剪在相机视锥体外的三角形，以优化渲染。

3. **光栅化和片段处理**：
   - **重心插值**：光栅化使用重心坐标在三角形表面上插值像素值，以准确计算颜色、纹理和深度。
   - **深度缓冲（Z 缓冲）**：管理深度信息，以正确渲染重叠对象，确保只有离相机最近的表面可见。
   - **片段着色器**：基于光照和表面属性计算像素颜色，应用如 Phong 着色的着色模型实现真实感光照。

4. **光照和着色模型**：
   - **Phong 光照模型**：通过基于表面法线和光方向计算环境、漫反射和镜面反射分量，模拟真实的光照。
   - **多光源支持**：支持点光源、方向光和环境光，每种光源为场景增加了深度和真实感。
   - **着色器定制**：可以定制着色器，以创建各种视觉效果，包括真实的材质和纹理。

5. **优化技术**：
   - **背面剔除**：剔除相机不可见的面，减少片段处理数量。
   - **屏幕空间优化**：减少对屏幕外对象的不必要计算。

### 构建和运行说明

#### 先决条件

在您的系统上安装以下依赖项：

```bash
sudo apt install doxygen graphviz clang-format clang-tidy cppcheck lcov gcc g++ libsdl2-dev libsdl2-ttf-dev libomp-dev libspdlog-dev cmake libassimp-dev
```

在 macOS 上，使用 [Homebrew](https://brew.sh/) 安装依赖项：

```bash
brew install doxygen graphviz clang-format clang-tidy cppcheck lcov gcc sdl2 sdl2_ttf libomp spdlog cmake assimp
```

#### 构建项目

##### 1. 默认构建 (Linux/Windows)

1. **克隆仓库**：

   ```bash
   git clone https://github.com/Simple-XX/SimpleRenderer.git
   cd SimpleRenderer
   ```

2. **使用 CMake 预设配置和构建**：

   ```bash
   cmake --preset=build
   cmake --build build --target all
   ```

3. **运行系统测试**：

   ```bash
   ./build/bin/system_test ../obj
   ```

##### 2. macOS 构建

使用 macOS 特定的预设以包含 RPATH 配置：

1. **使用 macOS 预设配置和构建**：

   ```bash
   cmake --preset=build-macos
   cmake --build build --target all
   ```

2. **运行系统测试**：

   ```bash
   ./build/bin/system_test ../obj
   ```

### 示例用法和控件

1. **运行渲染器**：
   
   ```bash
   ./build/bin/system_test ../obj
   ```

2. **控件**：
   - **方向键**：移动相机。
   - **TAB**：切换不同模型。
   - **数字键**：切换渲染模式（例如，线框、填充三角形）。

### 代码结构

- **src/rasterizer.cpp**：实现了核心光栅化技术，包括顶点和片段着色、三角形组装和深度缓冲。
- **src/renderer.cpp**：管理渲染管道，包括相机设置、模型加载、光照和着色器管理。
- **src/include/**：定义渲染器中使用的核心类和结构的头文件。
- **test/system_test/**：展示使用基础模型的系统测试。
- **test/unit_test/**：各组件的单元测试。

### 生成文档

生成并查看文档以获取详细的代码信息：

```bash
cmake --build build --target doc
xdg-open doc/html/index.html
```

### 贡献

我们欢迎您的贡献！请随时提交问题或拉取请求，以帮助改进 SimpleRenderer。

### 许可证

该项目基于 MIT 许可证发布。详情请参阅 `LICENSE` 文件。
