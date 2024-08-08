#pragma once

#include "vertex.hpp"
#include "material.hpp" 

#include <array>

namespace simple_renderer {

class Face {
public:
    // Default constructor
    Face() = default;
    // Copy constructor
    Face(const Face& face) = default;
    Face& operator=(const Face& face) = default;
    // Move constructor
    Face(Face&& face) = default;
    Face& operator=(Face&& face) = default;
    // Destructor
    ~Face() = default;

    explicit Face(const Vertex& v0, const Vertex& v1, const Vertex& v2, Material material)
        : vertices_{v0, v1, v2}, material_(std::move(material)) {
        calculateNormal();
    }

    void transform(const glm::mat4 &tran) {
        vertices_[0].transform(tran);
        vertices_[1].transform(tran);
        vertices_[2].transform(tran);
    }

    const std::array<Vertex, 3>& vertices() const { return vertices_; }
    const Vertex& vertex(size_t index) const { return vertices_[index]; }
    const glm::vec3& normal() const { return normal_; }
    const Material& material() const { return material_; }

private:
    std::array<Vertex, 3> vertices_;
    glm::vec3 normal_;
    Material material_;

    void calculateNormal() {
        glm::vec3 edge1 = glm::vec3(vertices_[1].position()) - glm::vec3(vertices_[0].position());
        glm::vec3 edge2 = glm::vec3(vertices_[2].position()) - glm::vec3(vertices_[0].position());
        normal_ = glm::normalize(glm::cross(edge1, edge2));
    }
};

}