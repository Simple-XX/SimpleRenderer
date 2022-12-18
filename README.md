![github ci](https://github.com/Simple-XX/SimpleKernel/workflows/CMake/badge.svg)
![last-commit](https://img.shields.io/github/last-commit/google/skia.svg)
![languages](https://img.shields.io/github/languages/count/badges/shields.svg)
![MIT License](https://img.shields.io/github/license/mashape/apistatus.svg)

# SimpleRenderer

A software renderer

软件光栅化渲染器

缓冲
配置
显示
光照
模型
相机
着色

输入 - 场景
将输入应用到场景上
根据场景的不同，使用的配置和输入不同
场景 - 配置 相机 输入 模型 光照
场景中包含了所有相机，模型，光照的信息，
绘制 - 场景 缓冲 着色
根据配置，使用指定的着色器将指定场景绘制到缓冲区中
渲染 - 缓冲 输入 场景 显示
将绘制好的缓冲区显示出来

读取模型，放入场景，初始化配置，相机

参考：https://github.com/ssloy/tinyrenderer

正在开发中...
