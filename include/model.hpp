
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
#include "log_system.h"
#include "matrix.hpp"
#include "vector.hpp"

namespace simple_renderer {

class Material {
public:
    Material() = default;
    Material(const Material& material) = default;
    Material(Material&& material) = default;
    auto operator=(const Material& material) -> Material& = default;
    auto operator=(Material&& material) -> Material& = default;
    ~Material() = default;

    float shininess = 0.0f;
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;
};

class Vertex {
public:
    explicit Vertex(glm::vec3 coord, glm::vec3 normal, glm::vec2 texture_coord, const Color& color);
    Vertex() = default;
    Vertex(const Vertex& vertex) = default;
    Vertex(Vertex&& vertex) = default;
    auto operator=(const Vertex& vertex) -> Vertex& = default;
    auto operator=(Vertex&& vertex) -> Vertex& = default;
    ~Vertex() = default;

    [[nodiscard]] Vertex operator*(const glm::mat4 &tran) const;

    glm::vec3 coord_;
    glm::vec3 normal_;
    glm::vec2 texture_coord_;
    Color color_;
};

class Face {
public:
    explicit Face(const Vertex& v0, const Vertex& v1, const Vertex& v2, Material material);
    Face() = default;
    Face(const Face& face) = default;
    Face(Face&& face) = default;
    auto operator=(const Face& face) -> Face& = default;
    auto operator=(Face&& face) -> Face& = default;
    ~Face() = default;

    [[nodiscard]] Face operator*(const glm::mat4 &tran) const;

    Vertex v0_;
    Vertex v1_;
    Vertex v2_;
    glm::vec3 normal_;
    Material material_;
};

class Model {
public:
    Model(const std::string &model_path);
    Model() = default;
    Model(const Model& model) = default;
    Model(Model&& model) = default;
    auto operator=(const Model& model) -> Model& = default;
    auto operator=(Model&& model) -> Model& = default;
    ~Model() = default;

    [[nodiscard]] Model operator*(const glm::mat4 &tran) const;
    
    const std::vector<Face>& GetFaces() const;
    const std::string ModelPath() const;

private:
    static constexpr const uint8_t kTriangleFaceVertexCount = 3;

    std::vector<Face> faces_;
    std::string model_path_ = "";

    std::pair<glm::vec3, glm::vec3> GetMaxMinXYZ() const;
    
    void NormalizeModel();
    void LoadModel(const std::string& path);
    void ProcessNode(aiNode* node, const aiScene* scene);
    Face ProcessMesh(aiMesh* mesh, const aiScene* scene);
};
}  // namespace simple_renderer

#endif /* SIMPLERENDER_SRC_INCLUDE_MODEL_HPP_ */