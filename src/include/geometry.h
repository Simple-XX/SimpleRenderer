
// This file is a part of SimpleXX/SimpleRenderer
// (https://github.com/SimpleXX/SimpleRenderer).
//
// geometry.h for SimpleXX/SimpleRenderer.

#ifndef __GEOMTRY_H__
#define __GEOMTRY_H__

#include "image.h"
#include "string"
#include "vector.hpp"

class Geometry {
private:
    // 判断 _p 是否为三角形重心
    bool is_barycentric(const Vectori2 &_vertex1, const Vectori2 &_vertex2,
                        const Vectori2 &_vertex3, const Vectori2 &_p) const;
    bool is_barycentric(const Vectorf2 &_vertex1, const Vectorf2 &_vertex2,
                        const Vectorf2 &_vertex3, const Vectorf2 &_p) const;
    bool is_barycentric(const Vectori3 &_vertex1, const Vectori3 &_vertex2,
                        const Vectori3 &_vertex3, const Vectori3 &_p) const;
    bool is_barycentric(const Vectorf3 &_vertex1, const Vectorf3 &_vertex2,
                        const Vectorf3 &_vertex3, const Vectorf3 &_p) const;
    // 获取最小 (x,y)
    Vectori2 get_min(const Vectori2 &_vertex1, const Vectori2 &_vertex2,
                     const Vectori2 &_vertex3) const;
    // 获取最大 (x,y)
    Vectori2 get_max(const Vectori2 &_vertex1, const Vectori2 &_vertex2,
                     const Vectori2 &_vertex3) const;
    // 获取最小 (x,y,z)
    Vectori3 get_min(const Vectori3 &_vertex1, const Vectori3 &_vertex2,
                     const Vectori3 &_vertex3) const;
    Vectorf3 get_min(const Vectorf3 &_vertex1, const Vectorf3 &_vertex2,
                     const Vectorf3 &_vertex3) const;
    // 获取最大 (x,y,z)
    Vectori3 get_max(const Vectori3 &_vertex1, const Vectori3 &_vertex2,
                     const Vectori3 &_vertex3) const;
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
    const TGAColor color_bg = TGAColor(0, 0, 0, 255);

public:
    Geometry(TGAImage &_image, const std::string &_filename = "output.tga");
    virtual ~Geometry(void);
    // 直线 Bresenham
    void line(int _x0, int _y0, int _x1, int _y1, const TGAColor &_color) const;
    void line(Vectori2 _v0, Vectori2 _v1, const TGAColor &_color) const;

    // 带 zbufferr 的直线
    void line(double _x0, double _y0, double _z0, double _x1, double _y1,
              double _z1, double *_zbuffer, const TGAColor &_color) const;
    void line(Vectorf3 _v0, Vectorf3 _v1, double *_zbuffer,
              const TGAColor &_color) const;

    // 三角形
    void triangle(const Vectori2 &_vertex1, const Vectori2 &_vertex2,
                  const Vectori2 &_vertex3, const TGAColor &_color) const;
    void triangle(const Vectori2 *_vertexes, const TGAColor &_color) const;

    // 带 zbufferr 的三角形
    void triangle(const Vectori3 &_vertex1, const Vectori3 &_vertex2,
                  const Vectori3 &_vertex3, double *_zbuffer,
                  const TGAColor &_color) const;
    void triangle(const Vectori3 *_vertexes, double *_zbuffer,
                  const TGAColor &_color) const;
    void triangle(const Vectorf3 &_vertex1, const Vectorf3 &_vertex2,
                  const Vectorf3 &_vertex3, double *_zbuffer,
                  const TGAColor &_color) const;

    // 圆 Bresenham
    void circle(int _x0, int _y0, float _r, TGAColor _color) const;
    // 带 zbufferr 的圆
    void circle(int _x0, int _y0, float _r, double *_zbuffer,
                const TGAColor &_color) const;

    int  get_width(void) const;
    int  get_height(void) const;
    bool save(std::string _filename = "output.tga") const;
};

#endif /* __GEOMTRY_H__ */
