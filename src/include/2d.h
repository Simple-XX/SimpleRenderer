
// This file is a part of SimpleXX/SimpleRenderer (https://github.com/SimpleXX/SimpleRenderer).
//
// 2d.h for SimpleXX/SimpleRenderer.

#ifndef __2D_H__
#define __2D_H__

#include "image.h"
#include "string"
#include "vector.hpp"

class TwoD {
private:
    TGAImage &image;
    std::string filename = "output.tga";
    // 背景色默认黑色
    const TGAColor color_bg = TGAColor(0, 0, 0, 255);
    // 归一化
    double normalize(double _val) const;
    // 判断 _p 是否为三角形重心
    bool is_barycentric(const Vectori2 &_vertex1, const Vectori2 &_vertex2, const Vectori2 &_vertex3, const Vectori2 &_p) const;
    // 获取最小 (x,y)
    Vectori2 get_min(const Vectori2 &_vertex1, const Vectori2 &_vertex2, const Vectori2 &_vertex3) const;
    // 获取最大 (x,y)
    Vectori2 get_max(const Vectori2 &_vertex1, const Vectori2 &_vertex2, const Vectori2 &_vertex3) const;

public:
    TwoD(TGAImage &_image, const std::string &_filename = "output.tga");
    ~TwoD(void);
    void line(int _x0, int _y0, int _x1, int _y1, const TGAColor &_color) const;
    void triangle(const Vectori2 &_vertex1, const Vectori2 &_vertex2, const Vectori2 &_vertex3, const TGAColor &_color) const;
    void triangle(const Vectori2 * _vertexes, const TGAColor &_color) const;
    void circle(const int _x0, const int _y0, const double _r) const;
    int get_width(void) const;
    int get_height(void) const;
    bool save(std::string _filename = "output.tga") const;
};

#endif /* __2D_H__ */
