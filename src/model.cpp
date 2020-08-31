
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

void Model::to_tga(string _filename) {
    for(int i = 0 ; i < nfaces() ; i++) {
        vector<int> face = this->face(i);
        for(int j = 0 ; j < 3 ; j++) {
            Vectord3 v0 = vert(face.at(j) );
            Vectord3 v1 = vert(face.at( (j + 1) % 3) );
            int x0 = (v0.get_vect().at(0) + 1.) * twod.get_width() / 3.;
            int y0 = twod.get_height() - (v0.get_vect().at(1) + 1.) * twod.get_height() / 2.;
            int x1 = (v1.get_vect().at(0) + 1.) * twod.get_width() / 3.;
            int y1 = twod.get_height() - (v1.get_vect().at(1) + 1.) * twod.get_height() / 2.;
            twod.line(x0, y0, x1, y1);
        }
    }
    twod.save(_filename);
    return;
}
