
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

#include "model.hpp"

#include <utility>

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

#include "exception.hpp"
#include "log.h"

model_t::vertex_t::vertex_t(coord_t _coord, normal_t _normal,
                            texture_coord_t _texture_coord,
                            const color_t &_color)
    : coord(std::move(_coord)), normal(std::move(_normal)),
      texture_coord(std::move(_texture_coord)), color(_color) {}

auto model_t::vertex_t::operator*(const matrix4f_t &_tran) const
    -> model_t::vertex_t {
  auto vertex(*this);

  auto coord4 = vector4f_t(coord.x(), coord.y(), coord.z(), 1);
  auto coord3 = _tran * coord4;

  vertex.coord.x() = coord3.x();
  vertex.coord.y() = coord3.y();
  vertex.coord.z() = coord3.z();

  /// @todo 变换法线

  return vertex;
}

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

auto model_t::face_t::operator*(const matrix4f_t &_tran) const
    -> model_t::face_t {
  auto face(*this);
  face.v0 = face.v0 * _tran;
  face.v1 = face.v1 * _tran;
  face.v2 = face.v2 * _tran;

  /// @todo 通过矩阵变换法线
  auto v2v0 = face.v2.coord - face.v0.coord;
  auto v1v0 = face.v1.coord - face.v0.coord;
  face.normal = (v2v0.cross(v1v0)).normalized();

  return face;
}

model_t::model_t(const std::string &_obj_path, const std::string &_mtl_path)
    : obj_path(_obj_path), mtl_path(_mtl_path) {
  tinyobj::ObjReader reader;
  tinyobj::ObjReaderConfig config;
  config.mtl_search_path = _mtl_path;
  // 默认开启三角化
  auto ret = reader.ParseFromFile(_obj_path, config);
  if (!ret) {
    if (!reader.Error().empty()) {
      throw SimpleRenderer::exception(reader.Error());
    }
  }

  if (!reader.Warning().empty()) {
    SPDLOG_LOGGER_WARN(SRLOG, "TinyObjReader {}", reader.Warning());
  }

  const auto &attrib = reader.GetAttrib();
  const auto &shapes = reader.GetShapes();
  const auto &materials = reader.GetMaterials();

  SPDLOG_LOGGER_INFO(
      SRLOG,
      "加载模型: {}, 顶点数: {}, 法线数: {}, 颜色数: {}, UV数: {}, "
      "子模型数: {}, 材质数: {}",
      _obj_path, attrib.vertices.size() / 3, attrib.normals.size() / 3,
      attrib.colors.size() / 3, attrib.texcoords.size() / 2, shapes.size(),
      materials.size());

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
      if (num_face_vertices != TRIANGLE_FACE_VERTEX_COUNT) {
        throw SimpleRenderer::exception(
            "num_face_vertices != TRIANGLE_FACE_VERTEX_COUNT");
      }

      auto vertexes = std::array<vertex_t, 3>();
      // 遍历面上的顶点，这里 fv == 3
      for (size_t num_face_vertices_idx = 0;
           num_face_vertices_idx < num_face_vertices; num_face_vertices_idx++) {
        // 获取索引
        auto idx = shapes[shapes_size]
                       .mesh.indices[index_offset + num_face_vertices_idx];

        // 构造顶点信息并保存
        // 每组顶点信息有 xyz 三个分量，因此需要 3*
        auto coord = coord_t(attrib.vertices[3 * size_t(idx.vertex_index) + 0],
                             attrib.vertices[3 * size_t(idx.vertex_index) + 1],
                             attrib.vertices[3 * size_t(idx.vertex_index) + 2]);

        // 如果法线索引存在(即 idx.normal_index >= 0)，
        // 则构造并保存，否则设置为 0
        normal_t normal = normal_t::Zero();
        if (idx.normal_index >= 0) {
          normal = normal_t(attrib.normals[3 * size_t(idx.normal_index) + 0],
                            attrib.normals[3 * size_t(idx.normal_index) + 1],
                            attrib.normals[3 * size_t(idx.normal_index) + 2]);
        }

        // 如果贴图索引存在(即 idx.texcoord_index >= 0)，
        // 则构造并保存，否则设置为 0
        texture_coord_t texture_coord = texture_coord_t::Zero();
        if (idx.texcoord_index >= 0) {
          texture_coord = texture_coord_t(
              attrib.texcoords[2 * size_t(idx.texcoord_index) + 0],
              attrib.texcoords[2 * size_t(idx.texcoord_index) + 1]);
        }

        // 顶点颜色，如果 obj 文件中没有指定则设为 1(白色)，范围 [0, 1]
        auto color = color_t(attrib.colors[3 * size_t(idx.vertex_index) + 0],
                             attrib.colors[3 * size_t(idx.vertex_index) + 1],
                             attrib.colors[3 * size_t(idx.vertex_index) + 2]);
        vertexes.at(num_face_vertices_idx) =
            vertex_t(coord, normal, texture_coord, color);
      }
      index_offset += num_face_vertices;

      // 如果材质不为空，加载材质信息
      auto material = material_t();
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
      // 添加到 face 中
      faces.emplace_back(vertexes[0], vertexes[1], vertexes[2], material);
    }
  }

  // 计算模型包围盒
  set_box();
  normalize();
}

