
// This file is a part of SimpleXX/SimpleRenderer
// (https://github.com/SimpleXX/SimpleRenderer).
//
// main.cpp for SimpleXX/SimpleRenderer.

#include "geometry.h"
#include "image.h"
#include "iostream"
#include "model.h"
#include "renderer.h"
#include "test.h"
#include "vector.hpp"

using namespace std;

TGAColor  white  = TGAColor(255, 255, 255, 255);
TGAColor  red    = TGAColor(255, 0, 0, 255);
const int width  = 1920;
const int height = 1080;

int main(int argc, char **argv) {
    string filename;
    if (2 == argc) {
        filename = argv[1];
    }
    else {
        filename = "obj/african_head.obj";
    }
    Model    model(filename);
    TGAImage image(width, height, TGAImage::RGBA);
    Geometry painter(image);
    Renderer render = Renderer(painter, model);
    render.render();
    render.save();
    return 0;
}
