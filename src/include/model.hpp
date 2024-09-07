
/**
 * @file model.hpp
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

#ifndef SIMPLERENDER_SRC_INCLUDE_MODEL_HPP_
#define SIMPLERENDER_SRC_INCLUDE_MODEL_HPP_

#include <assimp/scene.h>

#include <string>
#include <vector>

#include "color.h"
#include "config.h"
#include "face.hpp"
#include "log_system.h"
#include "math.hpp"
#include "vertex.hpp"

namespace simple_renderer {

/* * * * * * * * * */
/* --- Model --- */
class Model {
 public:
  // Default constructor
  // 默认构造函数
  Model() = default;

  // Default copy constructor
  // 默认拷贝构造函数
  Model(const Model& model) = default;
  // Default copy assignment operator
  // 默认拷贝赋值运算符
  Model& operator=(const Model& model) = default;

  // Default move constructor
  // 默认移动构造函数
  Model(Model&& model) = default;
  // Default move assignment operator
  // 默认移动赋值运算符
  Model& operator=(Model&& model) = default;

  // Destructor
  // 析构函数
  ~Model() = default;

  // Constructor that loads a model from a file path
  // 从文件路径加载模型的构造函数
  Model(const std::string& model_path);

  // Get functions
  // 获取函数
  const std::vector<Vertex>& vertices() const { return vertices_; };
  const std::vector<Face>& faces() const { return faces_; };
  const std::string& modelPath() const { return directory_; };

 private:
  // Number of vertices per triangle face
  // 每个三角形面的顶点数
  static constexpr const uint8_t kTriangleFaceVertexCount = 3;
  // Directory where the model is located
  // 模型所在的目录
  std::string directory_;
  // List of faces(triangles) that make up the model
  // 构成模型的面(三角形）列表
  std::vector<Face> faces_;

  // Load the model from the specified file path
  // 从指定的文件路径加载模型
  void loadModel(const std::string& path);

  // Process a node in the model
  // 处理模型中的一个节点
  void processNode(aiNode* node, const aiScene* scene);

  // Process a mesh in the model
  // 处理模型中的一个网格
  void processMesh(aiMesh* mesh, const aiScene* scene);

  // Process the material of the model
  // 处理模型的材质
  Material processMaterial(aiMaterial* material);

  // Texture cache
  // 纹理缓存
  std::unordered_map<std::string, Texture> texture_cache_;
};
}  // namespace simple_renderer

#endif /* SIMPLERENDER_SRC_INCLUDE_MODEL_HPP_ */