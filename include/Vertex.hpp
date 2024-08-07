#pragma once

#include <glm/glm.hpp>

struct Vertex {
    glm::vec4 Position;     // Position of the vertex in homogenous coordinates
    glm::vec3 Normal;       // Normal vector for the vertex
    glm::vec2 TexCoords;    // Texture coordinates
    glm::vec3 Color;        // Color of the vertex

    // Constructor with parameters
    Vertex(const glm::vec4& pos, const glm::vec3& norm, const glm::vec2& tex, const glm::vec3& color = glm::vec3(1.0f, 1.0f, 1.0f))
        : Position(pos), Normal(norm), TexCoords(tex), Color(color) {}

    // Transform the vertex with a matrix
    Vertex transform(const glm::mat4& matrix) const {
        return Vertex(matrix * Position, Normal, TexCoords, Color);
    }

    // Perspective divide to convert from clip space to normalized device coordinates
    void perspectiveDivide() {
        if (Position.w != 0) {
            Position.x /= Position.w;
            Position.y /= Position.w;
            Position.z /= Position.w;
            Position.w = 1.0f; // Homogenize
        }
    }
};