auto model_t::operator*(const matrix4f_t &_tran) const -> model_t {
  auto model = model_t(*this);

  for (auto &i : model.faces) {
    i = i * _tran;
  }

  return model;
}

auto model_t::get_face() const -> const std::vector<model_t::face_t> & {
  return faces;
}

void model_t::set_box() {
  auto max = faces.at(0).v0.coord;
  auto min = faces.at(0).v0.coord;

  for (const auto &i : faces) {
    auto curr_max_x =
        std::max(i.v0.coord.x(), std::max(i.v1.coord.x(), i.v2.coord.x()));
    auto curr_max_y =
        std::max(i.v0.coord.y(), std::max(i.v1.coord.y(), i.v2.coord.y()));
    auto curr_max_z =
        std::max(i.v0.coord.z(), std::max(i.v1.coord.z(), i.v2.coord.z()));

    max.x() = curr_max_x > max.x() ? curr_max_x : max.x();
    max.y() = curr_max_y > max.y() ? curr_max_y : max.y();
    max.z() = curr_max_z > max.z() ? curr_max_z : max.z();

    auto curr_min_x =
        std::max(i.v0.coord.x(), std::max(i.v1.coord.x(), i.v2.coord.x()));
    auto curr_min_y =
        std::max(i.v0.coord.y(), std::max(i.v1.coord.y(), i.v2.coord.y()));
    auto curr_min_z =
        std::max(i.v0.coord.z(), std::max(i.v1.coord.z(), i.v2.coord.z()));

    min.x() = curr_min_x < min.x() ? curr_min_x : min.x();
    min.y() = curr_min_y < min.y() ? curr_min_y : min.y();
    min.z() = curr_min_z < min.z() ? curr_min_z : min.z();
  }
  box.min = min;
  box.max = max;
  center.x() = (max.x() + min.x()) / 2.f;
  center.y() = (max.y() + min.y()) / 2.f;
  center.z() = (max.z() + min.z()) / 2.f;

  SPDLOG_LOGGER_INFO(SRLOG, "box: \n{},\ncenter: {}", box, center);
}

void model_t::normalize() {
  auto w = std::abs(box.max.x()) + std::abs(box.min.x());
  auto h = std::abs(box.max.y()) + std::abs(box.min.y());
  auto d = std::abs(box.max.z()) + std::abs(box.min.z());
  auto scale = 2.0f / std::max(w, std::max(h, d));

  // 平移
  auto translate_mat = matrix4f_t();
  translate_mat.setIdentity();
  translate_mat(0, 3) = -center.x();
  translate_mat(1, 3) = -center.y();
  translate_mat(2, 3) = -center.z();

  SPDLOG_LOGGER_INFO(SRLOG, "translate_mat: {}", translate_mat);

  // 缩放
  auto scale_mat = matrix4f_t();
  scale_mat.setIdentity();
  scale_mat.diagonal()[0] = scale;
  scale_mat.diagonal()[1] = scale;
  scale_mat.diagonal()[2] = scale;

  box.min = vector3f_t(-1, -1, -1);
  box.max = vector3f_t(1, 1, 1);
  center = vector3f_t(0, 0, 0);

  *this = *this * scale_mat * translate_mat;
}
