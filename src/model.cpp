/**
 * @file model.cpp
 * @brief Model abstraction (模型抽象)
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

#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include <assimp/Importer.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <string>
#include <utility>

#include "log_system.h"

namespace simple_renderer {

// Constructor that loads a model from a file path
// 构造函数从文件路径加载模型
Model::Model(const std::string& model_path) { loadModel(model_path); }

// Load the model using Assimp and process its nodes and meshes
// 使用 Assimp 加载模型并处理其节点和网格
void Model::loadModel(const std::string& path) {
  Assimp::Importer importer;
  const aiScene* scene =
      importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);

  // Check for errors in loading the model
  // 检查加载模型时的错误
  if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE ||
      !scene->mRootNode) {
    SPDLOG_ERROR("Assimp Error: {}", importer.GetErrorString());
    throw std::runtime_error("Failed to load model with Assimp");
  }

  // Store the directory path of the model
  // 存储模型的目录路径
  directory_ = path.substr(0, path.find_last_of('/'));

  SPDLOG_INFO(
      "Loaded model path: {}, Directory: {}, with meshes: {}, materials: {}",
      path, directory_, scene->mNumMeshes, scene->mNumMaterials);

  // Process the root node recursively
  // 递归处理根节点
  processNode(scene->mRootNode, scene);
}

// Recursively process nodes in the model
// 递归处理模型中的节点
void Model::processNode(aiNode* node, const aiScene* scene) {
  // Process each mesh in the node
  // 处理节点中的每个网格
  for (unsigned int i = 0; i < node->mNumMeshes; i++) {
    aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
    processMesh(mesh, scene);
  }

  // Recursively process each child node
  // 递归处理每个子节点
  for (unsigned int i = 0; i < node->mNumChildren; i++) {
    processNode(node->mChildren[i], scene);
  }
}

// Process a single mesh and extract vertices, normals, and faces
// 处理单个网格并提取顶点、法线和面
void Model::processMesh(aiMesh* mesh, const aiScene* scene) {
  std::vector<Vertex> vertices;

  // Process vertices
  // 处理顶点
  for (unsigned int i = 0; i < mesh->mNumVertices; ++i) {
    Vector3f position(mesh->mVertices[i].x, mesh->mVertices[i].y,
                      mesh->mVertices[i].z);
    Vector3f normal(mesh->mNormals[i].x, mesh->mNormals[i].y,
                    mesh->mNormals[i].z);
    Vector2f texCoords(0.0f, 0.0f);
    // Check if the mesh has texture coordinates
    // 检查网格是否有纹理坐标
    if (mesh->mTextureCoords[0]) {
      texCoords =
          Vector2f(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y);
    }

    Color color(255.f, 255.f, 255.f, 255.f);  // Default color (white)
                                              // 默认颜色（白色）

    vertices.emplace_back(Vector4f(position, 1.0f), normal, texCoords, color);
  }

  // Process faces (assuming triangulation, so each face has 3 vertices)
  // 处理面（假设三角化，因此每个面有3个顶点）
  for (unsigned int i = 0; i < mesh->mNumFaces; ++i) {
    aiFace face = mesh->mFaces[i];
    if (face.mNumIndices == 3) {  // Triangle, 三角形
      Vertex v0 = vertices[face.mIndices[0]];
      Vertex v1 = vertices[face.mIndices[1]];
      Vertex v2 = vertices[face.mIndices[2]];

      // Process the material associated with this mesh
      // 处理与此网格关联的材质
      Material material =
          processMaterial(scene->mMaterials[mesh->mMaterialIndex]);

      // Create a Face object and store it
      // 创建一个 Face 对象并存储它
      faces_.emplace_back(v0, v1, v2, std::move(material));
    }
  }
}

// Extract material properties from the Assimp material structure
// 从 Assimp 材质结构中提取材质属性
Material Model::processMaterial(aiMaterial* mat) {
  Material material;

  aiColor3D ambient(0.0f, 0.0f, 0.0f);
  if (AI_SUCCESS == mat->Get(AI_MATKEY_COLOR_AMBIENT, ambient)) {
    material.ambient = Vector3f(ambient.r, ambient.g, ambient.b);
  }

  aiColor3D diffuse(1.0f, 1.0f, 1.0f);
  if (AI_SUCCESS == mat->Get(AI_MATKEY_COLOR_DIFFUSE, diffuse)) {
    material.diffuse = Vector3f(diffuse.r, diffuse.g, diffuse.b);
  }

  aiColor3D specular(0.0f, 0.0f, 0.0f);
  if (AI_SUCCESS == mat->Get(AI_MATKEY_COLOR_SPECULAR, specular)) {
    material.specular = Vector3f(specular.r, specular.g, specular.b);
  }

  float shininess = 0.0f;
  if (AI_SUCCESS == mat->Get(AI_MATKEY_SHININESS, shininess)) {
    material.shininess = shininess;
  }

  // load textures
  // 加载纹理
  if (mat->GetTextureCount(aiTextureType_AMBIENT) > 0) {
    aiString texture_path;
    mat->GetTexture(aiTextureType_AMBIENT, 0, &texture_path);
    std::string fullpath = directory_ + "/" + texture_path.C_Str();
    if (texture_cache_.find(fullpath) != texture_cache_.end()) {
      // load from cache
      // 从缓存加载
      material.ambient_texture = texture_cache_[fullpath];
    } else {
      // first time loading
      // 第一次加载纹理
      material.ambient_texture = Texture::loadTextureFromFile(fullpath);
      texture_cache_[fullpath] = material.ambient_texture;
    }
    material.has_ambient_texture = true;
  }

  if (mat->GetTextureCount(aiTextureType_DIFFUSE) > 0) {
    aiString texture_path;
    mat->GetTexture(aiTextureType_DIFFUSE, 0, &texture_path);
    std::string fullpath = directory_ + "/" + texture_path.C_Str();
    if (texture_cache_.find(fullpath) != texture_cache_.end()) {
      // load from cache
      // 从缓存加载
      material.diffuse_texture = texture_cache_[fullpath];
    } else {
      // first time loading
      // 第一次加载纹理
      material.diffuse_texture = Texture::loadTextureFromFile(fullpath);
      texture_cache_[fullpath] = material.diffuse_texture;
    }
    material.has_diffuse_texture = true;
  }

  if (mat->GetTextureCount(aiTextureType_SPECULAR) > 0) {
    aiString texture_path;
    mat->GetTexture(aiTextureType_SPECULAR, 0, &texture_path);
    std::string fullpath = directory_ + "/" + texture_path.C_Str();
    if (texture_cache_.find(fullpath) != texture_cache_.end()) {
      // load from cache
      // 从缓存加载
      material.specular_texture = texture_cache_[fullpath];
    } else {
      // first time loading
      // 第一次加载纹理
      material.specular_texture = Texture::loadTextureFromFile(fullpath);
      texture_cache_[fullpath] = material.specular_texture;
    }
    material.has_specular_texture = true;
  }

  return material;
}

// Apply a transformation matrix to all faces in the model
// 对模型中的所有面应用变换矩阵
void Model::transform(const Matrix4f& tran) {
  for (auto& face : faces_) {
    face.transform(tran);
  }
}
}  // namespace simple_renderer
