
// This file is a part of SimpleXX/SimpleRenderer
// (https://github.com/SimpleXX/SimpleRenderer).
//
// geometry.cpp for SimpleXX/SimpleRenderer.

#include "iostream"
#include "geometry.h"
#include "vector"

using namespace std;

Geometry::Geometry(TGAImage &_image, const string &_filename)
    : image(_image), filename(_filename) {
    for (auto w = 0; w < image.get_width(); w++) {
        for (auto h = 0; h < image.get_height(); h++) {
            image.set(w, h, color_bg);
        }
    }
    return;
}

Geometry::~Geometry(void) {
    return;
}

// 推导过程见
bool Geometry::is_barycentric(const Vectori2 &_vertex1,
                              const Vectori2 &_vertex2,
                              const Vectori2 &_vertex3,
                              const Vectori2 &_p) const {
    // 边向量
    Vectori2 edge1 = (_vertex3 - _vertex1);
    Vectori2 edge2 = (_vertex2 - _vertex1);
    // 到点 P 的向量
    Vectori2 edge3 = (_p - _vertex1);
    double   v1v1  = edge1 * edge1;
    double   v1v2  = edge1 * edge2;
    double   v1v3  = edge1 * edge3;
    double   v2v2  = edge2 * edge2;
    double   v2v3  = edge2 * edge3;

    double deno = (v1v1 * v2v2) - (v1v2 * v1v2);
    double u    = ((v2v2 * v1v3) - (v1v2 * v2v3)) / deno;
    if (u < 0. || u > 1.) {
        return false;
    }
    double v = ((v1v1 * v2v3) - (v1v2 * v1v3)) / deno;
    if (v < 0. || v > 1.) {
        return false;
    }
    if (u + v >= 1.) {
        return false;
    }
    return true;
}

bool Geometry::is_barycentric(const Vectori3 &_vertex1,
                              const Vectori3 &_vertex2,
                              const Vectori3 &_vertex3,
                              const Vectori3 &_p) const {
    // 边向量
    Vectori3 edge1 = (_vertex3 - _vertex1);
    Vectori3 edge2 = (_vertex2 - _vertex1);
    // 到点 P 的向量
    Vectori3 edge3 = (_p - _vertex1);
    double   v1v1  = edge1 * edge1;
    double   v1v2  = edge1 * edge2;
    double   v1v3  = edge1 * edge3;
    double   v2v2  = edge2 * edge2;
    double   v2v3  = edge2 * edge3;

    double deno = (v1v1 * v2v2) - (v1v2 * v1v2);
    double u    = ((v2v2 * v1v3) - (v1v2 * v2v3)) / deno;
    if (u < 0. || u > 1.) {
        return false;
    }
    double v = ((v1v1 * v2v3) - (v1v2 * v1v3)) / deno;
    if (v < 0. || v > 1.) {
        return false;
    }
    if (u + v >= 1.) {
        return false;
    }
    return true;
}

Vectori2 Geometry::get_min(const Vectori2 &_vertex1, const Vectori2 &_vertex2,
                           const Vectori2 &_vertex3) const {
    int x = min(_vertex1.coord.x, min(_vertex2.coord.x, _vertex3.coord.x));
    int y = min(_vertex1.coord.y, min(_vertex2.coord.y, _vertex3.coord.y));
    return Vectori2(x, y);
}

Vectori2 Geometry::get_max(const Vectori2 &_vertex1, const Vectori2 &_vertex2,
                           const Vectori2 &_vertex3) const {
    int x = max(_vertex1.coord.x, max(_vertex2.coord.x, _vertex3.coord.x));
    int y = max(_vertex1.coord.y, max(_vertex2.coord.y, _vertex3.coord.y));
    return Vectori2(x, y);
}

Vectori3 Geometry::get_min(const Vectori3 &_vertex1, const Vectori3 &_vertex2,
                           const Vectori3 &_vertex3) const {
    int x = min(_vertex1.coord.x, min(_vertex2.coord.x, _vertex3.coord.x));
    int y = min(_vertex1.coord.y, min(_vertex2.coord.y, _vertex3.coord.y));
    int z = min(_vertex1.coord.z, min(_vertex2.coord.z, _vertex3.coord.z));
    return Vectori3(x, y, z);
}

