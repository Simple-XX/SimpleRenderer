
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
#include <glm/gtc/matrix_transform.hpp>

#include <utility>

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

#include "log_system.h"

namespace simple_renderer {

Vertex::Vertex(glm::vec3 coord, glm::vec3 normal, glm::vec2 texture_coord, const Color &color)
    : coord_(std::move(coord)), normal_(std::move(normal)), texture_coord_(std::move(texture_coord)), color_(color) {}

Vertex Vertex::operator*(const glm::mat4 &tran) const {
    glm::vec4 transformed_coord = tran * glm::vec4(coord_, 1.0f);
    return Vertex(glm::vec3(transformed_coord), normal_, texture_coord_, color_);
}

Face::Face(const Vertex &v0, const Vertex &v1, const Vertex &v2, Material material)
    : v0_(v0), v1_(v1), v2_(v2), material_(std::move(material)) {

    if (glm::length(v0_.normal_) > 0.0f && glm::length(v1_.normal_) > 0.0f && glm::length(v2_.normal_) > 0.0f) {
        normal_ = glm::normalize(v0_.normal_ + v1_.normal_ + v2_.normal_);
    } else {
        normal_ = glm::normalize(glm::cross(v2_.coord_ - v0_.coord_, v1_.coord_ - v0_.coord_));
    }
}

Face Face::operator*(const glm::mat4 &tran) const {
    Face face = *this;
    face.v0_ = face.v0_ * tran;
    face.v1_ = face.v1_ * tran;
    face.v2_ = face.v2_ * tran;
    face.normal_ = glm::normalize(glm::cross(face.v2_.coord_ - face.v0_.coord_,
                                        face.v1_.coord_ - face.v0_.coord_));
    return face;
}

Model::Model(const std::string &model_path)
    : model_path_(model_path) {
    LoadModel(model_path_);
}

void Model::LoadModel(const std::string& path) {
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenNormals);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        SPDLOG_ERROR("Assimp Error: {}", importer.GetErrorString());
        throw std::runtime_error("Failed to load model with Assimp");
    }

    SPDLOG_INFO("Loaded model: {}, with meshes: {}, materials: {}", path, scene->mNumMeshes, scene->mNumMaterials);

    ProcessNode(scene->mRootNode, scene);

    NormalizeModel();
}

void Model::ProcessNode(aiNode* node, const aiScene* scene) {
    for (unsigned int i = 0; i < node->mNumMeshes; i++) {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        Face face = ProcessMesh(mesh, scene);
        faces_.push_back(face);
    }

    for (unsigned int i = 0; i < node->mNumChildren; i++) {
        ProcessNode(node->mChildren[i], scene);
    }
}

Face Model::ProcessMesh(aiMesh* mesh, const aiScene* scene) {
    std::vector<Vertex> vertices;
    for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
        glm::vec3 coord(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);
        glm::vec3 normal = mesh->HasNormals() ? glm::vec3(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z) : glm::vec3(0.0f);
        glm::vec2 texCoord = mesh->mTextureCoords[0] ? glm::vec2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y) : glm::vec2(0.0f);
        Color color = mesh->HasVertexColors(0) ? Color(mesh->mColors[0][i].r, mesh->mColors[0][i].g, mesh->mColors[0][i].b) : Color(1.0f);

        vertices.emplace_back(coord, normal, texCoord, color);
    }

    Material material;
    if (mesh->mMaterialIndex >= 0) {
        aiMaterial* aiMaterial = scene->mMaterials[mesh->mMaterialIndex];
        aiColor3D ambient(0.0f, 0.0f, 0.0f);
        aiColor3D diffuse(0.0f, 0.0f, 0.0f);
        aiColor3D specular(0.0f, 0.0f, 0.0f);
        float shininess;

        aiMaterial->Get(AI_MATKEY_COLOR_AMBIENT, ambient);
        aiMaterial->Get(AI_MATKEY_COLOR_DIFFUSE, diffuse);
        aiMaterial->Get(AI_MATKEY_COLOR_SPECULAR, specular);
        aiMaterial->Get(AI_MATKEY_SHININESS, shininess);

        material.ambient = glm::vec3(ambient.r, ambient.g, ambient.b);
        material.diffuse = glm::vec3(diffuse.r, diffuse.g, diffuse.b);
        material.specular = glm::vec3(specular.r, specular.g, specular.b);
        material.shininess = shininess;
    }

    return Face(vertices[0], vertices[1], vertices[2], material); // Assumes a triangular face
}

Model Model::operator*(const glm::mat4 &tran) const {
    auto model = *this;
    for (auto &face : model.faces_) {
        face = face * tran;
    }
    return model;
}

const std::vector<Face>& Model::GetFaces() const {
    return faces_;
}

std::pair<glm::vec3, glm::vec3> Model::GetMaxMinXYZ() const {
    glm::vec3 max_coords(std::numeric_limits<float>::lowest());
    glm::vec3 min_coords(std::numeric_limits<float>::max());

    for (const auto &face : faces_) {
        for (const auto &vertex : {face.v0_, face.v1_, face.v2_}) {
            max_coords = glm::max(max_coords, vertex.coord_);
            min_coords = glm::min(min_coords, vertex.coord_);
        }
    }
    return {max_coords, min_coords};
}

void Model::NormalizeModel() {
    auto [max, min] = GetMaxMinXYZ();
    glm::vec3 scale_factors = max - min;
    float scale_factor = 1.0f / glm::max(scale_factors.x, glm::max(scale_factors.y, scale_factors.z));

    glm::mat4 scale_matrix = glm::scale(glm::mat4(1.0f), glm::vec3(scale_factor));
    glm::vec3 center = (max + min) * 0.5f;
    glm::mat4 translation_matrix = glm::translate(glm::mat4(1.0f), -center);

    glm::mat4 normalization_matrix = scale_matrix * translation_matrix;
    SPDLOG_DEBUG("Normalization matrix: \n{}", glm::to_string(normalization_matrix));

    *this = *this * normalization_matrix;
}

const std::string Model::ModelPath() const {
    return model_path_;
}

}  // namespace simple_renderer
