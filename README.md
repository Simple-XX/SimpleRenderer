[![codecov](https://codecov.io/gh/Simple-XX/SimpleRenderer/graph/badge.svg?token=J7NKK3SBNJ)](https://codecov.io/gh/Simple-XX/SimpleRenderer)
![workflow](https://github.com/Simple-XX/SimpleRenderer/actions/workflows/workflow.yml/badge.svg)
![commit-activity](https://img.shields.io/github/commit-activity/t/Simple-XX/SimpleRenderer)
![last-commit-build](https://img.shields.io/github/last-commit/Simple-XX/SimpleRenderer/build)
![MIT License](https://img.shields.io/github/license/mashape/apistatus.svg)
[![LICENSE](https://img.shields.io/badge/license-Anti%20996-blue.svg)](https://github.com/996icu/996.ICU/blob/master/LICENSE)
[![996.icu](https://img.shields.io/badge/link-996.icu-red.svg)](https://996.icu)

# SimpleRenderer

A software renderer

软件光栅化渲染器

本体只做一件事：接收输入，渲染并将结果保存到指定内存中

1. 接受输入
   obj/贴图/光源 等
2. 渲染
   直线、曲线、填充、着色等图形学方法
3. 保存到指定内存
   写到输出缓冲区

在测试中，应当有输入输入的处理

1. 对 camera 的处理
2. 不同 camera 的支持（环绕、top-down 等）

为了实现以上目的，在渲染器初始化时应有以下参数

- 渲染长度
  渲染画面的长度

- 渲染宽度
  渲染画面的宽度

- 渲染目标帧率
  最大帧率限制，缺省时不限制

- 绘制方式
  绘制线框或进行填充

- 输出缓冲区
  用户输出结果的一段内存

- 绘制方式
  指定如何写像素的底层接口

类接口

- 添加 obj 对象
- 执行渲染
- 获取渲染结果
-

示例接口

```c++

enum draw_type {
  LINE,
  DRAW,
};

typedef void (*draw_func)(uint64_t _x, uint64_t _y, uint32_t color);

struct paras {
  uint64_t height;
  uint64_t width;
  uint64_t fps;
  draw_type type;
  void *out_put_buffer;
  draw_func *func;
};

SimpleRenderer::SimpleRenderer(const paras&);
```

应对单次渲染很简单，如果是连续渲染该怎么处理呢？
深度缓冲区怎么处理呢？

渲染管线：

1. 应用层
    场景大小
    场景中的物体
    物体位置
    摄像机位置
    光源信息

2. 几何层

    1. Vertex Shading
        计算每一个顶点的光照信息

    2. MVP
        模型、视窗、投影变换

    3. Clipping
        裁剪视锥外的物体

    4. 窗口映射
        将计算好的数据映射到屏幕上

3. 光栅化
    计算所有图元（像素）的颜色

4. 像素处理
    使用着色器(PBR)对所有图元的颜色进行处理，计算出最终颜色，并写回颜色缓冲区（输出）




整体流程

1. 解析 obj 文件
2. 设置 mvp 矩阵
3. 应用变换
4. 将变换好的数据交给 render
5. render 只处理绘制相关的内容，不进行变换
6. 访问输出缓冲获取结果并显示

缓冲
保存像素与深度信息，同时实现了 场景-模型-三角形-直线 四级绘制方法，
配置
保存了全局的信息 分辨率，绘制方法等
显示
将缓冲区的数据显示到屏幕上
光照
保存光线的颜色，位置，方向信息
模型
从 obj 文件读取数据保存到内存中，同时会计算法向量，碰撞盒
相机
相机的位置，方向，移动方式等
需要搞定 lookat 矩阵
着色
顶点着色与片元着色，
将 mvp 矩阵应用到每一个顶点上，
再使用片元着色确定像素的深度和颜色绘制三角形

输入 - 场景
将输入应用到场景上
根据场景的不同，使用的配置和输入不同
传入场景作为参数，在输入处理中对场景的相机、配置等进行修改
done
场景 - 配置 相机 输入 模型 光照
场景中包含了所有相机，模型，光照的信息，
绘制 - 场景 缓冲 着色
根据配置，使用指定的着色器将指定场景绘制到缓冲区中
渲染 - 缓冲 输入 场景 显示
将绘制好的缓冲区显示出来

读取模型，放入场景，初始化配置，相机

参考：https://github.com/ssloy/tinyrenderer

正在开发中...

数字键 1：绘制线框
数字键 2：绘制三角形
TAB：切换模型

sudo apt install doxygen cppcheck clang-tidy clang-format lcov libsdl2-dev libspdlog-dev

```shell
brew install gcc g++ cmake doxygen graphviz cppcheck llvm lcov sdl2 spdlog libomp
CC=gcc-13 CXX=g++-13 cmake --preset build
```

## Refs

- 架构相关
  https://zhuanlan.zhihu.com/p/536810232