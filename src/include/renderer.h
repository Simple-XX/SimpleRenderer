
// This file is a part of SimpleXX/SimpleRenderer
// (https://github.com/SimpleXX/SimpleRenderer).
//
// renderer.h for SimpleXX/SimpleRenderer.

#ifndef __RENDERER_H__
#define __RENDERER_H__

#include "vector"
#include "common.h"
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
    Vectorf3 light_dir = Vectorf3(0, 0, -1);
    // z-buffer 缓冲
    double *zbuffer;
    // 大小
    size_t width;
    size_t height;
    // 由 obj 坐标转换为屏幕坐标
    int get_x(float _x) const;
    int get_y(float _y) const;

protected:
public:
    Renderer(Geometry &_painter2, const Model &_model);
    ~Renderer(void);
    // 渲染
    bool render(void) const;
    bool render(void);
    // 直线
    bool line(void) const;
    bool line_zbuffer();
    // 填充三角
    bool fill(void) const;
    bool fill_zbuffer();
    // 圆
    bool circle(void) const;
    // 保存
    bool save(const std::string &_filename = "output.tga") const;
    // 设置参数
    bool set_light(const Vectorf3 &_light);
    bool set_size(size_t _w, size_t _h);
    // 读取参数
    Vectorf3 get_light(void) const;
    Vectori2 get_size(void) const;
};

#endif /* __RENDER_H__ */
