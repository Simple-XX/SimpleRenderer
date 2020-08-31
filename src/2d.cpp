
// This file is a part of SimpleXX/SimpleRenderer (https://github.com/SimpleXX/SimpleRenderer).
//
// 2d.cpp for SimpleXX/SimpleRenderer.

#include "iostream"
#include "tga.h"

using namespace std;

void line(int _x0, int _y0, int _x1, int _y1, TGAImage &_image, const TGAColor &_color) {
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
            _image.set(y, _image.get_height() - x, _color);
        }
        else {
            cout << "x: " << x << "y: " << y << endl;
            _image.set(x, _image.get_height() - y, _color);
        }
        de += abs(dy2);
        if(de > 1) {
            y += (_y1 > _y0 ? 1 : -1);
            de -= dx2;
        }
    }
    return;
}
