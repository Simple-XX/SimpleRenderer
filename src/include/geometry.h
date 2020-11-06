
// This file is a part of Simple-XX/SimpleRenderer
// (https://github.com/Simple-XX/SimpleRenderer).
//
// geometry.h for Simple-XX/SimpleRenderer.

#ifndef __GEOMTRY_H__
#define __GEOMTRY_H__

#include "string"
#include "common.h"
#include "image.h"
#include "vector.hpp"

// 负责确定几何图形的绘制
class Geometry {
private:
    // 判断 _p 是否为三角形重心
    bool is_barycentric(const Vectors2 &_vertex1, const Vectors2 &_vertex2,
                        const Vectors2 &_vertex3, const Vectors2 &_p) const;
    bool is_barycentric(const Vectorf3 &_vertex1, const Vectorf3 &_vertex2,
                        const Vectorf3 &_vertex3, const Vectorf3 &_p) const;
    // 获取最小 (x,y)
    Vectors2 get_min(const Vectors2 &_vertex1, const Vectors2 &_vertex2,
                     const Vectors2 &_vertex3) const;
    // 获取最大 (x,y)
    Vectors2 get_max(const Vectors2 &_vertex1, const Vectors2 &_vertex2,
                     const Vectors2 &_vertex3) const;
    // 获取最小 (x,y,z)
    Vectorf3 get_min(const Vectorf3 &_vertex1, const Vectorf3 &_vertex2,
                     const Vectorf3 &_vertex3) const;
    // 获取最大 (x,y,z)
    Vectorf3 get_max(const Vectorf3 &_vertex1, const Vectorf3 &_vertex2,
                     const Vectorf3 &_vertex3) const;
    // 求平面法向量
    Vectorf3 normal(const Vectorf3 &_vertex1, const Vectorf3 &_vertex2,
                    const Vectorf3 &_vertex3) const;
    // 计算平面上 (x, y) 位置的深度
    float get_z(const Vectorf3 &_vertex1, const Vectorf3 &_vertex2,
                const Vectorf3 &_vertex3, size_t _x, size_t _y) const;

protected:
    TGAImage &  image;
    std::string filename = "output.tga";
    // 背景色默认黑色
    const TGAColor color_bg = black;

public:
    Geometry(TGAImage &_image, const std::string &_filename = "output.tga");
    virtual ~Geometry(void);
    // 获取画布宽度
    size_t get_width(void) const;
    // 获取画布高度
    size_t get_height(void) const;
    // 保存修改
    bool save(std::string _filename = "output.tga") const;

    // 直线 Bresenham
    void line(size_t _x0, size_t _y0, size_t _x1, size_t _y1,
              const TGAColor &_color) const;
    void line(Vectors2 _v0, Vectors2 _v1, const TGAColor &_color) const;
    void line(Vectors2 *_vertexes, const TGAColor &_color) const;

    // 带 zbufferr 的直线
    void line(float _x0, float _y0, float _z0, float _x1, float _y1, float _z1,
              float *_zbuffer, const TGAColor &_color);
    void line(Vectorf3 _v0, Vectorf3 _v1, float *_zbuffer,
              const TGAColor &_color);
    void line(Vectorf3 *_vertexes, float *_zbuffer, const TGAColor &_color);

    // 三角形
    void triangle(const Vectors2 &_vertex1, const Vectors2 &_vertex2,
                  const Vectors2 &_vertex3, const TGAColor &_color) const;
    void triangle(const Vectors2 *_vertexes, const TGAColor &_color) const;

    // 带 zbufferr 的三角形
    void triangle(const Vectorf3 &_vertex1, const Vectorf3 &_vertex2,
                  const Vectorf3 &_vertex3, float *_zbuffer,
                  const TGAColor &_color) const;
    void triangle(const Vectorf3 *_vertexes, float *_zbuffer,
                  const TGAColor &_color);

    // 圆 Bresenham
    void circle(size_t _x0, size_t _y0, float _r, TGAColor _color) const;

    // 带 zbufferr 的圆
    void circle(size_t _x0, size_t _y0, float _r, float *_zbuffer,
                const TGAColor &_color) const;
};

#endif /* __GEOMTRY_H__ */
