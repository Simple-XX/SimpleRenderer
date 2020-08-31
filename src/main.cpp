
// This file is a part of SimpleXX/SimpleRenderer (https://github.com/SimpleXX/SimpleRenderer).
//
// main.cpp for SimpleXX/SimpleRenderer.

#include "iostream"
#include "vector.hpp"
#include "tga.h"
#include "test.h"

using namespace std;
// #include "model.h"

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red   = TGAColor(255, 0,   0,   255);
// Model * model = NULL;
const int width  = 1920;
const int height = 1080;

int main(int argc, char * * argv) {
    Test test;
    test.test_vector();
    test.test_line();
    // if(2 == argc) {
    //     model = new Model(argv[1]);
    // } else {
    //     model = new Model("obj/african_head.obj");
    // }
    // TGAImage image(width, height, TGAImage::RGB);

    // image.write_tga_file("output.tga");
    // for(int i = 0 ; i < model->nfaces() ; i++) {
    //     std::vector<int> face = model->face(i);
    //     for(int j = 0 ; j < 3 ; j++) {
    //         Vec3f v0 = model->vert(face[j]);
    //         Vec3f v1 = model->vert(face[(j + 1) % 3]);
    //         int _x0 = (v0.x + 1.) * width / 2.;
    //         int _y0 = (v0.y + 1.) * height / 2.;
    //         int _x1 = (v1.x + 1.) * width / 2.;
    //         int _y1 = (v1.y + 1.) * height / 2.;
    //         line(800 - _x0, 800 - _y0, 800 - _x1, 800 - _y1, image, white);
    //     }
    // }
    // delete model;
    return 0;
}
