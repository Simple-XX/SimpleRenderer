
// This file is a part of SimpleXX/SimpleRenderer (https://github.com/SimpleXX/SimpleRenderer).
//
// main.cpp for SimpleXX/SimpleRenderer.

#include "iostream"
#include "vector.hpp"
#include "2d.h"
#include "tga.h"
#include "model.h"
#include "test.h"

using namespace std;

TGAColor white = TGAColor(255, 255, 255, 255);
TGAColor red   = TGAColor(255, 0,   0,   255);
const int width  = 1920;
const int height = 1080;

int main(int argc, char * * argv) {
    Test test;
    test.test_vector();
    test.test_2d();
    TGAImage image(width, height, TGAImage::RGBA);
    TwoD draw(image);
    string filename;
    if(2 == argc) {
        filename = argv[1];
    } else {
        filename = "obj/african_head.obj";
    }
    Model model(filename, draw);
    // model.to_tga_line();
    model.to_tga_fill();
    return 0;
}
