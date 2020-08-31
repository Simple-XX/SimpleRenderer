
// This file is a part of SimpleXX/SimpleRenderer (https://github.com/SimpleXX/SimpleRenderer).
//
// 2d.h for SimpleXX/SimpleRenderer.

#ifndef __2D_H__
#define __2D_H__

#include "tga.h"

// 在给定 _image 上按照 _x0, _y0, _x1, _y1 给出的坐标绘制直线，颜色由 _color 指定
// [(_x0, _y0), (_x1, _y1)) 左上角为原点
void line(int _x0, int _y0, int _x1, int _y1, TGAImage &_image, const TGAColor &_color);

#endif /* __2D_H__ */
