#ifndef SIMPLERENDER_SRC_INCLUDE_MATERIAL_HPP_
#define SIMPLERENDER_SRC_INCLUDE_MATERIAL_HPP_

#include "color.h"
#include "math.hpp"

namespace simple_renderer {

/* * * * * * * * * */
/* --- Texture --- */
class Texture {
 public:
  // Default constructor
  // 默认构造函数
  Texture() = default;

  // Copy constructor
  // 拷贝构造函数
  Texture(const Texture& texture) = default;

  // Copy assignment operator
  // 拷贝赋值操作符
  Texture& operator=(const Texture& texture) = default;

  // Move constructor
  // 移动构造函数
  Texture(Texture&& texture) = default;

  // Move assignment operator
  // 移动赋值操作符
  Texture& operator=(Texture&& texture) = default;

  // Destructor
  // 析构函数
  ~Texture() = default;

  // Load a texture from a file
  // 从文件加载纹理
  static Texture LoadTextureFromFile(const std::string& path);

  // Free the texture data
  // 释放纹理数据
  void free();

  // get pixel from texture
  // 从纹理获取像素
  Color GetPixel(int x, int y) const;

  // Texture data
  // 纹理数据
  uint8_t* data = nullptr;
  // Texture channels
  // 纹理通道数
  int channels = 0;
  // Texture width
  // 纹理宽度
  int width = 0;
  // Texture height
  // 纹理高度
  int height = 0;
};

/* * * * * * * * * */
/* ---  Material --- */
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

  // Texture
  // 纹理
  Texture ambient_texture;
  Texture diffuse_texture;
  Texture specular_texture;

  bool has_ambient_texture = false;
  bool has_diffuse_texture = false;
  bool has_specular_texture = false;
};
}  // namespace simple_renderer

#endif /* SIMPLERENDER_SRC_INCLUDE_MATERIAL_HPP_ */