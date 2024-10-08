#ifndef SIMPLERENDER_SRC_INCLUDE_VERTEX_HPP_
#define SIMPLERENDER_SRC_INCLUDE_VERTEX_HPP_

#include <math.hpp>

#include "color.h"

namespace simple_renderer {

class Vertex {
 public:
  // Default constructor
  // 默认构造函数
  Vertex() = default;

  // Copy constructor
  // 拷贝构造函数
  Vertex(const Vertex& vertex) = default;
  // Copy assignment operator
  // 拷贝赋值操作符
  Vertex& operator=(const Vertex& vertex) = default;

  // Move constructor
  // 移动构造函数
  Vertex(Vertex&& vertex) = default;
  // Move assignment operator
  // 移动赋值操作符
  Vertex& operator=(Vertex&& vertex) = default;

  // Destructor
  // 析构函数
  ~Vertex() = default;

  // Constructor with parameters 带参数的构造函数
  explicit Vertex(const Vector4f& pos, const Vector3f& norm,
                  const Vector2f& tex, const Color& color_)
      : position_(pos), normal_(norm), texCoords_(tex), color_(color_) {}

  // Transform the vertex with a matrix     使用矩阵变换顶点
  void transform(const Matrix4f& matrix) { position_ = matrix * position_; }

  // Getter functions
  // 获取函数
  [[nodiscard]] inline Vector4f GetPosition() const { return position_; }
  [[nodiscard]] inline Vector3f GetNormal() const { return normal_; }
  [[nodiscard]] inline Vector2f GetTexCoords() const { return texCoords_; }
  [[nodiscard]] inline Color GetColor() const { return color_; }

 private:
  Vector4f position_;   // 3D position, 3D顶点坐标
  Vector3f normal_;     // Normal vector, 顶点法向量
  Vector2f texCoords_;  // Texture coordinates, 顶点纹理坐标
  Color color_;
};

inline Vertex operator*(const Matrix4f& matrix, const Vertex& vertex) {
  return Vertex(matrix * vertex.GetPosition(),
                Matrix3f(matrix) * vertex.GetNormal(), vertex.GetTexCoords(),
                vertex.GetColor());
}

}  // namespace simple_renderer

#endif