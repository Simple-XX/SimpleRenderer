
// This file is a part of SimpleXX/SimpleRenderer (https://github.com/SimpleXX/SimpleRenderer).
//
// 2d.h for SimpleXX/SimpleRenderer.

#ifndef __2D_H__
#define __2D_H__

#include "tga.h"
#include "string"

class TwoD {
private:
    TGAImage &image;
    // white
    TGAColor color = TGAColor(255, 255, 255, 255);
    // black
    TGAColor color_bg = TGAColor(0, 0, 0, 255);
    std::string filename = "output.tga";

public:
    TwoD(TGAImage &_image, std::string _filename = "output.tga");
    ~TwoD(void);
    void line(int _x0, int _y0, int _x1, int _y1) const;
    void set_bg(TGAColor &_color_bg);
    void set_color(TGAColor & _color);
    TGAColor get_color(void) const;
    int get_width(void) const;
    int get_height(void) const;
    bool save(std::string _filename = "output.tga") const;
};

#endif /* __2D_H__ */
