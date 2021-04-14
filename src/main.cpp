
// This file is a part of SimpleXX/SimpleRenderer
// (https://github.com/SimpleXX/SimpleRenderer).
//
// main.cpp for SimpleXX/SimpleRenderer.

#include "common.h"
#include "geometry.h"
#include "image.h"
#include "iostream"
#include "model.h"
#include "renderer.h"
#include "test.h"
#include "vector.hpp"

using namespace std;

Test test;

int main(int argc, char **argv) {
    string filename;
    if (2 == argc) {
        filename = argv[1];
    }
    else {
        filename = "../../src/obj/african_head.obj";
    }
    Model    model(filename);
    TGAImage image(width, height, TGAImage::RGBA);
    Geometry painter(image);
    Renderer render = Renderer(painter, model);
    render.render();
    render.save();
    // test.test_vector();
    // test.test_matrix();
    return 0;
}
