
// This file is a part of SimpleXX/SimpleRenderer (https://github.com/SimpleXX/SimpleRenderer).
// Based on https://github.com/ssloy/tinyrenderer
// model.cpp for SimpleXX/SimpleRenderer.

#include "iostream"
#include "fstream"
#include "sstream"
#include "model.h"

using namespace std;

Model::Model(string _filename, TwoD &_twod) : verts(), faces(), twod(_twod) {
    std::ifstream in;
    in.open(_filename, std::ifstream::in);
    if(in.fail() ) return;
    std::string line;
    while(!in.eof() ) {
        std::getline(in, line);
        std::istringstream iss(line.c_str() );
        char trash;
        if(!line.compare(0, 2, "v ") ) {
            iss >> trash;
            std::vector<double> tmp;
            for(int i = 0 ; i < 3 ; i++) {
                double s;
                iss >> s;
                tmp.push_back(s);
            }
            Vectord3 v(tmp);
            verts.push_back(v);
        } else if(!line.compare(0, 2, "f ") ) {
            std::vector<int> f;
            int itrash, idx;
            iss >> trash;
            while(iss >> idx >> trash >> itrash >> trash >> itrash) {
                idx--; // in wavefront obj all indices start at 1, not zero
                f.push_back(idx);
            }
            faces.push_back(f);
        }
    }
    std::cerr << "# v# " << verts.size() << " f# "  << faces.size() << std::endl;
}

Model::~Model() {
}

int Model::nverts() {
    return (int)verts.size();
}

int Model::nfaces() {
    return (int)faces.size();
}

std::vector<int> Model::face(int _idx) {
    return faces.at(_idx);
}

Vectord3 Model::vert(int _i) {
    return verts.at(_i);
}

void Model::to_tga_line(string _filename) {
    for(int i = 0 ; i < nfaces() ; i++) {
        vector<int> face = this->face(i);
        for(int j = 0 ; j < 3 ; j++) {
            Vectord3 v0 = vert(face.at(j) );
            Vectord3 v1 = vert(face.at( (j + 1) % 3) );
            int x0 = (v0.get_vect()[0] + 1.) * twod.get_width() / 3.;
            int y0 = twod.get_height() - (v0.get_vect()[1] + 1.) * twod.get_height() / 2.;
            int x1 = (v1.get_vect()[0] + 1.) * twod.get_width() / 3.;
            int y1 = twod.get_height() - (v1.get_vect()[1] + 1.) * twod.get_height() / 2.;
            twod.line(x0, y0, x1, y1);
        }
    }
    twod.save(_filename);
    return;
}

void Model::to_tga_fill(string _filename) {
    Vectord3 light_dir(0, 0, -1);
    for(int i = 0 ; i < this->nfaces() ; i++) {
        std::vector<int> face = this->face(i);
        Vectori2 screen_coords[3];
        Vectord3 world_coords[3];
        for(int j = 0 ; j < 3 ; j++) {
            Vectord3 v = this->vert(face[j]);
            screen_coords[j] = Vectori2(
                (v.get_vect()[0] + 1.) * twod.get_width() / 3.,
                twod.get_height() - (v.get_vect()[1] + 1.) * twod.get_height() / 2.
                );
            world_coords[j]  = v;
        }
        Vectord3 n = (world_coords[2] - world_coords[0]) ^ (world_coords[1] - world_coords[0]);
        double intensity = light_dir * Vectord3(const_cast<double *>(n.get_unit() ) );
        if(intensity > 0) {
            twod.set_fill(TGAColor(intensity * 255, intensity * 255, intensity * 255, 255) );
            twod.triangle(screen_coords[0], screen_coords[1], screen_coords[2]);
        }
    }
    twod.set_fill();
    twod.save(_filename);
    return;
}
