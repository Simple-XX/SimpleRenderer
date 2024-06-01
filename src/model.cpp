
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
#include <tiny_obj_loader.h>

#include "exception.hpp"
#include "log_system.h"

namespace simple_renderer {

Model::Vertex::Vertex(Coord coord, Normal normal, TextureCoord texture_coord,
                      const Color &color)
    : coord_(std::move(coord)),
      normal_(std::move(normal)),
      texture_coord_(std::move(texture_coord)),
      color_(color) {}

auto Model::Vertex::operator*(const Matrix4f &tran) const -> Model::Vertex {
  auto vertex(*this);

  auto res4 = Vector4f(coord_.x(), coord_.y(), coord_.z(), 1);
  auto ret4 = Vector4f(tran * res4);

  vertex.coord_.x() = ret4.x();
  vertex.coord_.y() = ret4.y();
  vertex.coord_.z() = ret4.z();

  /// @todo 变换法线

  return vertex;
}

Model::Face::Face(const Model::Vertex &v0, const Model::Vertex &v1,
                  const Model::Vertex &v2, Material material)
    : v0_(v0), v1_(v1), v2_(v2), material_(std::move(material)) {
  // 计算法向量
  // 如果 obj 内包含法向量，直接使用即可
  if (v0.normal_.norm() != 0 && v1.normal_.norm() != 0 &&
      v2.normal_.norm() != 0) {
    normal_ = (v0.normal_ + v1.normal_ + v2.normal_).normalized();
  }
  // 手动计算
  else {
    // 两条相临边的叉积
    auto v2v0 = v2.coord_ - v0.coord_;
    auto v1v0 = v1.coord_ - v0.coord_;
    normal_ = (v2v0.cross(v1v0)).normalized();
  }
}

auto Model::Face::operator*(const Matrix4f &tran) const -> Model::Face {
  auto face(*this);
  face.v0_ = face.v0_ * tran;
  face.v1_ = face.v1_ * tran;
  face.v2_ = face.v2_ * tran;

  /// @todo 通过矩阵变换法线
  auto v2v0 = face.v2_.coord_ - face.v0_.coord_;
  auto v1v0 = face.v1_.coord_ - face.v0_.coord_;
  face.normal_ = (v2v0.cross(v1v0)).normalized();

  return face;
}

Model::Model(const std::string &obj_path, const std::string &mtl_path)
    : obj_path_(obj_path), mtl_path_(mtl_path) {
  SPDLOG_DEBUG(SRLOG, "obj_path: {}", obj_path_);

  tinyobj::ObjReader reader;
  tinyobj::ObjReaderConfig config;
  config.mtl_search_path = mtl_path_;
  // 默认开启三角化
  auto ret = reader.ParseFromFile(obj_path_, config);
  if (!ret) {
    if (!reader.Error().empty()) {
      throw Exception(reader.Error());
    }
  }

  if (!reader.Warning().empty()) {
    SPDLOG_WARN("TinyObjReader {}", reader.Warning());
  }

  const auto &attrib = reader.GetAttrib();
  const auto &shapes = reader.GetShapes();
  const auto &materials = reader.GetMaterials();

  SPDLOG_INFO(
      "加载模型: {}, 顶点数: {}, 法线数: {}, 颜色数: {}, UV数: {}, "
      "子模型数: {}, 材质数: {}",
      obj_path_, attrib.vertices.size() / 3, attrib.normals.size() / 3,
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
      if (num_face_vertices != kTriangleFaceVertexCount) {
        throw Exception("num_face_vertices != kTriangleFaceVertexCount");
      }

      auto vertexes = std::array<Vertex, 3>();
      // 遍历面上的顶点，这里 fv == 3
      for (size_t num_face_vertices_idx = 0;
           num_face_vertices_idx < num_face_vertices; num_face_vertices_idx++) {
        // 获取索引
        auto idx = shapes[shapes_size]
                       .mesh.indices[index_offset + num_face_vertices_idx];

        // 构造顶点信息并保存
        // 每组顶点信息有 xyz 三个分量，因此需要 3*
        auto coord = Coord(attrib.vertices[3 * size_t(idx.vertex_index) + 0],
                           attrib.vertices[3 * size_t(idx.vertex_index) + 1],
                           attrib.vertices[3 * size_t(idx.vertex_index) + 2]);

        // 如果法线索引存在(即 idx.normal_index >= 0)，
        // 则构造并保存，否则设置为 0
        Normal normal = Normal::Zero();
        if (idx.normal_index >= 0) {
          normal = Normal(attrib.normals[3 * size_t(idx.normal_index) + 0],
                          attrib.normals[3 * size_t(idx.normal_index) + 1],
                          attrib.normals[3 * size_t(idx.normal_index) + 2]);
        }

        // 如果贴图索引存在(即 idx.texcoord_index >= 0)，
        // 则构造并保存，否则设置为 0
        TextureCoord texture_coord = TextureCoord::Zero();
        if (idx.texcoord_index >= 0) {
          texture_coord = TextureCoord(
              attrib.texcoords[2 * size_t(idx.texcoord_index) + 0],
              attrib.texcoords[2 * size_t(idx.texcoord_index) + 1]);
        }

        // 顶点颜色，如果 obj 文件中没有指定则设为 1(白色)，范围 [0, 1]
        auto color = Color(attrib.colors[3 * size_t(idx.vertex_index) + 0],
                           attrib.colors[3 * size_t(idx.vertex_index) + 1],
                           attrib.colors[3 * size_t(idx.vertex_index) + 2]);
        vertexes.at(num_face_vertices_idx) =
            Vertex(coord, normal, texture_coord, color);
      }
      index_offset += num_face_vertices;

      // 如果材质不为空，加载材质信息
      auto material = Material();
      if (!materials.empty()) {
        material.shininess = materials[shapes_size].shininess;
        material.ambient = Vector3f(materials[shapes_size].ambient[0],
                                    materials[shapes_size].ambient[1],
                                    materials[shapes_size].ambient[2]);
        material.diffuse = Vector3f(materials[shapes_size].diffuse[0],
                                    materials[shapes_size].diffuse[1],
                                    materials[shapes_size].diffuse[2]);
        material.specular = Vector3f(materials[shapes_size].specular[0],
                                     materials[shapes_size].specular[1],
                                     materials[shapes_size].specular[2]);
      }
      // 添加到 face 中
      faces_.emplace_back(vertexes[0], vertexes[1], vertexes[2], material);
    }
  }

