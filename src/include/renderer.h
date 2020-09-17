
// This file is a part of SimpleXX/SimpleRenderer
// (https://github.com/SimpleXX/SimpleRenderer).
//
// renderer.h for SimpleXX/SimpleRenderer.

#ifndef __RENDERER_H__
#define __RENDERER_H__

#include "vector"
#include "model.h"
#include "vector.hpp"
#include "geometry.h"

class Renderer {
private:
    // 要渲染的模型
    const Model &model;
    // 着色器?
    // 绘制像素
    Geometry &painter;
    // 光照
    Vectord3 light_dir = Vectord3(0, 0, -1);
    // z-buffer 缓冲
    uint16_t *zbuffer;
    // 大小
    size_t         width;
    size_t         height;
    const TGAColor black = TGAColor(0, 0, 0, 255);
    const TGAColor white = TGAColor(255, 255, 255, 255);
    const TGAColor red   = TGAColor(255, 0, 0, 255);

protected:
public:
    Renderer(Geometry &_painter2, const Model &_model);
    ~Renderer(void);
    // 渲染
    bool render(void) const;
    // 描线
    bool line(void) const;
    // 填充
    bool fill(void) const;
    // 保存
    bool save(const std::string &_filename = "output.tga") const;
    // 设置参数
    bool set_light(const Vectord3 &_light);
    bool set_size(size_t _w, size_t _h);
    // 读取参数
    Vectord3 get_light(void) const;
    Vectori2 get_size(void) const;
};

#endif /* __RENDER_H__ */
