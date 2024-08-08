
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
#include "log_math.hpp"

#include "vertex.hpp"
#include "face.hpp"

namespace simple_renderer {

class Model {
public:
    
    // Default constructor
    Model() = default;
    // Default copy constructor
    Model(const Model& model) = default;
    Model& operator=(const Model& model) = default;
    // Default move constructor
    Model(Model&& model) = default;
    Model& operator=(Model&& model) = default;
    ~Model() = default;

    Model(const std::string &model_path);

    void transform(const glm::mat4 &tran);
    
    const std::vector<Face>& faces() const { return faces_; };
    const std::string& modelPath() const { return directory_; };

private:
    static constexpr const uint8_t kTriangleFaceVertexCount = 3;
    std::string directory_;

    std::vector<Face> faces_;

    void loadModel(const std::string& path);
    void processNode(aiNode* node, const aiScene* scene);
    void processMesh(aiMesh* mesh, const aiScene* scene);
    Material processMaterial(aiMaterial* material);
};
}  // namespace simple_renderer

#endif /* SIMPLERENDER_SRC_INCLUDE_MODEL_HPP_ */