
/**
 * @file model.h
 * @brief 模型抽象
 * @author Zone.N (Zone.Niuzh@hotmail.com)
 * @version 1.0
 * @date 2022-06-06
 * @copyright MIT LICENSE
 * https://github.com/Simple-XX/SimpleRenderer
 * @par change log:
 * <table>
 * <tr><th>Date<th>Author<th>Description
 * <tr><td>2022-09-04<td>Zone.N<td>迁移到 doxygen
 * </table>
 */

#ifndef _MODEL_H_
#define _MODEL_H_

#include "vector"
#include "string"
#include "3rd/tiny_obj_loader.h"

// 顶点
struct vertex_t {
    float x;
    float y;
    float z;
    vertex_t(void) {
        x = 0;
        y = 0;
        z = 0;
        return;
    }
    vertex_t(const float _x, const float _y, const float _z) {
        x = _x;
        y = _y;
        z = _z;
        return;
    }
};

// 法向量
struct normal_t {
    float x;
    float y;
    float z;
    normal_t(const float _x, const float _y, const float _z) {
        x = _x;
        y = _y;
        z = _z;
        return;
    }
};

// 贴图
struct texcoord_t {
    float x;
    float y;
    float z;
    texcoord_t(const float _x, const float _y, const float _z) {
        x = _x;
        y = _y;
        z = _z;
        return;
    }
};

// 索引
struct index_t {
    int v;
    int vn;
    int vt;
    index_t(const int _v, const int _vn, const int _vt) {
        v  = _v;
        vn = _vn;
        vt = _vt;
        return;
    }
};

// obj/mtl 文件的原始数据
struct mesh_t {
    std::vector<vertex_t>            vertices;
    std::vector<normal_t>            normals;
    std::vector<texcoord_t>          texcoords;
    std::vector<index_t>             indices;
    std::vector<tinyobj::material_t> materials;
};

// 面
struct face_t {
    vertex_t p0;
    vertex_t p1;
    vertex_t p2;
    face_t(const vertex_t &_p0, const vertex_t &_p1, const vertex_t &_p2) {
        p0 = _p0;
        p1 = _p1;
        p2 = _p2;
        return;
    }
};

class model_t {
private:
    // 原始数据
    mesh_t mesh;
    // 所有三角形面
    std::vector<face_t> faces;

public:
    model_t(const std::string &_obj_path, const std::string &_mtl_path = "");
    ~model_t(void);
    const std::vector<vertex_t>            &get_vertex(void) const;
    const std::vector<normal_t>            &get_normal(void) const;
    const std::vector<texcoord_t>          &get_texcoord(void) const;
    const std::vector<index_t>             &get_index(void) const;
    const std::vector<tinyobj::material_t> &get_material(void) const;
    const std::vector<face_t>              &get_face(void) const;
};

#endif /* _MODEL_H_ */
