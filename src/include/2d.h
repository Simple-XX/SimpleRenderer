
// This file is a part of SimpleXX/SimpleRenderer (https://github.com/SimpleXX/SimpleRenderer).
//
// 2d.h for SimpleXX/SimpleRenderer.

#ifndef __2D_H__
#define __2D_H__

#include "tga.h"
#include "string"
#include "vector.hpp"

class TwoD {
private:
    TGAImage &image;
    // white
    TGAColor color_edge = TGAColor(255, 255, 255, 255);
    // black
    TGAColor color_bg = TGAColor(0, 0, 0, 255);
    // red
    TGAColor color_fill = TGAColor(255, 0, 0, 255);
    std::string filename = "output.tga";
    // 归一化
    double normalize(double _val) const;
    // 判断 _p 是否为三角形重心
    bool is_barycentric(Vectori2 _vertex1, Vectori2 _vertex2, Vectori2 _vertex3, Vectori2 _p) const;
    // 获取最小 (x,y)
    Vectori2 get_min(Vectori2 _vertex1, Vectori2 _vertex2, Vectori2 _vertex3) const;
    // 获取最大 (x,y)
    Vectori2 get_max(Vectori2 _vertex1, Vectori2 _vertex2, Vectori2 _vertex3) const;

public:
    TwoD(TGAImage &_image, std::string _filename = "output.tga");
    ~TwoD(void);
    void line(int _x0, int _y0, int _x1, int _y1) const;
    void triangle(Vectori2 _vertex1, Vectori2 _vertex2, Vectori2 _vertex3) const;
    void triangle(Vectori2 * _vertexes) const;
    void circle(int _x0, int _y0, double _r) const;
    void set_edge(TGAColor _color_edge = TGAColor(255, 255, 255, 255) );
    void set_fill(TGAColor _color_fill = TGAColor(255, 0, 0, 255) );
    void set_bg(TGAColor _color_bg = TGAColor(0, 0, 0, 255) );
    TGAColor get_edge(void) const;
    TGAColor get_fill(void) const;
    TGAColor get_bg(void) const;
    int get_width(void) const;
    int get_height(void) const;
    bool save(std::string _filename = "output.tga") const;
};

#endif /* __2D_H__ */
