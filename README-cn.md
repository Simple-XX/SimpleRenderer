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


## 概述

SimpleRenderer 是一个软件渲染器，主要目标是教育开发者了解 3D 渲染和图形管道的基本原理。通过提供一个简化但功能完善的渲染框架，它揭示了渲染图形过程中涉及的复杂过程，模拟了 OpenGL 和其他图形 API 的内部工作方式。

### 目的

- **教育重点**：旨在教授渲染的核心概念，包括顶点处理、光栅化、着色和变换。
- **解密 OpenGL**：提供对 OpenGL 和基于 GPU 的渲染管道内部工作方式的深入了解。
- **动手学习**：允许开发者实验渲染算法并实时观察各种技术的效果。

### 主要功能

- **可定制的着色器**：实现了顶点和片段着色器，展示着色的基本工作原理。
- **简化的渲染管道**：将渲染过程分解为易于理解的阶段，模拟 OpenGL 管道。
- **跨平台兼容性**：兼容 Linux 和 macOS，方便在不同环境中学习。
- **详尽的文档**：提供每个组件的详细解释，辅助学习和理解。

### 学习目标

通过探索 SimpleRenderer，您将学习：

- 如何将顶点从 3D 空间转换为 2D 屏幕坐标。
- 组装图元（三角形）和执行裁剪的过程。
- 光栅化如何将矢量信息转换为像素。
- 着色模型的基础知识，包括光照计算。
- 深度缓冲和背面剔除如何优化渲染。

---

## 快速开始

### 前置条件

确保您已安装以下依赖项：

```bash
sudo apt install doxygen graphviz clang-format clang-tidy cppcheck lcov gcc g++ libsdl2-dev libsdl2-ttf-dev libomp-dev libspdlog-dev cmake libassimp-dev
```

对于 macOS 用户，使用 Homebrew 安装依赖项：

```bash
brew install doxygen graphviz clang-format clang-tidy cppcheck lcov gcc sdl2 sdl2_ttf libomp spdlog cmake assimp
```

### 构建项目

#### 1. 克隆仓库

```bash
git clone https://github.com/Simple-XX/SimpleRenderer.git
cd SimpleRenderer
```

#### 2. 使用 CMake 预设配置和构建

标准构建：

```bash
cmake --preset=build
cmake --build build --target all
```

对于 macOS：

```bash
cmake --preset=build-macos
cmake --build build-macos --target all
```

#### 3. 运行示例应用程序

```bash
./build/bin/system_test ../obj
```

---

## 理解渲染器

### 核心渲染管道

SimpleRenderer 的渲染管道旨在模拟典型的基于 GPU 的管道阶段，提供每个组件如何贡献于最终渲染图像的清晰视图。

1. **顶点处理和变换**

   - **目标**：理解 3D 模型如何投影到 2D 屏幕。
   - **关键概念**：
     - **模型矩阵**：定位和定向世界中的模型。
     - **视图矩阵**：表示相机的位置和方向。
     - **投影矩阵**：定义相机的镜头（视野、纵横比）。

2. **图元组装和裁剪**

   - **目标**：了解单个顶点如何形成三角形以及如何处理屏幕外的部分。
   - **关键概念**：
     - **三角形组装**：将顶点分组为可绘制的图元。
     - **裁剪**：丢弃或调整视锥体外的图元。

3. **光栅化和片段处理**

   - **目标**：发现三角形如何转换为像素数据。
   - **关键概念**：
     - **重心坐标**：用于在三角形上插值顶点属性。
     - **深度缓冲**：确保正确渲染重叠对象。
     - **片段着色器**：计算每个像素的颜色和其他属性。

4. **着色和光照模型**

   - **目标**：探索光照如何影响表面的外观。
   - **关键概念**：
     - **Phong 着色模型**：通过环境光、漫反射和镜面反射分量模拟真实光照。
     - **表面法线**：确定光线如何与表面交互。
     - **光源**：了解不同类型的光（方向光、点光源、环境光）。

5. **优化技术**

   - **目标**：学习提高渲染效率的方法。
   - **关键概念**：
     - **背面剔除**：消除相机不可见的面。
     - **空间划分**：组织对象以减少渲染工作负载。

### 代码结构

- **src/rasterizer.cpp**

  关注光栅化过程，将矢量数据转换为光栅图像。主要学习点包括：

  - 实现重心插值。
  - 管理深度缓冲。
  - 处理光栅化中的边缘情况。

- **src/renderer.cpp**

  协调渲染过程。亮点包括：

  - 设置变换矩阵。
  - 管理渲染循环。
  - 集成着色器和处理用户输入。

- **src/include/**

  包含头文件，详细注释解释类和方法的目的和功能。

---

## 实验和学习

为最大化学习效果，考虑以下步骤：

- **修改着色器**

  实验着色器代码，观察更改如何影响渲染。

- **调整变换**

  修改模型、视图和投影矩阵，理解它们对场景的影响。

- **实现新功能**

  尝试添加新的光照模型、纹理或着色技术。

---

## 文档

生成文档以深入了解代码库：

```bash
cmake --build build --target doc
xdg-open doc/html/index.html
```

文档提供详细的解释和图表，增强理解。

---

## 贡献

您的贡献可以帮助他人学习。欢迎您：

- 提交改进或新的教育功能的拉取请求。
- 报告问题或提出建议。
- 分享您的学习体验。

---

## 许可证

该项目基于 MIT 许可证。有关详细信息，请参阅 `LICENSE` 文件。