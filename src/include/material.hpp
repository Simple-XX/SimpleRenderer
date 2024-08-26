#ifndef SIMPLERENDER_SRC_INCLUDE_MATERIAL_HPP_
#define SIMPLERENDER_SRC_INCLUDE_MATERIAL_HPP_

#include <math.hpp>

namespace simple_renderer {

class Material {
 public:
  // Default constructor
  // 默认构造函数
  Material() = default;

  // Copy constructor
  // 拷贝构造函数
  Material(const Material& material) = default;

  // Copy assignment operator
  // 拷贝赋值操作符
  Material& operator=(const Material& material) = default;

  // Move constructor
  // 移动构造函数
  Material(Material&& material) = default;

  // Move assignment operator
  // 移动赋值操作符
  Material& operator=(Material&& material) = default;

  // Destructor
  // 析构函数
  ~Material() = default;

  float shininess = 0.0f;
  Vector3f ambient;
  Vector3f diffuse;
  Vector3f specular;
};

}  // namespace simple_renderer

#endif