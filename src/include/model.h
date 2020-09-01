
// This file is a part of SimpleXX/SimpleRenderer (https://github.com/SimpleXX/SimpleRenderer).
// Based on https://github.com/ssloy/tinyrenderer
// model.h for SimpleXX/SimpleRenderer.

#ifndef __MODEL_H__
#define __MODEL_H__

#include "vector"
#include "string"
#include "vector.hpp"
#include "tga.h"
#include "2d.h"

class Model {
private:
    std::vector<Vectord3> verts;
    std::vector<std::vector<int> > faces;
    int nverts(void);
    int nfaces(void);
    Vectord3 vert(int i);
    std::vector<int> face(int _idx);
    TwoD &twod;

public:
    Model(std::string _filename, TwoD &_twod);
    ~Model(void);
    void to_tga_line(std::string _filename = "model2tga_line.tga");
    void to_tga_fill(std::string _filename = "model2tga_fill.tga");
};

#endif /* __MODEL_H__ */
