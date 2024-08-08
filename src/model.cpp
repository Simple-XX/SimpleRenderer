
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

#include "log_system.h"

namespace simple_renderer {


Model::Model(const std::string &model_path) {
    loadModel(model_path);
}

void Model::loadModel(const std::string& path) {
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        SPDLOG_ERROR("Assimp Error: {}", importer.GetErrorString());
        throw std::runtime_error("Failed to load model with Assimp");
    }

    directory_ = path.substr(0, path.find_last_of('/'));

    SPDLOG_INFO("Loaded model path: {}, Directory: {}, with meshes: {}, materials: {}", path, directory_, scene->mNumMeshes, scene->mNumMaterials);

    processNode(scene->mRootNode, scene);
}

void Model::processNode(aiNode* node, const aiScene* scene) {
    for (unsigned int i = 0; i < node->mNumMeshes; i++) {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        processMesh(mesh, scene);
    }

    for (unsigned int i = 0; i < node->mNumChildren; i++) {
        processNode(node->mChildren[i], scene);
    }
}

void Model::processMesh(aiMesh* mesh, const aiScene* scene) {
    std::vector<Vertex> vertices;

    // Process vertices
    for (unsigned int i = 0; i < mesh->mNumVertices; ++i) {
        glm::vec3 position(
            mesh->mVertices[i].x,
            mesh->mVertices[i].y,
            mesh->mVertices[i].z
        );
        glm::vec3 normal(
            mesh->mNormals[i].x,
            mesh->mNormals[i].y,
            mesh->mNormals[i].z
        );
        glm::vec2 texCoords(0.0f, 0.0f);
        if (mesh->mTextureCoords[0]) { // Check if the mesh contains texture coordinates
            texCoords = glm::vec2(
                mesh->mTextureCoords[0][i].x,
                mesh->mTextureCoords[0][i].y
            );
        }

        Color color(255.f, 255.f, 255.f, 255.f);

        vertices.emplace_back(glm::vec4(position, 1.0f), normal, texCoords, color);
    }
    // Process faces (assuming triangulation, so each face has 3 vertices)
    for (unsigned int i = 0; i < mesh->mNumFaces; ++i) {
        aiFace face = mesh->mFaces[i];
        if (face.mNumIndices == 3) {  // Ensure we are working with triangles
            Vertex v0 = vertices[face.mIndices[0]];
            Vertex v1 = vertices[face.mIndices[1]];
            Vertex v2 = vertices[face.mIndices[2]];

            Material material = processMaterial(scene->mMaterials[mesh->mMaterialIndex]);

            faces_.emplace_back(v0, v1, v2, std::move(material));
        }
    }
}

Material Model::processMaterial(aiMaterial* mat) {
    Material material;

    // Extract ambient color
    aiColor3D ambient(0.0f, 0.0f, 0.0f);
    if (AI_SUCCESS == mat->Get(AI_MATKEY_COLOR_AMBIENT, ambient)) {
        material.ambient = glm::vec3(ambient.r, ambient.g, ambient.b);
    }

    // Extract diffuse color
    aiColor3D diffuse(1.0f, 1.0f, 1.0f); // Default to white
    if (AI_SUCCESS == mat->Get(AI_MATKEY_COLOR_DIFFUSE, diffuse)) {
        material.diffuse = glm::vec3(diffuse.r, diffuse.g, diffuse.b);
    }

    // Extract specular color
    aiColor3D specular(0.0f, 0.0f, 0.0f);
    if (AI_SUCCESS == mat->Get(AI_MATKEY_COLOR_SPECULAR, specular)) {
        material.specular = glm::vec3(specular.r, specular.g, specular.b);
    }

    // Extract shininess
    float shininess = 0.0f;
    if (AI_SUCCESS == mat->Get(AI_MATKEY_SHININESS, shininess)) {
        material.shininess = shininess;
    }

    return material;
}

void Model::transform(const glm::mat4 &tran) {
    for (auto& face : faces_) {
        face.transform(tran);
    }
}

}  // namespace simple_renderer
