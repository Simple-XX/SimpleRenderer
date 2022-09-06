
// This file is a part of Simple-XX/SimpleRenderer
// (https://github.com/Simple-XX/SimpleRenderer).
//
// geometry.cpp for Simple-XX/SimpleRenderer.

#include "iostream"
#include "vector"
#include "geometry.h"

using namespace std;

// 推导过程见
bool Geometry::is_barycentric(const vector2i_t &_vertex1,
                              const vector2i_t &_vertex2,
                              const vector2i_t &_vertex3,
                              const vector2i_t &_p) const {
    // 边向量
    vector2i_t edge1 = (_vertex3 - _vertex1);
    vector2i_t edge2 = (_vertex2 - _vertex1);
    // 到点 P 的向量
    vector2i_t edge3 = (_p - _vertex1);
    float      v1v1  = edge1 * edge1;
    float      v1v2  = edge1 * edge2;
    float      v1v3  = edge1 * edge3;
    float      v2v2  = edge2 * edge2;
    float      v2v3  = edge2 * edge3;

    float deno = (v1v1 * v2v2) - (v1v2 * v1v2);
    float u    = ((v2v2 * v1v3) - (v1v2 * v2v3)) / deno;
    if (u < 0. || u > 1.) {
        return false;
    }
    float v = ((v1v1 * v2v3) - (v1v2 * v1v3)) / deno;
    if (v < 0. || v > 1.) {
        return false;
    }
    if (u + v >= 1.) {
        return false;
    }
    return true;
}

bool Geometry::is_barycentric(const vector3f_t &_vertex1,
                              const vector3f_t &_vertex2,
                              const vector3f_t &_vertex3,
                              const vector3f_t &_p) const {
    // 边向量
    vector3f_t edge1 = (_vertex3 - _vertex1);
    vector3f_t edge2 = (_vertex2 - _vertex1);
    // 到点 P 的向量
    vector3f_t edge3 = (_p - _vertex1);
    float      v1v1  = edge1 * edge1;
    float      v1v2  = edge1 * edge2;
    float      v1v3  = edge1 * edge3;
    float      v2v2  = edge2 * edge2;
    float      v2v3  = edge2 * edge3;

    float deno = (v1v1 * v2v2) - (v1v2 * v1v2);
    float u    = ((v2v2 * v1v3) - (v1v2 * v2v3)) / deno;
    if (u < 0. || u > 1.) {
        return false;
    }
    float v = ((v1v1 * v2v3) - (v1v2 * v1v3)) / deno;
    if (v < 0. || v > 1.) {
        return false;
    }
    if (u + v >= 1.) {
        return false;
    }
    return true;
}

vector2i_t Geometry::get_min(const vector2i_t &_vertex1,
                             const vector2i_t &_vertex2,
                             const vector2i_t &_vertex3) const {
    size_t x = min(_vertex1.x, min(_vertex2.x, _vertex3.x));
    size_t y = min(_vertex1.y, min(_vertex2.y, _vertex3.y));
    return vector2i_t(x, y);
}

vector2i_t Geometry::get_max(const vector2i_t &_vertex1,
                             const vector2i_t &_vertex2,
                             const vector2i_t &_vertex3) const {
    size_t x = max(_vertex1.x, max(_vertex2.x, _vertex3.x));
    size_t y = max(_vertex1.y, max(_vertex2.y, _vertex3.y));
    return vector2i_t(x, y);
}

vector3f_t Geometry::get_min(const vector3f_t &_vertex1,
                             const vector3f_t &_vertex2,
                             const vector3f_t &_vertex3) const {
    size_t x = min(_vertex1.x, min(_vertex2.x, _vertex3.x));
    size_t y = min(_vertex1.y, min(_vertex2.y, _vertex3.y));
    size_t z = min(_vertex1.z, min(_vertex2.z, _vertex3.z));
    return vector3f_t(x, y, z);
}

vector3f_t Geometry::get_max(const vector3f_t &_vertex1,
                             const vector3f_t &_vertex2,
                             const vector3f_t &_vertex3) const {
    float x = max(_vertex1.x, max(_vertex2.x, _vertex3.x));
    float y = max(_vertex1.y, max(_vertex2.y, _vertex3.y));
    float z = max(_vertex1.z, max(_vertex2.z, _vertex3.z));
    return vector3f_t(x, y, z);
}

