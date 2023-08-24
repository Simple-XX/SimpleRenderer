
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
#include <utility>

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

#include "log.hpp"

model_t::vertex_t::vertex_t(coord_t _coord, normal_t _normal,
                            texture_coord_t _texture_coord,
                            const color_t &_color)
    : coord(std::move(_coord)), normal(std::move(_normal)),
      texture_coord(std::move(_texture_coord)), color(_color) {}

// model_t::vertex_t
// operator*(const std::pair<const matrix4f_t, const matrix4f_t> &_matrices,
//           const model_t::vertex_t &_vertex) {
//   auto ret(_vertex);
//   // 变换坐标
//
//   ret.coord = _matrices.first * _vertex.coord;
//   ret.coord = _matrices.first * _vertex.coord;
//   ret.coord = _matrices.first * _vertex.coord;
//   // 变换法线
//   /// @todo 法线变换矩阵
//   // ret.normal   = _matrices.second * _vertex.normal;
//   return ret;
// }

// model_t::face_t
// operator*(const std::pair<const matrix4f_t, const matrix4f_t> &_matrices,
//           const model_t::face_t &_face) {
//   auto ret(_face);
//   // 变换坐标
//   ret.v0 = _matrices * _face.v0;
//   ret.v1 = _matrices * _face.v1;
//   ret.v2 = _matrices * _face.v2;
//   /// @todo 法线变换矩阵
//   auto v2v0 = ret.v2.coord - ret.v0.coord;
//   auto v1v0 = ret.v1.coord - ret.v0.coord;
//   ret.normal = (v2v0.cross(v1v0)).normalized();
//   return ret;
// }

model_t::face_t::face_t(const model_t::vertex_t &_v0,
                        const model_t::vertex_t &_v1,
                        const model_t::vertex_t &_v2, material_t _material)
    : v0(_v0), v1(_v1), v2(_v2), material(std::move(_material)) {
  // 计算法向量
  // 如果 obj 内包含法向量，直接使用即可
  if (_v0.normal.norm() != 0 && _v1.normal.norm() != 0 &&
      _v2.normal.norm() != 0) {
    normal = (_v0.normal + _v1.normal + _v2.normal).normalized();
  }
  // 手动计算
  else {
    // 两条相临边的叉积
    auto v2v0 = _v2.coord - _v0.coord;
    auto v1v0 = _v1.coord - _v0.coord;
    normal = (v2v0.cross(v1v0)).normalized();
  }
}

