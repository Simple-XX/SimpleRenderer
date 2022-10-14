
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

#include "model.h"
#include "iostream"

#define TINYOBJLOADER_IMPLEMENTATION
#include "3rd/tiny_obj_loader.h"

model_t::vertex_t::vertex_t(void) {
    return;
}

model_t::vertex_t::vertex_t(const vertex_t& _vertex)
    : coord(_vertex.coord),
      normal(_vertex.normal),
      texcoord(_vertex.texcoord),
      color(_vertex.color) {
    return;
}

model_t::vertex_t::vertex_t(const coord_t& _coord, const normal_t& _normal,
                            const texcoord_t& _texcoord, const color_t& _color)
    : coord(_coord), normal(_normal), texcoord(_texcoord) {
    // 将颜色归一化
    if (_color.x > 1 || _color.y > 1 || _color.z > 1) {
        color = _color / std::numeric_limits<uint8_t>::max();
    }
    else {
        color = _color;
    }
    return;
}

model_t::vertex_t::~vertex_t(void) {
    return;
}

model_t::vertex_t& model_t::vertex_t::operator=(const vertex_t& _vertex) {
    coord    = _vertex.coord;
    normal   = _vertex.normal;
    texcoord = _vertex.texcoord;
    color    = _vertex.color;
    return *this;
}

model_t::face_t::face_t(void) {
    return;
}

model_t::face_t::face_t(const face_t& _face)
    : v0(_face.v0),
      v1(_face.v1),
      v2(_face.v2),
      normal(_face.normal),
      material(_face.material) {
    return;
}

model_t::face_t::face_t(const model_t::vertex_t& _v0,
                        const model_t::vertex_t& _v1,
                        const model_t::vertex_t& _v2,
                        const material_t&        _material)
    : v0(_v0), v1(_v1), v2(_v2), material(_material) {
    // 计算法向量
    // 如果 obj 内包含法向量，直接使用即可
    if (_v0.normal.length() != 0 && _v1.normal.length() != 0
        && _v2.normal.length() != 0) {
        normal = (_v0.normal + _v1.normal + _v2.normal);
        normal = normal.normalize();
    }
    // 手动计算
    else {
        // 两条相临边的叉积
        auto v2v0   = _v2.coord - _v0.coord;
        auto v1v0   = _v1.coord - _v0.coord;
        auto normal = v2v0 ^ v1v0;
        normal      = normal.normalize();
    }
    return;
}

model_t::face_t::~face_t(void) {
    return;
}

model_t::face_t& model_t::face_t::operator=(const face_t& _face) {
    v0       = _face.v0;
    v1       = _face.v1;
    v2       = _face.v2;
    normal   = _face.normal;
    material = _face.material;
    return *this;
}

/// @todo 确认这里的乘法顺序
const model_t::face_t model_t::face_t::operator*(
  const std::pair<const matrix4f_t, const matrix4f_t>& _matrices) const {
    face_t ret(*this);
    // 变换坐标
    ret.v0.coord = _matrices.first * ret.v0.coord;
    ret.v1.coord = _matrices.first * ret.v1.coord;
    ret.v2.coord = _matrices.first * ret.v2.coord;
    // 变换法线
    ret.normal   = _matrices.second * ret.normal;
    return ret;
}

model_t::face_t& model_t::face_t::operator*=(
  const std::pair<const matrix4f_t, const matrix4f_t>& _matrices) {
    // 变换坐标
    v0.coord = _matrices.first * v0.coord;
    v1.coord = _matrices.first * v1.coord;
    v2.coord = _matrices.first * v2.coord;
    // 变换法线
    normal   = _matrices.second * normal;
    return *this;
}

model_t::model_t(void) {
    return;
}

model_t::model_t(const model_t& _model) {
    face = _model.face;
    return;
}

