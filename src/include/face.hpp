#pragma once

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
  explicit Face(const Vertex& v0, const Vertex& v1, const Vertex& v2,
                Material material)
      : vertices_{v0, v1, v2}, material_(std::move(material)) {
    // Calculate the normal vector when the face is created
    // 创建 Face 时计算法向量
    calculateNormal();
  }

  // Apply a transformation matrix to the vertices
  // 对顶点应用变换矩阵
  void transform(const Matrix4f& tran) {
    vertices_[0].transform(tran);
    vertices_[1].transform(tran);
    vertices_[2].transform(tran);
  }

  // Get functions
  // 获取函数
  const std::array<Vertex, 3>& vertices() const { return vertices_; }
  const Vertex& vertex(size_t index) const { return vertices_[index]; }
  const Vector3f& normal() const { return normal_; }
  const Material& material() const { return material_; }

 private:
  std::array<Vertex, 3> vertices_;
  Vector3f normal_;
  Material material_;

  // Calculate the normal vector based on the vertices
  // 根据顶点计算法向量
  void calculateNormal() {
    Vector3f edge1 =
        Vector3f(vertices_[1].position()) - Vector3f(vertices_[0].position());
    Vector3f edge2 =
        Vector3f(vertices_[2].position()) - Vector3f(vertices_[0].position());
    normal_ = glm::normalize(
        // Normalize the cross product to get the
        // normal 归一化叉积以获得法向量
        glm::cross(edge1, edge2));
  }
};

}  // namespace simple_renderer