vector3f_t Geometry::normal(const vector3f_t &_vertex1,
                            const vector3f_t &_vertex2,
                            const vector3f_t &_vertex3) const {
    vector3f_t edge1 = (_vertex3 - _vertex1);
    vector3f_t edge2 = (_vertex2 - _vertex1);
    return edge1 ^ edge2;
}

float Geometry::get_z(const vector3f_t &_vertex1, const vector3f_t &_vertex2,
                      const vector3f_t &_vertex3, size_t _x, size_t _y) const {
    vector3f_t abc = normal(_vertex1, _vertex2, _vertex3);
    float d   = -(abc.x * _vertex1.x + abc.y * _vertex1.y + abc.z * _vertex1.z);
    float res = -(abc.x * _x + abc.y * _y + d) / abc.z;
    return res;
}

Geometry::Geometry(TGAImage &_image, const string &_filename)
    : image(_image), filename(_filename) {
    for (size_t w = 0; w < image.get_width(); w++) {
        for (size_t h = 0; h < image.get_height(); h++) {
            image.set(w, h, color_bg);
        }
    }
    return;
}

Geometry::~Geometry(void) {
    return;
}

size_t Geometry::get_width(void) const {
    return image.get_width();
}

size_t Geometry::get_height(void) const {
    return image.get_height();
}

bool Geometry::save(string _filename) const {
    return image.write_tga_file(_filename);
}