Vectori3 Geometry::get_max(const Vectori3 &_vertex1, const Vectori3 &_vertex2,
                           const Vectori3 &_vertex3) const {
    int x = max(_vertex1.coord.x, max(_vertex2.coord.x, _vertex3.coord.x));
    int y = max(_vertex1.coord.y, max(_vertex2.coord.y, _vertex3.coord.y));
    int z = max(_vertex1.coord.z, max(_vertex2.coord.z, _vertex3.coord.z));
    return Vectori3(x, y, z);
}

// 在给定 _image 上按照 _x0, _y0, _x1, _y1 给出的坐标绘制直线，颜色由 _color
// 指定
// [(_x0, _y0), (_x1, _y1)) 左上角为原点
void Geometry::line(int _x0, int _y0, int _x1, int _y1,
                    const TGAColor &_color) const {
    // 斜率大于 1
    bool steep = false;
    if (abs(_x0 - _x1) < abs(_y0 - _y1)) {
        swap(_x0, _y0);
        swap(_x1, _y1);
        steep = true;
    }
    // 终点 x 坐标在起点 左边
    if (_x0 > _x1) {
        swap(_x0, _x1);
        swap(_y0, _y1);
    }
    int de  = 0;
    int dy2 = (_y1 - _y0) << 1;
    int dx2 = (_x1 - _x0) << 1;
    int y   = _y0;
    for (int x = _x0; x <= _x1; x++) {
        if (steep == true) {
            image.set(y, image.get_height() - x, _color);
        }
        else {
            image.set(x, image.get_height() - y, _color);
        }
        de += abs(dy2);
        if (de > 1) {
            y += (_y1 > _y0 ? 1 : -1);
            de -= dx2;
        }
    }
    return;
}

// 绘制三角形，_vertex1 _vertex2 _vertex3 为三个顶点
void Geometry::triangle(const Vectori2 &_vertex1, const Vectori2 &_vertex2,
                        const Vectori2 &_vertex3,
                        const TGAColor &_color) const {
    // 遍历平行四边形，如果在三角形内则填充
    Vectori2 min = get_min(_vertex1, _vertex2, _vertex3);
    Vectori2 max = get_max(_vertex1, _vertex2, _vertex3);
    Vectori2 p;
    for (p.coord.x = min.coord.x; p.coord.x <= max.coord.x; p.coord.x++) {
        for (p.coord.y = min.coord.y; p.coord.y <= max.coord.y; p.coord.y++) {
            if (is_barycentric(_vertex1, _vertex2, _vertex3, p) == true) {
                image.set(p.coord.x, image.get_height() - p.coord.y, _color);
            }
        }
    }
    return;
}

void Geometry::triangle(const Vectori2 *_vertexes,
                        const TGAColor &_color) const {
    triangle(_vertexes[0], _vertexes[1], _vertexes[2], _color);
    return;
}

void Geometry::triangle(const Vectori3 &_vertex1, const Vectori3 &_vertex2,
                        const Vectori3 &_vertex3, double *_zbuffer,
                        const TGAColor &_color) const {
    Vectori3 min = get_min(_vertex1, _vertex2, _vertex3);
    Vectori3 max = get_max(_vertex1, _vertex2, _vertex3);
    Vectori3 p;
    for (p.coord.x = min.coord.x; p.coord.x <= max.coord.x; p.coord.x++) {
        for (p.coord.y = min.coord.y; p.coord.y <= max.coord.y; p.coord.y++) {
            if (is_barycentric(_vertex1, _vertex2, _vertex3, p) == true) {
                image.set(p.coord.x, image.get_height() - p.coord.y, _color);
            }
        }
    }
    return;
}

void Geometry::triangle(const Vectori3 *_vertexes, double *_zbuffer,
                        const TGAColor &_color) const {
    triangle(_vertexes[0], _vertexes[1], _vertexes[2], _zbuffer, _color);
    return;
}

void Geometry::circle(int _x0, int _y0, double _r, TGAColor _color) const {
    assert(_r < 0);
    return;
}

void Geometry::circle(int _x0, int _y0, double _r, double *_zbuffer,
                      const TGAColor &_color) const {
    assert(_r < 0);
    return;
}

int Geometry::get_width(void) const {
    return image.get_width();
}

int Geometry::get_height(void) const {
    return image.get_height();
}

bool Geometry::save(string _filename) const {
    return image.write_tga_file(_filename);
}