  Normalize();
}

auto Model::operator*(const Matrix4f &tran) const -> Model {
  auto model = Model(*this);

  for (auto &i : model.faces_) {
    i = i * tran;
  }

  return model;
}

auto Model::GetFace() const -> const std::vector<Model::Face> & {
  return faces_;
}

std::pair<Model::Coord, Model::Coord> Model::GetMaxMinXYX() {
  auto max = faces_.at(0).v0_.coord_;
  auto min = faces_.at(0).v0_.coord_;

  for (const auto &i : faces_) {
    auto curr_max_x = std::max(i.v0_.coord_.x(),
                               std::max(i.v1_.coord_.x(), i.v2_.coord_.x()));
    auto curr_max_y = std::max(i.v0_.coord_.y(),
                               std::max(i.v1_.coord_.y(), i.v2_.coord_.y()));
    auto curr_max_z = std::max(i.v0_.coord_.z(),
                               std::max(i.v1_.coord_.z(), i.v2_.coord_.z()));

    max.x() = curr_max_x > max.x() ? curr_max_x : max.x();
    max.y() = curr_max_y > max.y() ? curr_max_y : max.y();
    max.z() = curr_max_z > max.z() ? curr_max_z : max.z();

    auto curr_min_x = std::min(i.v0_.coord_.x(),
                               std::min(i.v1_.coord_.x(), i.v2_.coord_.x()));
    auto curr_min_y = std::min(i.v0_.coord_.y(),
                               std::min(i.v1_.coord_.y(), i.v2_.coord_.y()));
    auto curr_min_z = std::min(i.v0_.coord_.z(),
                               std::min(i.v1_.coord_.z(), i.v2_.coord_.z()));

    min.x() = curr_min_x < min.x() ? curr_min_x : min.x();
    min.y() = curr_min_y < min.y() ? curr_min_y : min.y();
    min.z() = curr_min_z < min.z() ? curr_min_z : min.z();
  }
  return {max, min};
}

void Model::Normalize() {
  auto [max, min] = GetMaxMinXYX();

  auto x = std::abs(max.x()) + std::abs(min.x());
  auto y = std::abs(max.y()) + std::abs(min.y());
  auto z = std::abs(max.z()) + std::abs(min.z());

  auto scale = 1.0f / std::max(x, std::max(y, z));
  auto scale_matrix = Matrix4f(Matrix4f::Identity());
  scale_matrix.diagonal() << scale, scale, scale, 1;

  auto center = Coord((max.x() + min.x()) / 2.f, (max.y() + min.y()) / 2.f,
                      (max.z() + min.z()) / 2.f);
  auto translation_matrix = Matrix4f(Matrix4f::Identity());
  translation_matrix.col(translation_matrix.cols() - 1) << -center.x(),
      -center.y(), -center.z(), 1;

  auto matrix = Matrix4f(scale_matrix * translation_matrix);

  SPDLOG_DEBUG("matrix: {}", matrix);

  *this = *this * matrix;
}

}  // namespace simple_renderer
