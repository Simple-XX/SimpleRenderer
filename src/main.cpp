
// This file is a part of SimpleXX/SimpleRenderer (https://github.com/SimpleXX/SimpleRenderer).
//
// main.cpp for SimpleXX/SimpleRenderer.

#include "tga.h"
#include "vector.hpp"
#include "iostream"
#include "test.h"

using namespace std;
// #include "model.h"

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red   = TGAColor(255, 0,   0,   255);
// Model * model = NULL;
const int width  = 1920;
const int height = 1080;

// void line(int _x0, int _y0, int _x1, int _y1, TGAImage &_image, const TGAColor &_color) {
//     bool steep = false;
//     if(std::abs(_x0 - _x1) < std::abs(_y0 - _y1) ) {
//         std::swap(_x0, _y0);
//         std::swap(_x1, _y1);
//         steep = true;
//     }
//     if(_x0 > _x1) {
//         std::swap(_x0, _x1);
//         std::swap(_y0, _y1);
//     }
//     int dx = _x1 - _x0;
//     int dy = _y1 - _y0;
//     int derror2 = std::abs(dy) * 2;
//     int error2 = 0;
//     int y = _y0;
//     for(int x = _x0 ; x <= _x1 ; x++) {
//         if(steep) {
//             _image.set(y, x, _color);
//         } else {
//             _image.set(x, y, _color);
//         }
//         error2 += derror2;
//         if(error2 > dx) {
//             y += (_y1 > _y0 ? 1 : -1);
//             error2 -= dx * 2;
//         }
//     }
// }

int main(int argc, char * * argv) {
    Test test;
    test.test_vector();
    // if(2 == argc) {
    //     model = new Model(argv[1]);
    // } else {
    //     model = new Model("obj/african_head.obj");
    // }
    // TGAImage image;
    //
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
