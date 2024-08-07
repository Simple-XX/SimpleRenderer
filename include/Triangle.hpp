#pragma once
#include "Vertex.hpp"

#include <glm/glm.hpp>
#include <array>

class Triangle {
public:
    std::array<Vertex, 3> vertices;  // Holds the three vertices of the triangle

    Triangle(const Vertex& v0, const Vertex& v1, const Vertex& v2)
        : vertices{v0, v1, v2} {}

    // Calculate the normal of the triangle using the cross product
    glm::vec3 normal() const {
        glm::vec3 edge1 = vertices[1].Position - vertices[0].Position;
        glm::vec3 edge2 = vertices[2].Position - vertices[0].Position;
        return glm::normalize(glm::cross(edge1, edge2));
    }
};
