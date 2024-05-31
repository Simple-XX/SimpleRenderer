[![codecov](https://codecov.io/gh/Simple-XX/SimpleRenderer/graph/badge.svg?token=J7NKK3SBNJ)](https://codecov.io/gh/Simple-XX/SimpleRenderer)
![workflow](https://github.com/Simple-XX/SimpleRenderer/actions/workflows/workflow.yml/badge.svg)
![commit-activity](https://img.shields.io/github/commit-activity/t/Simple-XX/SimpleRenderer)
![last-commit-main](https://img.shields.io/github/last-commit/Simple-XX/SimpleRenderer/main)
![MIT License](https://img.shields.io/github/license/mashape/apistatus.svg)
[![LICENSE](https://img.shields.io/badge/license-Anti%20996-blue.svg)](https://github.com/996icu/996.ICU/blob/master/LICENSE)
[![996.icu](https://img.shields.io/badge/link-996.icu-red.svg)](https://996.icu)

# SimpleRenderer

A software renderer

一个软光栅化渲染库

提供模型信息由库进行绘制，返回绘制好的内存。

## 控制流

<img src='https://g.gravizo.com/svg?
  @startuml;
  start
  :SimpleRenderer(size_t, size_t, std::span<uint32_t> &, DrawPixelFunc);
  :render(const Model &);
  :DrawModel(const ShaderBase &, const Light &, const Model &, bool, bool);
  stop
  @enduml
'>

<img src='https://g.gravizo.com/svg?
  @startuml;
  start
  :DrawModel(const ShaderBase &, const Light &, const Model &, bool, bool);
  if (draw_line) then (true)
      repeat
          :model.GetFace();
          :shader.Vertex(ShaderVertexIn(f)).face_;
          :DrawLine(float, float, float, float, const Color &);
      repeat while (model.GetFace() != nullptr)
  endif
  if (draw_triangle) then (true)
      repeat
          :model.GetFace();
          :shader.Vertex(ShaderVertexIn(f)).face_;
          :DrawTriangle(const ShaderBase &, const Light &, const Model::Normal &, const Model::Face &);
      repeat while (model.GetFace() != nullptr)
  endif
  stop
  @enduml
'>


## 依赖

```shell
sudo apt install doxygen graphviz clang-format clang-tidy cppcheck lcov gcc g++ libsdl2-dev libsdl2-ttf-dev libomp-dev libspdlog-dev cmake vim
```

## 使用

参考 system_test