model_t::model_t(const std::string& _obj_path, const std::string& _mtl_path) {
    tinyobj::ObjReader       reader;
    tinyobj::ObjReaderConfig config;
    config.mtl_search_path = _mtl_path;
    // 默认开启三角化
    auto ret               = reader.ParseFromFile(_obj_path, config);
    if (ret == false) {
        if (reader.Error().empty() == false) {
            throw(std::runtime_error(log(reader.Error())));
        }
    }

    if (!reader.Warning().empty()) {
        std::cout << "TinyObjReader: " << reader.Warning();
    }

    const auto& attrib    = reader.GetAttrib();
    const auto& shapes    = reader.GetShapes();
    const auto& materials = reader.GetMaterials();

    printf("顶点数：%ld, ", attrib.vertices.size() / 3);
    printf("法线数：%ld, ", attrib.normals.size() / 3);
    printf("颜色数：%ld, ", attrib.colors.size() / 3);
    printf("UV数：%ld, ", attrib.texcoords.size() / 2);
    printf("子模型数：%ld, ", shapes.size());
    printf("材质数：%ld\n", materials.size());
    // Loop over shapes
    for (size_t s = 0; s < shapes.size(); s++) {
        // Loop over faces(polygon)
        size_t index_offset = 0;
        for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) {
            // 由于开启了三角化，所有的 shape 都是由三个点组成的，即 fv == 3
            size_t fv = size_t(shapes[s].mesh.num_face_vertices[f]);
            if (fv != 3) {
                throw(std::runtime_error(log("fv != 3")));
            }
            coord_t    coord;
            normal_t   normal;
            color_t    color;
            texcoord_t texcoord;
            material_t material;
            vertex_t   vertexes[3];
            // 遍历面上的顶点，这里 fv == 3
            for (size_t v = 0; v < fv; v++) {
                // 获取索引
                tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];

                // 构造顶点信息并保存
                // 每组顶点信息有 xyz 三个分量，因此需要 3*
                coord
                  = coord_t(attrib.vertices[3 * size_t(idx.vertex_index) + 0],
                            attrib.vertices[3 * size_t(idx.vertex_index) + 1],
                            attrib.vertices[3 * size_t(idx.vertex_index) + 2],
                            vector4f_t::W_POINT);

                // 如果法线索引存在(即 idx.normal_index >= 0)，
                // 则构造并保存，否则设置为 0
                if (idx.normal_index >= 0) {
                    normal = normal_t(
                      attrib.normals[3 * size_t(idx.normal_index) + 0],
                      attrib.normals[3 * size_t(idx.normal_index) + 1],
                      attrib.normals[3 * size_t(idx.normal_index) + 2]);
                }
                else {
                    normal = normal_t(0, 0, 0);
                }

                // 如果贴图索引存在(即 idx.texcoord_index >= 0)，
                // 则构造并保存，否则设置为 0
                if (idx.texcoord_index >= 0) {
                    texcoord = texcoord_t(
                      attrib.texcoords[2 * size_t(idx.texcoord_index) + 0],
                      attrib.texcoords[2 * size_t(idx.texcoord_index) + 1]);
                }
                else {
                    texcoord = texcoord_t(0, 0);
                }

                // 顶点颜色，如果 obj 文件中没有指定则设为 1(白色)，范围 [0, 1]
                color
                  = color_t(attrib.colors[3 * size_t(idx.vertex_index) + 0],
                            attrib.colors[3 * size_t(idx.vertex_index) + 1],
                            attrib.colors[3 * size_t(idx.vertex_index) + 2]);
                vertexes[v] = vertex_t(coord, normal, texcoord, color);
            }
            index_offset += fv;

            // per-face material
            if (materials.size() > 0) {
                // std::cout << "materials[s].name: " << materials[s].name
                //           << std::endl;
                // std::cout << "materials[s].diffuse_texname: "
                //           << materials[s].diffuse_texname << std::endl;
                // std::cout << "materials[s].diffuse_texname: "
                //           << materials[s].ambient[0] << std::endl;
                material.shininess = materials[s].shininess;
                material.ambient
                  = vector4f_t(materials[s].ambient[0], materials[s].ambient[1],
                               materials[s].ambient[2]);
                material.diffuse
                  = vector4f_t(materials[s].diffuse[0], materials[s].diffuse[1],
                               materials[s].diffuse[2]);
                material.specular = vector4f_t(materials[s].specular[0],
                                               materials[s].specular[1],
                                               materials[s].specular[2]);
            }
            face.push_back(face_t(vertexes[0], vertexes[1], vertexes[2],
                                  material));
        }
    }

    return;
}

model_t::~model_t(void) {
    return;
}

model_t& model_t::operator=(const model_t& _model) {
    face = _model.face;
    return *this;
}

const std::vector<model_t::face_t>& model_t::get_face(void) const {
    return face;
}