// 在给定 _image 上按照 _x0, _y0, _x1, _y1 给出的坐标绘制直线，颜色由 _color
// 指定
// [(_x0, _y0), (_x1, _y1)) 左上角为原点
void Geometry::line(size_t _x0, size_t _y0, size_t _x1, size_t _y1,
                    const TGAColor &_color) const {
    // 斜率大于 1
    bool steep = false;
    if (abs((int)(_x0 - _x1)) < abs((int)(_y0 - _y1))) {
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
    int    dy2 = (_y1 - _y0) << 1;
    int    dx2 = (_x1 - _x0) << 1;
    size_t y   = _y0;
    for (size_t x = _x0; x <= _x1; x++) {
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

void Geometry::line(vector2i_t _v0, vector2i_t _v1,
                    const TGAColor &_color) const {
    line(_v0.x, _v0.y, _v1.x, _v1.y, _color);
    return;
}

void Geometry::line(vector2i_t *_vertexes, const TGAColor &_color) const {
    line(_vertexes[0].x, _vertexes[0].y, _vertexes[1].x, _vertexes[1].y,
         _color);
    return;
}

void Geometry::line(float _x0, float _y0, float _z0, float _x1, float _y1,
                    float _z1, float *_zbuffer, const TGAColor &_color) {
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
    float  dy2 = (_y1 - _y0) * 2;
    float  dx2 = (_x1 - _x0) * 2;
    float  y   = _y0;
    float  z   = _z0;
    float  dz  = (_z1 - _z0) / (_x1 - _x0);
    size_t idx = 0;
    for (float x = _x0; x <= _x1; x++) {
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

void Geometry::line(vector3f_t _v0, vector3f_t _v1, float *_zbuffer,
                    const TGAColor &_color) {
    line(_v0.x, _v0.y, _v0.z, _v1.x, _v1.y, _v1.z, _zbuffer, _color);
    return;
}

void Geometry::line(vector3f_t *_vertexes, float *_zbuffer,
                    const TGAColor &_color) {
    line(_vertexes[0].x, _vertexes[0].y, _vertexes[0].z, _vertexes[1].x,
         _vertexes[1].y, _vertexes[1].z, _zbuffer, _color);
    return;
}

// 绘制三角形，_vertex1 _vertex2 _vertex3 为三个顶点
void Geometry::triangle(const vector2i_t &_vertex1, const vector2i_t &_vertex2,
                        const vector2i_t &_vertex3,
                        const TGAColor   &_color) const {
    // 遍历平行四边形，如果在三角形内则填充
    vector2i_t min = get_min(_vertex1, _vertex2, _vertex3);
    vector2i_t max = get_max(_vertex1, _vertex2, _vertex3);
    vector2i_t p;
    for (p.x = min.x; p.x <= max.x; p.x++) {
        for (p.y = min.y; p.y <= max.y; p.y++) {
            if (is_barycentric(_vertex1, _vertex2, _vertex3, p) == true) {
                image.set(p.x, image.get_height() - p.y, _color);
            }
        }
    }
    return;
}

void Geometry::triangle(const vector2i_t *_vertexes,
                        const TGAColor   &_color) const {
    triangle(_vertexes[0], _vertexes[1], _vertexes[2], _color);
    return;
}

void Geometry::triangle(const vector3f_t &_vertex1, const vector3f_t &_vertex2,
                        const vector3f_t &_vertex3, float *_zbuffer,
                        const TGAColor &_color) const {
    vector3f_t min = get_min(_vertex1, _vertex2, _vertex3);
    vector3f_t max = get_max(_vertex1, _vertex2, _vertex3);
    vector3f_t p;
    size_t     idx = 0;
    float      z   = 0;
    for (p.x = min.x; p.x <= max.x; p.x++) {
        for (p.y = min.y; p.y <= max.y; p.y++) {
            if (is_barycentric(_vertex1, _vertex2, _vertex3, p) == true) {
                idx = (size_t)(p.x + p.y * image.get_height());
                z   = get_z(_vertex1, _vertex2, _vertex3, p.x, p.y);
                if (_zbuffer[idx] < z) {
                    _zbuffer[idx] = z;
                    image.set(p.x, image.get_height() - p.y, _color);
                }
            }
        }
    }
    return;
}

void Geometry::triangle(const vector3f_t *_vertexes, float *_zbuffer,
                        const TGAColor &_color) {
    triangle(_vertexes[0], _vertexes[1], _vertexes[2], _zbuffer, _color);
    return;
}

void Geometry::circle(size_t _x0, size_t _y0, float _r, TGAColor _color) const {
    assert(_r > 0);
    size_t x = 0;
    size_t y = _r;

    // 起点(0,R)
    // 下一点中点(1,R-0.5)
    // d=1*1+(R-0.5)*(R-0.5)-R*R=1.25-R
    // d只参与整数运算，所以小数部分可省略
    int d = 1 - _r;

    while (y > x) // y>x即第一象限的第1区八分圆
    {
        image.set(x + _x0, image.get_height() - (y + _y0), _color);
        image.set(y + _x0, image.get_height() - (x + _y0), _color);
        image.set(-x + _x0, image.get_height() - (y + _y0), _color);
        image.set(-y + _x0, image.get_height() - (x + _y0), _color);

        image.set(-x + _x0, image.get_height() - (-y + _y0), _color);
        image.set(-y + _x0, image.get_height() - (-x + _y0), _color);
        image.set(x + _x0, image.get_height() - (-y + _y0), _color);
        image.set(y + _x0, image.get_height() - (-x + _y0), _color);
        if (d < 0) {
            d = d + 2 * x + 3;
        }
        else {
            d = d + 2 * (x - y) + 5;
            y--;
        }
        x++;
    }
    return;
}

void Geometry::circle(size_t _x0, size_t _y0, float _r, float *_zbuffer,
                      const TGAColor &_color) const {
    assert(_r > 0);
    size_t x = 0;
    size_t y = _r;

    // 起点(0,R)
    // 下一点中点(1,R-0.5)
    // d=1*1+(R-0.5)*(R-0.5)-R*R=1.25-R
    // d 只参与整数运算，所以小数部分可省略
    int d = 1 - _r;

    // y > x 即第一象限的第1区八分圆
    while (y > x) {
        image.set(x + _x0, image.get_height() - (y + _y0), _color);
        image.set(y + _x0, image.get_height() - (x + _y0), _color);
        image.set(-x + _x0, image.get_height() - (y + _y0), _color);
        image.set(-y + _x0, image.get_height() - (x + _y0), _color);

        image.set(-x + _x0, image.get_height() - (-y + _y0), _color);
        image.set(-y + _x0, image.get_height() - (-x + _y0), _color);
        image.set(x + _x0, image.get_height() - (-y + _y0), _color);
        image.set(y + _x0, image.get_height() - (-x + _y0), _color);
        if (d < 0) {
            d = d + 2 * x + 3;
        }
        else {
            d = d + 2 * (x - y) + 5;
            y--;
        }
        x++;
    }
    _zbuffer = _zbuffer;
    return;
}
