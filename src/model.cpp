
// This file is a part of SimpleXX/SimpleRenderer
// (https://github.com/SimpleXX/SimpleRenderer).
// Based on https://github.com/ssloy/tinyrenderer
// model.cpp for SimpleXX/SimpleRenderer.

#include "iostream"
#include "fstream"
#include "sstream"
#include "model.h"

using namespace std;

Model::Model(const string &_filename) : verts(), faces() {
    std::ifstream in;
    in.open(_filename, std::ifstream::in);
    if (in.fail()) {
        return;
    }
    std::string line;
    while (!in.eof()) {
        std::getline(in, line);
        std::istringstream iss(line.c_str());
        char               trash;
        if (!line.compare(0, 2, "v ")) {
            iss >> trash;
            std::vector<double> tmp;
            for (int i = 0; i < 3; i++) {
                double s;
                iss >> s;
                tmp.push_back(s);
            }
            Vectord3 v(tmp);
            verts.push_back(v);
        }
        else if (!line.compare(0, 2, "f ")) {
            std::vector<int> f;
            int              itrash, idx;
            iss >> trash;
            while (iss >> idx >> trash >> itrash >> trash >> itrash) {
                // in wavefront obj all indices start at 1, not zero
                idx--;
                f.push_back(idx);
            }
            faces.push_back(f);
        }
    }
    std::cerr << "# v# " << verts.size() << " f# " << faces.size() << std::endl;
    return;
}

Model::~Model() {
    return;
}

int Model::nverts() const {
    return (int)verts.size();
}

int Model::nfaces() const {
    return (int)faces.size();
}

std::vector<int> Model::face(int _idx) const {
    return faces.at(_idx);
}

Vectord3 Model::vert(int _i) const {
    return verts.at(_i);
}
