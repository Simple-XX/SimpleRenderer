//
//// This file is a part of Simple-XX/SimpleRenderer
//// (https://github.com/Simple-XX/SimpleRenderer).
////
//// renderer.h for Simple-XX/SimpleRenderer.
//
// #ifndef __RENDERER_H__
// #define __RENDERER_H__
//
// #include "vector"
// #include "common.h"
// #include "model.h"
// #include "vector.hpp"
// #include "geometry.h"
//
//// 负责渲染模型到指定画布
// class Renderer {
// private:
//     // 要渲染的模型
//     const Model &model;
//     // 着色器?
//     // 绘制像素
//     Geometry &painter;
//     // 光照
//     Vectorf3 light_dir = Vectorf3(0, 0, -1);
//     // z-buffer 缓冲
//     float *zbuffer;
//     // 大小
//     size_t width;
//     size_t height;
//     // 由 obj 坐标转换为屏幕坐标
//     size_t get_x(float _x) const;
//     size_t get_y(float _y) const;
//
// protected:
// public:
//     Renderer(Geometry &_painter2, const Model &_model);
//     ~Renderer(void);
//     // 渲染
//     bool render(void);
//     bool render(void) const;
//     // 设置参数
//     bool set_light(const Vectorf3 &_light);
//     bool set_size(size_t _w, size_t _h);
//     // 读取参数
//     Vectorf3 get_light(void) const;
//     Vectors2 get_size(void) const;
//     // 保存
//     bool save(const std::string &_filename = "output.tga") const;
//
//     // 直线
//     bool line(void) const;
//     // 带 zbuffer 的直线
//     bool line_zbuffer();
//     // TODO: 升级成任意曲线
//     // 圆
//     bool circle(void) const;
//     // 填充圆
//     bool fill_circle(void) const;
//     // 带 zbuffer 的圆
//     bool fill_circle_zbuffer(void);
//
//     // 填充三角
//     bool fill_triangle(void) const;
//     // 带 zbuffer 的三角
//     bool fill_triangle_zbuffer(void);
// };
//
// #endif /* __RENDER_H__ */
