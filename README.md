![workflow](https://github.com/Simple-XX/SimpleGameEngine/actions/workflows/workflow.yml/badge.svg)
![MIT License](https://img.shields.io/github/license/mashape/apistatus.svg)

# SimpleGameEngine

[![cn](https://img.shields.io/badge/language-Chinese-pink.svg)](https://github.com/Simple-XX/SimpleGameEngine/blob/main/README.md)
[![en](https://img.shields.io/badge/language-English-lightblue.svg)](https://github.com/Simple-XX/SimpleGameEngine/blob/main/README_ENG.md)

一个模块化的教育游戏引擎，基于软件渲染器实现，帮助开发者理解渲染管线和游戏引擎架构。

## 概述

SimpleGameEngine 是一个以教育为核心目标的模块化游戏引擎，采用分层架构设计，将 ECS、渲染、输入、场景管理等子系统解耦为独立 crate。内置的软件渲染器完整模拟了 GPU 渲染管线（顶点着色 → 光栅化 → 片段着色 → 深度测试），帮助开发者从底层理解 3D 图形学原理。

本项目使用最小化 `unsafe` Rust 实现，采用 Cargo workspace 结构，包含 7 个 crate。最低支持 Rust 1.73+。

### 设计目标

- **模块化**：每个子系统是独立 crate，通过 trait 接口连接，可按需引入
- **教育性**：代码可读，每个模块可独立学习；软件渲染保留为教学参考实现
- **可扩展**：Plugin 系统支持第三方扩展，渲染后端可切换（RenderBackend trait）

### 主要功能

- **hecs ECS**：基于 archetype 的实体组件系统，高性能且源码可读（~2000 行）
- **Plugin 架构**：Plugin trait + AppBuilder + Phase-based Schedule，模块化注册系统
- **四种软件渲染策略**：PerTriangle / TileBased / Deferred / TileBasedDeferred，运行时切换
- **Blinn-Phong 着色**：完整的顶点/片段着色器，环境光 + 漫反射 + 镜面反射
- **RenderBackend 抽象**：统一的渲染后端接口，支持未来接入 wgpu 等 GPU 后端
- **多线程渲染**：独立渲染线程通过无锁三缓冲与编辑器 UI 解耦
- **egui 编辑器**：场景树 + 属性面板 + 渲染视口 + FPS 相机控制
- **数学工具**：AABB、Ray、Frustum 几何体 + glam 线性代数
- **统一输入管理**：KeyboardState / MouseState，支持 just_pressed / just_released

---

## 架构

### 分层设计

```
Layer 4 — Application
  engine_editor       egui 编辑器 GUI

Layer 2 — Infrastructure
  engine_scene        Scene + AssetManager + Components
  engine_render_sw    软件渲染后端

Layer 1 — Foundation（互不依赖）
  engine_core         hecs ECS + Plugin + Schedule + EventBus + ResourceMap
  engine_math         glam re-export + AABB / Ray / Frustum
  engine_render_api   RenderBackend trait + FrameData + 数据类型
  engine_input        键盘 / 鼠标状态管理
```

Layer 1 的 4 个基础 crate **互不依赖**，上层只依赖下层。

### Crate 概览

| Crate | 职责 |
|-------|------|
| `engine_core` | hecs ECS、Plugin/Schedule/EventBus/ResourceMap、Transform 层级、Handle\<T\> 资产句柄 |
| `engine_math` | glam 数学库 re-export + AABB/Ray/Frustum 几何工具 |
| `engine_render_api` | RenderBackend trait、FrameData/DrawCall、MeshHandle/TextureHandle/MaterialHandle |
| `engine_input` | KeyCode/KeyboardState、MouseButton/MouseState、InputState |
| `engine_render_sw` | 软件渲染器：4 种渲染模式、Blinn-Phong 着色、三缓冲 |
| `engine_scene` | Scene（hecs World + AssetManager）、MeshRenderer/Camera/Light 组件 |
| `engine_editor` | eframe/egui 编辑器：渲染视口、场景树、属性面板、FPS 相机 |

---

## 快速开始

### 前置条件

确保已安装 Rust 1.73+ 和 Cargo：

```bash
curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sh
```

### 构建与运行

```bash
git clone https://github.com/Simple-XX/SimpleGameEngine.git
cd SimpleGameEngine

cargo build --workspace               # 构建全部 crate
cargo run -p engine_editor             # 运行 egui 编辑器
```

### 操作说明

| 按键 / 操作 | 效果 |
|---|---|
| `W` `A` `S` `D` | 移动相机 |
| 右键拖动 | 旋转相机 |
| `1` / `2` / `3` / `4` | 切换渲染模式 |
| `V` | 切换 VSync 模拟 |
| `B` | 切换缓冲模式（双缓冲 / 三缓冲） |

---

## 测试

```bash
cargo test --workspace                       # 全部 363 个测试
cargo test -p engine_core                    # ECS + Plugin + Schedule (63)
cargo test -p engine_math                    # AABB / Ray / Frustum (39)
cargo test -p engine_input                   # 输入状态 (20)
cargo test -p engine_render_api              # 渲染 API 类型 (13)
cargo test -p engine_render_sw               # 软件渲染器 (207)
cargo test -p engine_scene                   # 场景管理 (5)
cargo clippy --workspace -- -D warnings      # Lint（CI 要求零警告）
```

---

## 代码结构

```
SimpleGameEngine/
├── Cargo.toml                    # Workspace root
├── crates/
│   ├── engine_math/              # glam re-export + AABB/Ray/Frustum
│   ├── engine_core/              # hecs ECS + Plugin + Schedule + EventBus + ResourceMap
│   ├── engine_render_api/        # RenderBackend trait + FrameData + 数据类型
│   ├── engine_input/             # KeyboardState + MouseState
│   ├── engine_render_sw/         # 软件渲染后端（4 种渲染策略）
│   ├── engine_scene/             # Scene + AssetManager + Components
│   └── engine_editor/            # egui 编辑器 GUI
├── assets/models/                # 内置 3D 模型 (.obj/.mtl)
└── docs/                         # 设计文档 + 实施计划
```

---

## 核心渲染管线

软件渲染器模拟 GPU 渲染管线：

**顶点着色器 → 透视除法 → 视口变换 → 光栅化 → 片段着色器 → 深度测试 → 帧缓冲**

四种渲染策略：

| 模式 | 特点 |
|------|------|
| PerTriangle | 按三角形并行的前向渲染 |
| TileBased | 基于瓦片并行，支持 Early-Z |
| Deferred | 延迟着色（仅着色通过深度测试的像素） |
| TileBasedDeferred | 瓦片 + 延迟（结合两者优势） |

---

## 依赖

| Crate | 用途 |
|---|---|
| hecs 0.10 | Archetype ECS |
| glam 0.29 | 线性代数 (Vec3/Mat4) |
| tobj 4.0 | OBJ 模型加载 |
| image 0.25 | 纹理加载 (PNG/JPEG/BMP/TGA) |
| rayon 1.10 | 并行迭代 |
| thiserror 2 | 错误派生 |
| eframe/egui 0.31 | 编辑器 GUI |
| rfd 0.15 | 原生文件对话框 |

---

## 贡献

欢迎提交 Pull Request、报告问题或分享学习体验。

## 许可证

MIT License — 详情参阅 `LICENSE` 文件。
