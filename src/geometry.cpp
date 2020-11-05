
// This file is a part of SimpleXX/SimpleRenderer
// (https://github.com/SimpleXX/SimpleRenderer).
//
// geometry.cpp for SimpleXX/SimpleRenderer.

#include "iostream"
#include "geometry.h"
#include "vector"

using namespace std;

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

bool Geometry::is_barycentric(const Vectord2 &_vertex1,
                              const Vectord2 &_vertex2,
                              const Vectord2 &_vertex3,
                              const Vectord2 &_p) const {
    // 边向量
    Vectord2 edge1 = (_vertex3 - _vertex1);
    Vectord2 edge2 = (_vertex2 - _vertex1);
    // 到点 P 的向量
    Vectord2 edge3 = (_p - _vertex1);
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

bool Geometry::is_barycentric(const Vectord3 &_vertex1,
                              const Vectord3 &_vertex2,
                              const Vectord3 &_vertex3,
                              const Vectord3 &_p) const {
    // 边向量
    Vectord3 edge1 = (_vertex3 - _vertex1);
    Vectord3 edge2 = (_vertex2 - _vertex1);
    // 到点 P 的向量
    Vectord3 edge3 = (_p - _vertex1);
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

Vectord3 Geometry::get_min(const Vectord3 &_vertex1, const Vectord3 &_vertex2,
                           const Vectord3 &_vertex3) const {
    double x = min(_vertex1.coord.x, min(_vertex2.coord.x, _vertex3.coord.x));
    double y = min(_vertex1.coord.y, min(_vertex2.coord.y, _vertex3.coord.y));
    double z = min(_vertex1.coord.z, min(_vertex2.coord.z, _vertex3.coord.z));
    return Vectord3(x, y, z);
}

Vectori3 Geometry::get_max(const Vectori3 &_vertex1, const Vectori3 &_vertex2,
                           const Vectori3 &_vertex3) const {
    int x = max(_vertex1.coord.x, max(_vertex2.coord.x, _vertex3.coord.x));
    int y = max(_vertex1.coord.y, max(_vertex2.coord.y, _vertex3.coord.y));
    int z = max(_vertex1.coord.z, max(_vertex2.coord.z, _vertex3.coord.z));
    return Vectori3(x, y, z);
}

Vectord3 Geometry::get_max(const Vectord3 &_vertex1, const Vectord3 &_vertex2,
                           const Vectord3 &_vertex3) const {
    double x = max(_vertex1.coord.x, max(_vertex2.coord.x, _vertex3.coord.x));
    double y = max(_vertex1.coord.y, max(_vertex2.coord.y, _vertex3.coord.y));
    double z = max(_vertex1.coord.z, max(_vertex2.coord.z, _vertex3.coord.z));
    return Vectord3(x, y, z);
}

Vectord3 Geometry::normal(const Vectord3 &_vertex1, const Vectord3 &_vertex2,
                          const Vectord3 &_vertex3) const {
    Vectord3 edge1 = (_vertex3 - _vertex1);
    Vectord3 edge2 = (_vertex2 - _vertex1);
    return edge1 ^ edge2;
}

double Geometry::get_z(const Vectord3 &_vertex1, const Vectord3 &_vertex2,
                       const Vectord3 &_vertex3, size_t _x, size_t _y) const {
    Vectord3 abc = normal(_vertex1, _vertex2, _vertex3);
    double   d =
        0 - (abc.coord.x * _vertex1.coord.x + abc.coord.y * _vertex1.coord.y +
             abc.coord.z * _vertex1.coord.z);
    double res = (abc.coord.x * _x + abc.coord.y * _y + d) / abc.coord.z;
    // cout << "z: " << res << endl;
    return res;
}

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

void Geometry::line(Vectori2 _v0, Vectori2 _v1, const TGAColor &_color) const {
    line(_v0.coord.x, _v0.coord.y, _v1.coord.x, _v1.coord.y, _color);
    return;
}

void Geometry::line(double _x0, double _y0, double _z0, double _x1, double _y1,
                    double _z1, double *_zbuffer,
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
    int    de  = 0;
    double dy2 = (_y1 - _y0) * 2;
    double dx2 = (_x1 - _x0) * 2;
    double y   = _y0;
    double z   = _z0;
    double dz  = (_z1 - _z0) / (_x1 - _x0);
    size_t idx = 0;
    for (double x = _x0; x <= _x1; x++) {
        idx = (size_t)(x + y * image.get_height());
        if (steep == true) {
            if (_zbuffer[idx] < z) {
                image.set(y, image.get_height() - x, _color);
                _zbuffer[idx] = z;
            }
        }
        else {
            if (_zbuffer[idx] < z) {
                image.set(x, image.get_height() - y, _color);
                _zbuffer[idx] = z;
            }
        }
        de += abs(dy2);
        if (de > 1) {
            y += (_y1 > _y0 ? 1 : -1);
            de -= dx2;
        }
        z = _z0 + dz * (x - _x0);
    }
    return;
}

void Geometry::line(Vectord3 _v0, Vectord3 _v1, double *_zbuffer,
                    const TGAColor &_color) const {
    line(_v0.coord.x, _v0.coord.y, _v0.coord.z, _v1.coord.x, _v1.coord.y,
         _v1.coord.z, _zbuffer, _color);
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

Vectord3 barycentric(Vectord3 A, Vectord3 B, Vectord3 C, Vectord3 P) {
    Vectord3 s[2];
    for (int i = 2; i--;) {
        s[i][0] = C[i] - A[i];
        s[i][1] = B[i] - A[i];
        s[i][2] = A[i] - P[i];
    }
    Vectord3 u = s[0] ^ s[1];
    if (std::abs(u[2]) > 1e-2) // dont forget that u[2] is integer. If it is
                               // zero then triangle ABC is degenerate
        return Vectord3(1. - (u.coord.x + u.coord.y) / u.coord.z,
                        u.coord.y / u.coord.z, u.coord.x / u.coord.z);
    return Vectord3(-1, 1, 1); // in this case generate negative coordinates, it
                               // will be thrown away by the rasterizator
}

// #include "limits"
void Geometry::triangle(const Vectord3 &_vertex1, const Vectord3 &_vertex2,
                        const Vectord3 &_vertex3, double *_zbuffer,
                        const TGAColor &_color) const {
    Vectord3 min = get_min(_vertex1, _vertex2, _vertex3);
    Vectord3 max = get_max(_vertex1, _vertex2, _vertex3);
    Vectord3 p;
    size_t   idx = 0;
    double   z   = 0;
    for (p.coord.x = min.coord.x; p.coord.x <= max.coord.x; p.coord.x++) {
        for (p.coord.y = min.coord.y; p.coord.y <= max.coord.y; p.coord.y++) {
            if (is_barycentric(_vertex1, _vertex2, _vertex3, p) == true) {
                idx = (size_t)(p.coord.x + p.coord.y * image.get_height());
                z   = get_z(_vertex1, _vertex2, _vertex3, p.coord.x, p.coord.y);
                if (_zbuffer[idx] < z) {
                    _zbuffer[idx] = z;
                    image.set(p.coord.x, image.get_height() - p.coord.y,
                              _color);
                }
            }
        }
    }
    // Vectord2 bboxmin(std::numeric_limits<double>::max(),
    //                  std::numeric_limits<double>::max());
    // Vectord2 bboxmax(-std::numeric_limits<double>::max(),
    //                  -std::numeric_limits<double>::max());
    // Vectord3 pts[3];
    // pts[0] = _vertex1;
    // pts[1] = _vertex2;
    // pts[2] = _vertex3;
    // Vectord2 clamp(image.get_width() - 1, image.get_height() - 1);
    // for (int i = 0; i < 3; i++) {
    //     for (size_t j = 0; j < 2; j++) {
    //         bboxmin[j] = std::max(0., std::min(bboxmin[j], pts[i][j]));
    //         bboxmax[j] = std::min(clamp[j], std::max(bboxmax[j], pts[i][j]));
    //     }
    // }
    // Vectord3 P;
    // for (P.coord.x = bboxmin.coord.x; P.coord.x <= bboxmax.coord.x;
    //      P.coord.x++) {
    //     for (P.coord.y = bboxmin.coord.y; P.coord.y <= bboxmax.coord.y;
    //          P.coord.y++) {
    //         Vectord3 bc_screen = barycentric(pts[0], pts[1], pts[2], P);
    //         if (bc_screen.coord.x < 0 || bc_screen.coord.y < 0 ||
    //             bc_screen.coord.z < 0)
    //             continue;
    //         P.coord.z = 0;
    //         for (int i = 0; i < 3; i++)
    //             P.coord.z += pts[i][2] * bc_screen[i];
    //         if (_zbuffer[int(P.coord.x + P.coord.y * image.get_height())] <
    //             P.coord.z) {
    //             _zbuffer[int(P.coord.x + P.coord.y * image.get_height())] =
    //                 P.coord.z;
    //             image.set(P.coord.x,image.get_height() - P.coord.y, _color);
    //         }
    //     }
    // }
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
