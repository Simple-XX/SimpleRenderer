
/**
 * @file model.cpp
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

#include "iostream"
#include "model.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include "3rd/tiny_obj_loader.h"

/**
 * @brief 顶点回调函数
 * @param  _user_data       保存顶点信息
 * @param  _x               x 坐标
 * @param  _y               y 坐标
 * @param  _z               z 坐标
 * @param  _w               未使用
 */
static void vertex_cb(void *_user_data, float _x, float _y, float _z, float) {
    mesh_t *mesh = reinterpret_cast<mesh_t *>(_user_data);
    // printf("v[%ld] = %f, %f, %f (_w %f)\n", mesh->vertices.size(), _x, _y,
    // _z,
    //        _w);
    // 保存顶点信息
    // Discard _w
    vertex_t tmp(_x, _y, _z);
    mesh->vertices.push_back(tmp);
    return;
}

/**
 * @brief 法线回调函数
 * @param  _user_data       保存法线信息
 * @param  _x               x 坐标
 * @param  _y               y 坐标
 * @param  _z               z 坐标
 */
static void normal_cb(void *_user_data, float _x, float _y, float _z) {
    mesh_t *mesh = reinterpret_cast<mesh_t *>(_user_data);
    // printf("vn[%ld] = %f, %f, %f\n", mesh->normals.size() / 3, _x, _y, _z);
    // 保存法线信息
    normal_t tmp(_x, _y, _z);
    mesh->normals.push_back(tmp);
    return;
}

/**
 * @brief 贴图回调函数
 * @param  _user_data       保存贴图信息
 * @param  _x               x 坐标
 * @param  _y               y 坐标
 * @param  _z               z 坐标
 */
static void texcoord_cb(void *_user_data, float _x, float _y, float _z) {
    mesh_t *mesh = reinterpret_cast<mesh_t *>(_user_data);
    // printf("vt[%ld] = %f, %f, %f\n", mesh->texcoords.size() / 3, _x, _y, _z);
    // 保存贴图信息
    texcoord_t tmp(_x, _y, _z);
    mesh->texcoords.push_back(tmp);
    return;
}

/**
 * @brief 索引回调函数
 * @param  _user_data       保存索引信息
 * @param  _indices         索引数组
 * @param  _num_indices     数组长度
 */
static void index_cb(void *_user_data, tinyobj::index_t *_indices,
                     int _num_indices) {
    mesh_t *mesh = reinterpret_cast<mesh_t *>(_user_data);
    // 保存顶点索引/法线索引/贴图索引
    // 无效值用 -1 填充
    for (int i = 0; i < _num_indices; i++) {
        tinyobj::index_t idx = _indices[i];
        // printf("idx[%ld] = %d, %d, %d\n", mesh->indices.size(),
        //        idx.vertex_index - 1, idx.normal_index - 1,
        //        idx.texcoord_index - 1);
        // -1 是因为 obj 文件的索引从 1 开始，-1 以让其从 0 开始
        index_t tmp(idx.vertex_index - 1, idx.normal_index - 1,
                    idx.texcoord_index - 1);
        mesh->indices.push_back(tmp);
    }
    return;
}

/**
 * @brief 材质回调函数
 * @param  _user_data       保存材质数据
 * @param  _name            材质名
 * @param  _material_idx    材质索引
 */
static void usemtl_cb(void *_user_data, const char *_name, int _material_idx) {
    mesh_t *mesh = reinterpret_cast<mesh_t *>(_user_data);
    // 保存材质信息
    if ((_material_idx > -1) &&
        ((unsigned)_material_idx < mesh->materials.size())) {
        printf("usemtl. material id = %d(name = %s)\n", _material_idx,
               mesh->materials[_material_idx].name.c_str());
    }
    else {
        printf("usemtl. name = %s\n", _name);
    }
    return;
}

/**
 * @brief 材质回调函数
 * @param  _user_data       保存材质数据
 * @param  _materials       材质数组
 * @param  num_materials    材质数量
 */
static void mtllib_cb(void *_user_data, const tinyobj::material_t *_materials,
                      int num_materials) {
    mesh_t *mesh = reinterpret_cast<mesh_t *>(_user_data);
    printf("mtllib. # of materials = %d\n", num_materials);
    // 保存材质信息
    for (int i = 0; i < num_materials; i++) {
        mesh->materials.push_back(_materials[i]);
    }
    return;
}

/**
 * @brief 组回调函数
 * @param  _names           组名数据
 * @param  _num_names       组名数量
 */
static void group_cb(void *, const char **_names, int _num_names) {
    printf("group : name = \n");
    for (int i = 0; i < _num_names; i++) {
        printf("  %s\n", _names[i]);
    }
    return;
}

/**
 * @brief obj 回调函数
 * @param  _name            obj 名
 */
static void object_cb(void *, const char *_name) {
    printf("object : name = %s\n", _name);
    return;
}

model_t::model_t(const std::string &_obj_path, const std::string &_mtl_path) {
    tinyobj::callback_t cb;
    cb.vertex_cb   = vertex_cb;
    cb.normal_cb   = normal_cb;
    cb.texcoord_cb = texcoord_cb;
    cb.index_cb    = index_cb;
    cb.usemtl_cb   = usemtl_cb;
    cb.mtllib_cb   = mtllib_cb;
    cb.group_cb    = group_cb;
    cb.object_cb   = object_cb;

    try {
        std::string   warn;
        std::string   err;
        std::ifstream ifs(_obj_path.c_str());

        if (ifs.fail()) {
            throw(std::runtime_error(log("ifs.fail()")));
        }

        tinyobj::MaterialFileReader mtlReader(_mtl_path);
        // 加载
        auto ret = tinyobj::LoadObjWithCallback(ifs, cb, &mesh, &mtlReader,
                                                &warn, &err);
        if (ret == false) {
            throw(std::runtime_error(log("ret == false")));
        }
        if (!warn.empty()) {
            std::cout << "WARN: " << warn << std::endl;
        }
        if (!err.empty()) {
            std::cerr << err << std::endl;
        }

    } catch (const std::runtime_error &e) {
        std::cerr << log(e.what()) << std::endl;
        return;
    }

    printf("# of vertices         = %ld\n", mesh.vertices.size() / 3);
    printf("# of normals          = %ld\n", mesh.normals.size() / 3);
    printf("# of texcoords        = %ld\n", mesh.texcoords.size() / 2);
    printf("# of indices          = %ld\n", mesh.indices.size());
    printf("# of materials = %ld\n", mesh.materials.size());

    // 添加到 face
    for (size_t i = 0; i < mesh.indices.size(); i += 3) {
        vertex_t tmp_p0 = mesh.vertices.at(mesh.indices.at(i).x);
        vertex_t tmp_p1 = mesh.vertices.at(mesh.indices.at(i + 1).x);
        vertex_t tmp_p2 = mesh.vertices.at(mesh.indices.at(i + 2).x);
        faces.push_back(face_t(tmp_p0, tmp_p1, tmp_p2));
    }

    return;
}

model_t::~model_t(void) {
    return;
}

const std::vector<vertex_t> &model_t::get_vertex(void) const {
    return mesh.vertices;
}

const std::vector<normal_t> &model_t::get_normal(void) const {
    return mesh.normals;
}

const std::vector<texcoord_t> &model_t::get_texcoord(void) const {
    return mesh.texcoords;
}

const std::vector<index_t> &model_t::get_index(void) const {
    return mesh.indices;
}

const std::vector<tinyobj::material_t> &model_t::get_material(void) const {
    return mesh.materials;
}

const std::vector<face_t> &model_t::get_face(void) const {
    return faces;
}