model_t::model_t(const std::string &_obj_path, const std::string &_mtl_path) {
  tinyobj::ObjReader reader;
  tinyobj::ObjReaderConfig config;
  config.mtl_search_path = _mtl_path;
  // 默认开启三角化
  auto ret = reader.ParseFromFile(_obj_path, config);
  if (!ret) {
    if (!reader.Error().empty()) {
      throw(std::runtime_error(log(reader.Error())));
    }
  }

  if (!reader.Warning().empty()) {
    std::cout << "TinyObjReader: " << reader.Warning();
  }

  const auto &attrib = reader.GetAttrib();
  const auto &shapes = reader.GetShapes();
  const auto &materials = reader.GetMaterials();

  std::cout << "顶点数：%ld, " << attrib.vertices.size() / 3;
  std::cout << "法线数：%ld, " << attrib.normals.size() / 3;
  std::cout << "颜色数：%ld, " << attrib.colors.size() / 3;
  std::cout << "UV数：%ld, " << attrib.texcoords.size() / 2;
  std::cout << "子模型数：%ld, " << shapes.size();
  std::cout << "材质数：%ld" << materials.size() << '\n';

  // 用于计算最大/最小的点
  /// @todo
  auto min = vector3f_t(std::numeric_limits<float>::max(),
                        std::numeric_limits<float>::max(),
                        std::numeric_limits<float>::max());
  auto max = vector3f_t(std::numeric_limits<float>::min(),
                        std::numeric_limits<float>::min(),
                        std::numeric_limits<float>::min());

  // 遍历所有 shape
  for (size_t shapes_size = 0; shapes_size < shapes.size(); shapes_size++) {
    // Loop over faces(polygon)
    size_t index_offset = 0;
    for (size_t num_face_vertices_size = 0;
         num_face_vertices_size <
         shapes[shapes_size].mesh.num_face_vertices.size();
         num_face_vertices_size++) {
      // 由于开启了三角化，所有的 shape 都是由三个点组成的，即 fv == 3
      auto num_face_vertices = size_t(
          shapes[shapes_size].mesh.num_face_vertices[num_face_vertices_size]);
      if (num_face_vertices != 3) {
        throw(std::runtime_error(log("fv != 3")));
      }
      coord_t coord;
      normal_t normal;
      color_t color;
      texture_coord_t texture_coord;
      material_t material;
      std::array<vertex_t, 3> vertexes;

      // 遍历面上的顶点，这里 fv == 3
      for (size_t num_face_vertices_idx = 0;
           num_face_vertices_idx < num_face_vertices; num_face_vertices_idx++) {
        // 获取索引
        auto idx = shapes[shapes_size]
                       .mesh.indices[index_offset + num_face_vertices_idx];

        // 构造顶点信息并保存
        // 每组顶点信息有 xyz 三个分量，因此需要 3*
        coord = coord_t(attrib.vertices[3 * size_t(idx.vertex_index) + 0],
                        attrib.vertices[3 * size_t(idx.vertex_index) + 1],
                        attrib.vertices[3 * size_t(idx.vertex_index) + 2]);

        // 如果法线索引存在(即 idx.normal_index >= 0)，
        // 则构造并保存，否则设置为 0
        if (idx.normal_index >= 0) {
          normal = normal_t(attrib.normals[3 * size_t(idx.normal_index) + 0],
                            attrib.normals[3 * size_t(idx.normal_index) + 1],
                            attrib.normals[3 * size_t(idx.normal_index) + 2]);
        } else {
          normal = normal_t(0, 0, 0);
        }

        // 如果贴图索引存在(即 idx.texcoord_index >= 0)，
        // 则构造并保存，否则设置为 0
        if (idx.texcoord_index >= 0) {
          texture_coord = texture_coord_t(
              attrib.texcoords[2 * size_t(idx.texcoord_index) + 0],
              attrib.texcoords[2 * size_t(idx.texcoord_index) + 1]);
        } else {
          texture_coord = texture_coord_t(0, 0);
        }

        // 顶点颜色，如果 obj 文件中没有指定则设为 1(白色)，范围 [0, 1]
        color = color_t(attrib.colors[3 * size_t(idx.vertex_index) + 0],
                        attrib.colors[3 * size_t(idx.vertex_index) + 1],
                        attrib.colors[3 * size_t(idx.vertex_index) + 2]);
        vertexes.at(num_face_vertices_idx) =
            vertex_t(coord, normal, texture_coord, color);
      }
      index_offset += num_face_vertices;

      // per-face material
      if (!materials.empty()) {
        material.shininess = materials[shapes_size].shininess;
        material.ambient = vector3f_t(materials[shapes_size].ambient[0],
                                      materials[shapes_size].ambient[1],
                                      materials[shapes_size].ambient[2]);
        material.diffuse = vector3f_t(materials[shapes_size].diffuse[0],
                                      materials[shapes_size].diffuse[1],
                                      materials[shapes_size].diffuse[2]);
        material.specular = vector3f_t(materials[shapes_size].specular[0],
                                       materials[shapes_size].specular[1],
                                       materials[shapes_size].specular[2]);
      }
      face.emplace_back(vertexes[0], vertexes[1], vertexes[2], material);
    }
  }

  //  box.min = min;
  //  box.max = max;
}

auto model_t::operator*(const matrix4f_t &_tran) const -> model_t {
  auto model = model_t(*this);

  /// @todo
  (void)_tran;

  return model;
}

auto model_t::get_face() const -> const std::vector<model_t::face_t> & {
  return face;
}
