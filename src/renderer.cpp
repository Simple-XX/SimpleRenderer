
// This file is a part of Simple-XX/SimpleRenderer
// (https://github.com/Simple-XX/SimpleRenderer).
//
// renderer.cpp for Simple-XX/SimpleRenderer.

#include "iostream"
#include "limits"
#include "common.h"
#include "renderer.h"

using namespace std;

size_t Renderer::get_x(float _x) const {
    return (_x + 1.) * width / 3.;
}

size_t Renderer::get_y(float _y) const {
    return height - (_y + 1.) * height / 3.;
}

Renderer::Renderer(Geometry &_painter, const Model &_model)
    : model(_model), painter(_painter) {
    width   = painter.get_width();
    height  = painter.get_height();
    zbuffer = new float[width * height];
    for (size_t i = 0; i < width * height; i++) {
        zbuffer[i] = -std::numeric_limits<float>::max();
    }
    return;
}

Renderer::~Renderer(void) {
    delete zbuffer;
    return;
}

bool Renderer::render(void) {
    // line_zbuffer();
    fill_triangle_zbuffer();
    // fill_circle_zbuffer();
    return true;
}

bool Renderer::render(void) const {
    // line();
    fill_triangle();
    // fill_circle();
    return true;
}

bool Renderer::set_light(const Vectorf3 &_light) {
    light_dir = _light;
    return true;
}

bool Renderer::set_size(size_t _w, size_t _h) {
    width  = _w;
    height = _h;
    return true;
}

Vectorf3 Renderer::get_light(void) const {
    return light_dir;
}

Vectors2 Renderer::get_size(void) const {
    return Vectors2(width, height);
}

bool Renderer::save(const std::string &_filename) const {
    return painter.save(_filename);
}

bool Renderer::line(void) const {
    // 所有面
    for (size_t i = 0; i < model.nfaces(); i++) {
        // 取出一个面
        vector<int> face = model.face(i);
        // 处理 x y 坐标
        for (size_t j = 0; j < 3; j++) {
            Vectorf3 v0 = model.vert(face.at(j));
            Vectorf3 v1 = model.vert(face.at((j + 1) % 3));
            size_t   x0 = get_x(v0.coord.x);
            size_t   y0 = get_y(v0.coord.y);
            size_t   x1 = get_x(v1.coord.x);
            size_t   y1 = get_y(v1.coord.y);
            painter.line(x0, y0, x1, y1, white);
        }
    }
    return true;
}

bool Renderer::circle(void) const {
    painter.circle(mid_width, mid_height, 50, white);
    return true;
}

bool Renderer::line_zbuffer(void) {
    for (size_t i = 0; i < model.nfaces(); i++) {
        vector<int> face = model.face(i);
        for (size_t j = 0; j < 3; j++) {
            Vectorf3 v0 = model.vert(face.at(j));
            Vectorf3 v1 = model.vert(face.at((j + 1) % 3));
            size_t   x0 = get_x(v0.coord.x);
            size_t   y0 = get_y(v0.coord.y);
            size_t   x1 = get_x(v1.coord.x);
            size_t   y1 = get_y(v1.coord.y);
            float    z0 = v0.coord.z;
            float    z1 = v1.coord.z;
            painter.line(x0, y0, z0, x1, y1, z1, zbuffer, white);
        }
    }
    return true;
}

bool Renderer::fill_triangle(void) const {
    for (size_t i = 0; i < model.nfaces(); i++) {
        std::vector<int> face = model.face(i);
        Vectors2         screen_coords[3];
        Vectorf3         world_coords[3];
        for (size_t j = 0; j < 3; j++) {
            Vectorf3 v               = model.vert(face[j]);
            screen_coords[j].coord.x = get_x(v.coord.x);
            screen_coords[j].coord.y = get_y(v.coord.y);
            world_coords[j]          = model.vert(face[j]);
        }
        Vectorf3 n = (world_coords[2] - world_coords[0]) ^
                     (world_coords[1] - world_coords[0]);
        float intensity = light_dir * n.unit();
        if (intensity > 0) {
            painter.triangle(screen_coords[0], screen_coords[1],
                             screen_coords[2],
                             TGAColor(intensity * 255, intensity * 255,
                                      intensity * 255, 255));
        }
    }
    return true;
}

bool Renderer::fill_triangle_zbuffer(void) {
    for (size_t i = 0; i < model.nfaces(); i++) {
        std::vector<int> face = model.face(i);
        Vectorf3         screen_coords[3];
        Vectorf3         world_coords[3];
        for (size_t j = 0; j < 3; j++) {
            Vectorf3 v = model.vert(face[j]);
            screen_coords[j] =
                Vectorf3(get_x(v.coord.x), get_y(v.coord.y), v.coord.z);
            world_coords[j] = v;
        }
        Vectorf3 n = (world_coords[2] - world_coords[0]) ^
                     (world_coords[1] - world_coords[0]);
        float intensity = light_dir * n.unit();
        if (intensity > 0) {
            painter.triangle(screen_coords[0], screen_coords[1],
                             screen_coords[2], zbuffer,
                             TGAColor(intensity * 255, intensity * 255,
                                      intensity * 255, 255));
        }
    }
    return true;
}

bool Renderer::fill_circle(void) const {
    return true;
}

bool Renderer::fill_circle_zbuffer(void) {
    return true;
}
