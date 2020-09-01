
// This file is a part of SimpleXX/SimpleRenderer (https://github.com/SimpleXX/SimpleRenderer).
//
// 2d.cpp for SimpleXX/SimpleRenderer.

#include "iostream"
#include "2d.h"
#include "vector"

using namespace std;

TwoD::TwoD(TGAImage & _image, std::string _filename) : image(_image), filename(_filename) {
    for(auto w = 0 ; w < image.get_width() ; w++) {
        for(auto h = 0 ; h < image.get_height() ; h++) {
            image.set(w, h, color_bg);
        }
    }
    return;
}

TwoD::~TwoD(void) {
    return;
}

double TwoD::normalize(double _val) const {
    return 0;
}

// 推导过程见
bool TwoD::is_barycentric(Vectori2 _vertex1, Vectori2 _vertex2, Vectori2 _vertex3, Vectori2 _p) const {
    // 边向量
    Vectori2 edge1 = (_vertex3 - _vertex1);
    Vectori2 edge2 = (_vertex2 - _vertex1);
    // 到点 P 的向量
    Vectori2 edge3 = (_p - _vertex1);
    double v1v1 = edge1 * edge1;
    double v1v2 = edge1 * edge2;
    double v1v3 = edge1 * edge3;
    double v2v2 = edge2 * edge2;
    double v2v3 = edge2 * edge3;

    double deno = (v1v1 * v2v2) - (v1v2 * v1v2);
    double u = ( (v2v2 * v1v3) - (v1v2 * v2v3) ) / deno;
    if(u < 0. || u > 1.) {
        return false;
    }
    double v = ( (v1v1 * v2v3) - (v1v2 * v1v3) ) / deno;
    if(v < 0. || v > 1.) {
        return false;
    }
    if(u + v >= 1.) {
        return false;
    }
    return true;
}

Vectori2 TwoD::get_min(Vectori2 _vertex1, Vectori2 _vertex2, Vectori2 _vertex3) const {
    int x = min(_vertex1.get_vect()[0], min(_vertex2.get_vect()[0], _vertex3.get_vect()[0]) );
    int y = min(_vertex1.get_vect()[1], min(_vertex2.get_vect()[1], _vertex3.get_vect()[1]) );
    int tmp[2] = { x, y };
    return Vectori2(tmp);
}

Vectori2 TwoD::get_max(Vectori2 _vertex1, Vectori2 _vertex2, Vectori2 _vertex3) const {
    int x = max(_vertex1.get_vect()[0], max(_vertex2.get_vect()[0], _vertex3.get_vect()[0]) );
    int y = max(_vertex1.get_vect()[1], max(_vertex2.get_vect()[1], _vertex3.get_vect()[1]) );
    int tmp[2] = { x, y };
    return Vectori2(tmp);
}

// 在给定 _image 上按照 _x0, _y0, _x1, _y1 给出的坐标绘制直线，颜色由 _color 指定
// [(_x0, _y0), (_x1, _y1)) 左上角为原点
void TwoD::line(int _x0, int _y0, int _x1, int _y1) const {
    // 斜率大于 1
    bool steep = false;
    if(abs(_x0 - _x1) < abs(_y0 - _y1) ) {
        swap(_x0, _y0);
        swap(_x1, _y1);
        steep = true;
    }
    // 终点 x 坐标在起点 左边
    if(_x0 > _x1) {
        swap(_x0, _x1);
        swap(_y0, _y1);
    }
    int de = 0;
    int dy2 = (_y1 - _y0) << 1;
    int dx2 = (_x1 - _x0) << 1;
    int y = _y0;
    for(int x = _x0 ; x <= _x1 ; x++) {
        if(steep == true) {
            image.set(y, image.get_height() - x, color_edge);
        }
        else {
            image.set(x, image.get_height() - y, color_edge);
        }
        de += abs(dy2);
        if(de > 1) {
            y += (_y1 > _y0 ? 1 : -1);
            de -= dx2;
        }
    }
    return;
}

// 绘制三角形，_vertex1 _vertex2 _vertex3 为三个顶点
void TwoD::triangle(Vectori2 _vertex1, Vectori2 _vertex2, Vectori2 _vertex3) const {
    // 遍历平行四边形，如果在三角形内则填充
    Vectori2 min = get_min(_vertex1, _vertex2, _vertex3);
    Vectori2 max = get_max(_vertex1, _vertex2, _vertex3);
    Vectori2 p;
    for(p.set_vect(min.get_vect()[0], 0) ; p.get_vect()[0] <= max.get_vect()[0] ; p.set_vect(p.get_vect()[0] + 1, 0) ) {
        for(p.set_vect(min.get_vect()[1], 1) ; p.get_vect()[1] <= max.get_vect()[1] ; p.set_vect(p.get_vect()[1] + 1, 1) ) {
            if(is_barycentric(_vertex1, _vertex2, _vertex3, p) == true) {
                image.set(p.get_vect()[0], image.get_height() - p.get_vect()[1], color_fill);
            }
        }
    }
    return;
}

void TwoD::triangle(Vectori2 * _vertexes) const {
    triangle(_vertexes[0], _vertexes[1], _vertexes[2]);
    return;
}

void TwoD::circle(int _x0, int _y0, double _r) const {
    assert(_r < 0);
    return;
}

void TwoD::set_edge(TGAColor _color_edge) {
    color_edge = _color_edge;
    return;
}

void TwoD::set_fill(TGAColor _color_fill) {
    color_fill = _color_fill;
    return;
}

void TwoD::set_bg(TGAColor _color_bg) {
    color_bg = _color_bg;
    for(auto w = 0 ; w < image.get_width() ; w++) {
        for(auto h = 0 ; h < image.get_height() ; h++) {
            image.set(w, h, color_bg);
        }
    }
    return;
}

TGAColor TwoD::get_edge(void) const {
    return color_edge;
}

TGAColor TwoD::get_fill(void) const {
    return color_fill;
}

TGAColor TwoD::get_bg(void) const {
    return color_bg;
}

int TwoD::get_width(void) const {
    return image.get_width();
}

int TwoD::get_height(void) const {
    return image.get_height();
}

bool TwoD::save(string _filename) const {
    return image.write_tga_file(_filename);
}
