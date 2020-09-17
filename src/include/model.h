
// This file is a part of SimpleXX/SimpleRenderer
// (https://github.com/SimpleXX/SimpleRenderer).
// Based on https://github.com/ssloy/tinyrenderer
// model.h for SimpleXX/SimpleRenderer.

#ifndef __MODEL_H__
#define __MODEL_H__

#include "vector"
#include "string"
#include "vector.hpp"

class Model {
private:
    std::vector<Vectord3>         verts;
    std::vector<std::vector<int>> faces;

public:
    Model(const std::string &_filename);
    ~Model(void);
    int              nverts(void) const;
    int              nfaces(void) const;
    Vectord3         vert(int i) const;
    std::vector<int> face(int _idx) const;
};

#endif /* __MODEL_H__ */
