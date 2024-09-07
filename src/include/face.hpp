#ifndef SIMPLERENDER_SRC_INCLUDE_FACE_HPP_
#define SIMPLERENDER_SRC_INCLUDE_FACE_HPP_

#include <array>

#include "material.hpp"
#include "vertex.hpp"

namespace simple_renderer {

class Face {
 public:
  // Default constructor
  // 默认构造函数
  Face() = default;

  // Copy constructor
  // 拷贝构造函数
  Face(const Face& face) = default;
  // Copy assignment operator
  // 拷贝赋值操作符
  Face& operator=(const Face& face) = default;

  // Move constructor
  // 移动构造函数
  Face(Face&& face) = default;
  // Move assignment operator
  // 移动赋值操作符
  Face& operator=(Face&& face) = default;

  // Destructor
  // 析构函数
  ~Face() = default;

  // Constructor that initializes the Face with three vertices and a material
  // 使用三个顶点和材质初始化 Face 的构造函数
  explicit Face(size_t v0, size_t v1, size_t v2, const Material material)
      : indices_{v0, v1, v2}, material_(std::move(material)) {}

  // Get functions
  // 获取函数
  const std::array<size_t, 3>& indices() const { return indices_; }
  const size_t index(size_t index) const { return indices_[index]; }
  const Vector3f& normal() const { return normal_; }
  const Material& material() const { return material_; }

  // Calculate the normal vector based on the vertices
  // 根据顶点计算法向量
  void calculateNormal(const std::vector<Vertex>& vertices) {
    Vector3f edge1 = Vector3f(vertices[indices_[1]].position()) -
                     Vector3f(vertices[indices_[0]].position());
    Vector3f edge2 = Vector3f(vertices[indices_[2]].position()) -
                     Vector3f(vertices[indices_[0]].position());
    normal_ = glm::normalize(
        // Normalize the cross product to get the
        // normal 归一化叉积以获得法向量
        glm::cross(edge1, edge2));
  }

 private:
  std::array<size_t, 3> indices_;
  Vector3f normal_;
  Material material_;
};
}  // namespace simple_renderer

#endif