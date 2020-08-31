
// This file is a part of SimpleXX/SimpleRenderer (https://github.com/SimpleXX/SimpleRenderer).
//
// 2d.cpp for SimpleXX/SimpleRenderer.

#include "iostream"
#include "2d.h"

using namespace std;

TwoD::TwoD(TGAImage & _image, std::string _filename) : image(_image), filename(_filename) {
    return;
}

TwoD::~TwoD(void) {
    return;
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
            cout << "x: " << y << "y: " << x << endl;
            image.set(y, image.get_height() - x, color);
        }
        else {
            cout << "x: " << x << "y: " << y << endl;
            image.set(x, image.get_height() - y, color);
        }
        de += abs(dy2);
        if(de > 1) {
            y += (_y1 > _y0 ? 1 : -1);
            de -= dx2;
        }
    }
    return;
}

void TwoD::set_color(TGAColor & _color) {
    color = _color;
    return;
}

TGAColor TwoD::get_color(void) const {
    return color;
}

bool TwoD::save(string _filename) const {
    return image.write_tga_file(_filename);
}
