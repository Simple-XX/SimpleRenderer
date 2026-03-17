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

一个教育性的 Rust 软件渲染器，帮助开发者理解渲染管线的内部工作原理以及 OpenGL 的底层机制。

## 概述

SimpleRenderer 是一个以教育为核心目标的软件渲染器，旨在帮助开发者掌握 3D 渲染和图形管线的基本原理。通过提供一个简化但功能完备的渲染框架，它揭示了渲染图形过程中的复杂机制，模拟了 OpenGL 等图形 API 的内部工作方式。

本项目使用最小化 `unsafe` Rust 实现，采用 Cargo workspace 结构，包含两个 crate：`simple_renderer`（核心库）和 `system_test`（交互式演示程序）。最低支持 Rust 1.73+。

### 目的

- **教育重点**：教授渲染的核心概念，包括顶点处理、光栅化、着色和变换。
- **解密 OpenGL**：深入了解 OpenGL 和基于 GPU 的渲染管线内部工作方式。
- **动手学习**：实验渲染算法并实时观察各种技术的效果。

### 主要功能

- **可定制的着色器**：实现了顶点和片段着色器（`shader/` 模块），展示着色的基本工作原理。
- **简化的渲染管线**：将渲染过程分解为易于理解的阶段，模拟 OpenGL 管线。
- **四种渲染策略**：运行时可在 `PerTriangle`、`TileBased`、`Deferred` 和 `TileBasedDeferred` 四种渲染模式之间切换。
- **Blinn-Phong 着色**：真实光照效果，包含环境光、漫反射和镜面反射分量，以及高光 LUT 缓存。
- **多缓冲帧缓冲**：基于无锁三缓冲的独立渲染线程。支持运行时在双缓冲（GPU 风格 VSync 阻塞）和三缓冲（非阻塞）模式之间切换。
- **并行渲染**：使用 [rayon](https://github.com/rayon-rs/rayon) 实现扫描线级并行光栅化和分块/分片并行渲染策略。
- **缓冲区复用**：渲染器结构体持有深度/颜色缓冲区，每帧通过 `fill()` 重置，避免重复分配。
- **多线程架构**：独立渲染线程通过无锁三缓冲与输入/显示解耦，模拟真实 GPU 双缓冲/三缓冲行为。
- **最小化 Unsafe**：仅无锁三缓冲使用 `unsafe` 实现 `Send`/`Sync`，附完整安全不变量文档。所有渲染逻辑均为 100% 安全 Rust。
- **跨平台兼容**：兼容 Linux 和 macOS。仅支持小端字节序（编译期校验）。

### 学习目标

通过探索 SimpleRenderer，您将学习：

- 如何将顶点从 3D 空间变换为 2D 屏幕坐标。
- 图元组装（三角形）和裁剪的过程。
- 光栅化如何将矢量信息转换为像素。
- 着色模型的基础知识，包括光照计算。
- 深度缓冲和背面剔除如何优化渲染。
- 双缓冲和三缓冲如何将渲染与显示输出解耦。

---

## 快速开始

### 前置条件

确保已安装 Rust 1.73+ 和 Cargo。推荐通过 [rustup](https://rustup.rs/) 安装：

```bash
curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sh
```

无需其他系统依赖 — 所有库依赖均由 Cargo 管理。

### 构建项目

#### 1. 克隆仓库

```bash
git clone https://github.com/Simple-XX/SimpleRenderer.git
cd SimpleRenderer
```

#### 2. 构建

```bash
cargo build                              # Debug 构建
cargo build --release                    # Release 构建
```

#### 3. 运行示例程序

```bash
cargo run -p system_test -- ./obj        # 运行演示（犹他茶壶）
```

### 操作说明

| 按键 / 操作 | 效果 |
|---|---|
| `1` / `2` / `3` / `4` | 切换渲染模式（PerTriangle / TileBased / Deferred / TileBasedDeferred） |
| `W` `A` `S` `D` | 移动相机 |
| 右键拖动 | 旋转相机 |
| `V` | 切换 VSync 模拟（60 Hz） |
| `B` | 切换缓冲模式（双缓冲 / 三缓冲） |

---

## 测试

```bash
cargo test                               # 运行所有测试（223 个）
cargo test -p simple_renderer            # 仅运行库单元测试
cargo test --test integration_test       # 仅运行集成测试
cargo test --test property_tests         # 仅运行属性测试
cargo clippy --workspace -- -D warnings  # Lint 检查（CI 要求零警告）
```

测试套件包含 184 个单元测试、7 个集成测试、15 个属性测试（proptest）、16 个 system_test 测试和 1 个文档测试。集成测试使用内置的犹他茶壶模型在全部四种渲染模式下进行渲染验证。

---

## 理解渲染器

### 核心渲染管线

SimpleRenderer 的渲染管线模拟了典型的 GPU 渲染管线各阶段，清晰展示每个组件对最终渲染图像的贡献。

1. **顶点处理和变换**

   - **目标**：理解 3D 模型如何投影到 2D 屏幕。
   - **关键概念**：
     - **模型矩阵**：在世界空间中定位和定向模型。
     - **视图矩阵**：表示相机的位置和方向。
     - **投影矩阵**：定义相机镜头参数（视野角、宽高比）。

2. **图元组装和裁剪**

   - **目标**：了解顶点如何组成三角形，以及如何处理屏幕外的部分。
   - **关键概念**：
     - **三角形组装**：将顶点分组为可绘制的图元。
     - **裁剪**：丢弃或裁剪视锥体外的图元。

3. **光栅化和片段处理**

   - **目标**：了解三角形如何转换为像素数据。
   - **关键概念**：
     - **重心坐标**：用于在三角形上插值顶点属性。
     - **深度缓冲**：确保正确渲染重叠对象。
     - **片段着色器**：计算每个像素的颜色和其他属性。

4. **着色和光照模型**

   - **目标**：探索光照如何影响表面外观。
   - **关键概念**：
     - **Blinn-Phong 着色模型**：通过环境光、漫反射和镜面反射分量模拟真实光照。
     - **表面法线**：决定光线与表面的交互方式。
     - **光源**：了解不同类型的光（方向光、点光源、环境光）。

5. **优化技术**

   - **目标**：学习提高渲染效率的方法。
   - **关键概念**：
      - **背面剔除**：消除相机不可见的面。
      - **并行光栅化**：通过 rayon 并行处理扫描线。
      - **分片渲染**：将屏幕划分为 tile，实现缓存友好的并行处理。
      - **缓冲区复用**：渲染器持有深度/颜色缓冲区，每帧 `fill()` 重置而非重新分配。
      - **多缓冲**：无锁三缓冲允许渲染线程独立于显示刷新工作，避免管线停顿。

### 代码结构

本项目是一个 Cargo workspace，包含两个 crate：

#### `simple_renderer/` — 核心库

| 文件 | 作用 |
|---|---|
| `src/renderer.rs` | `SimpleRenderer`：模式选择和 `draw_model(&mut self)` 入口 |
| `src/renderers/` | 四种渲染策略，均实现 `Renderer` trait（`&mut self`） |
| `src/shader/` | 着色器模块目录 |
| `src/shader/mod.rs` | `Shader` 结构体、uniform 管理、Clone/Default |
| `src/shader/vertex.rs` | `vertex_shader(&self)`、矩阵缓存 |
| `src/shader/fragment.rs` | `fragment_shader(&self)`、Blinn-Phong 光照 |
| `src/shader/specular_lut.rs` | 高光 LUT 查找表、`RwLock` 缓存 |
| `src/rasterizer.rs` | 重心坐标插值，透视校正光栅化 |
| `src/model.rs` | OBJ 模型加载器（基于 tobj），带纹理缓存 |
| `src/buffer.rs` | 双缓冲帧缓冲（标志位交换，零拷贝） |
| `src/triple_buffer.rs` | 无锁三缓冲：`TripleBufferWriter` + `TripleBufferReader`，用于多线程渲染 |
| `src/vertex.rs` | `Vertex`（AoS）和 `VertexSoA`（SoA，供分片渲染器使用） |
| `src/fragment.rs` | 从光栅化器传递到片段着色器的片段数据 |
| `src/uniform.rs` | `UniformBuffer` + `uniform::names` 类型安全常量 |
| `src/material.rs` | `Material` 和 `Texture`（通过 image crate 加载） |
| `src/math.rs` | 重导出 glam 类型 |
| `src/light.rs` | `Light`（名称、位置、方向、颜色） |
| `src/face.rs` | `Face`（3 个顶点索引 + `Arc<Material>`） |
| `src/color.rs` | 32 位 RGBA 颜色，小端 u32 兼容（非小端编译报错） |
| `src/error.rs` | `RendererError`（thiserror）和 `Result<T>` 别名 |

#### `system_test/` — 交互式演示程序

| 文件 | 作用 |
|---|---|
| `src/main.rs` | 多线程渲染循环：主线程（输入/显示）+ 渲染线程（着色/光栅化） |
| `src/camera.rs` | FPS 风格自由相机（欧拉角） |
| `src/display.rs` | minifb 窗口、键盘/鼠标输入、渲染模式和缓冲模式切换 |

### 依赖

| Crate | 用途 |
|---|---|
| glam 0.29 | Vec3、Vec4、Mat4 数学运算 |
| tobj 4.0 | OBJ 模型加载 |
| image 0.25 | 纹理加载（PNG、JPEG、BMP、TGA） |
| rayon 1.10 | 并行迭代 |
| minifb 0.27 | 窗口和显示（仅 system_test 使用） |
| thiserror 2 | 错误派生宏 |
| log 0.4 + env_logger 0.11 | 日志 |
| proptest 1（dev） | 属性测试 |
| criterion 0.5（dev） | 基准测试 |

---

## 实验和学习

为最大化学习效果，建议尝试以下步骤：

- **修改着色器**

  在 `simple_renderer/src/shader/` 目录中实验着色器代码，`vertex.rs` 控制顶点变换，`fragment.rs` 控制光照计算。

- **调整变换**

  修改模型、视图和投影矩阵，理解它们对场景的影响。设置 uniform 时使用 `uniform::names` 常量避免拼写错误。

- **实现新功能**

  尝试添加新的光照模型、纹理或着色技术。

- **添加渲染策略**

  在 `simple_renderer/src/renderers/` 中实现 `Renderer` trait（`&mut self`），并在 `renderer.rs::create_renderer` 中注册。

---

## 贡献

您的贡献可以帮助他人学习。欢迎您：

- 提交改进或新教育功能的 Pull Request。
- 报告问题或提出建议。
- 分享您的学习体验。

---

## 许可证

本项目基于 MIT 许可证。详情请参阅 `LICENSE` 文件。
