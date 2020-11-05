
// This file is a part of SimpleXX/SimpleRenderer
// (https://github.com/SimpleXX/SimpleRenderer).
//
// renderer.cpp for SimpleXX/SimpleRenderer.

#include "renderer.h"
#include "iostream"
#include "limits"

using namespace std;

Renderer::Renderer(Geometry &_painter, const Model &_model)
    : model(_model), painter(_painter) {
    width   = painter.get_width();
    height  = painter.get_height();
    zbuffer = new double[width * height];
    for (size_t i = 0; i < width * height; i++) {
        zbuffer[i] = -std::numeric_limits<double>::max();
    }
    return;
}

Renderer::~Renderer(void) {
    delete zbuffer;
    return;
}

bool Renderer::render(void) const {
    line();
    // fill();
    return true;
}

bool Renderer::render(void) {
    cout << "non-const" << endl;
    line_zbuffer();
    // fill_zbuffer();
    return true;
}

bool Renderer::line(void) const {
    for (int i = 0; i < model.nfaces(); i++) {
        vector<int> face = model.face(i);
        for (int j = 0; j < 3; j++) {
            Vectord3 v0 = model.vert(face.at(j));
            Vectord3 v1 = model.vert(face.at((j + 1) % 3));
            int      x0 = (v0.coord.x + 1.) * width / 3.;
            int      y0 = height - (v0.coord.y + 1.) * height / 2.;
            int      x1 = (v1.coord.x + 1.) * width / 3.;
            int      y1 = height - (v1.coord.y + 1.) * height / 2.;
            painter.line(x0, y0, x1, y1, white);
        }
    }
    return true;
}

bool Renderer::line_zbuffer(void) {
    for (int i = 0; i < model.nfaces(); i++) {
        vector<int> face = model.face(i);
        for (int j = 0; j < 3; j++) {
            Vectord3 v0 = model.vert(face.at(j));
            Vectord3 v1 = model.vert(face.at((j + 1) % 3));
            int      x0 = (v0.coord.x + 1.) * width / 3.;
            int      y0 = height - (v0.coord.y + 1.) * height / 2.;
            int      x1 = (v1.coord.x + 1.) * width / 3.;
            int      y1 = height - (v1.coord.y + 1.) * height / 2.;
            double   z0 = v0.coord.z;
            double   z1 = v1.coord.z;
            painter.line(x0, y0, z0, x1, y1, z1, zbuffer, white);
        }
    }
    return true;
}

bool Renderer::fill(void) const {
    for (int i = 0; i < model.nfaces(); i++) {
        std::vector<int> face = model.face(i);
        Vectori2         screen_coords[3];
        Vectord3         world_coords[3];
        for (int j = 0; j < 3; j++) {
            Vectord3 v = model.vert(face[j]);
            screen_coords[j] =
                Vectori2((v.coord.x + 1.) * width / 3.,
                         height - (v.coord.y + 1.) * height / 2.);
            world_coords[j] = v;
        }
        Vectord3 n = (world_coords[2] - world_coords[0]) ^
                     (world_coords[1] - world_coords[0]);
        double intensity = light_dir * n.unit();
        if (intensity > 0) {
            painter.triangle(screen_coords[0], screen_coords[1],
                             screen_coords[2],
                             TGAColor(intensity * 255, intensity * 255,
                                      intensity * 255, 255));
        }
    }
    return true;
}

bool Renderer::fill_zbuffer(void) {
    for (int i = 0; i < model.nfaces(); i++) {
        std::vector<int> face = model.face(i);
        Vectord3         screen_coords[3];
        Vectord3         world_coords[3];
        for (int j = 0; j < 3; j++) {
            std::cerr << "1" << std::endl;
            Vectord3 v = model.vert(face[j]);
            std::cerr << "2" << std::endl;
            screen_coords[j] = Vectord3(
                (v.coord.x + 1.) * width / 3. + 0.5,
                height - (v.coord.y + 1.) * height / 2. + 0.5, v.coord.z);
            world_coords[j] = v;
        }
        Vectord3 n = (world_coords[2] - world_coords[0]) ^
                     (world_coords[1] - world_coords[0]);
        double intensity = light_dir * n.unit();
        if (intensity > 0) {
            painter.triangle(screen_coords[0], screen_coords[1],
                             screen_coords[2], zbuffer,
                             TGAColor(intensity * 255, intensity * 255,
                                      intensity * 255, 255));
        }
    }
    return true;
}

bool Renderer::save(const std::string &_filename) const {
    return painter.save(_filename);
}

bool Renderer::set_light(const Vectord3 &_light) {
    light_dir = _light;
    return true;
}

bool Renderer::set_size(size_t _w, size_t _h) {
    width  = _w;
    height = _h;
    return true;
}

// 读取参数
Vectord3 Renderer::get_light(void) const {
    return light_dir;
}
Vectori2 Renderer::get_size(void) const {
    return Vectori2(width, height);
}
