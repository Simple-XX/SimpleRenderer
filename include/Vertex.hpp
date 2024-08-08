#pragma once

#include <glm/glm.hpp>

#include "color.h"

namespace simple_renderer {

class Vertex {
public:

    // Default constructor
    Vertex() = default;
    // Copy constructor
    Vertex(const Vertex& vertex) = default;
    Vertex& operator=(const Vertex& vertex) = default;
    // Move constructor
    Vertex(Vertex&& vertex) = default;
    Vertex& operator=(Vertex&& vertex) = default;
    // Destructor
    ~Vertex() = default;

    // Constructor with parameters
    explicit Vertex(const glm::vec4& pos, const glm::vec3& norm, const glm::vec2& tex, const Color& color_)
        : position_(pos), normal_(norm), texCoords_(tex), color_(color_) {}

    // Transform the vertex with a matrix
    void transform(const glm::mat4& matrix) {
        position_ = matrix * position_;
    }

    // Perspective divide to convert from clip space to normalized device coordinates
    void perspectiveDivide() {
        if (position_.w != 0) {
            position_.x /= position_.w;
            position_.y /= position_.w;
            position_.z /= position_.w;
            position_.w = 1.0f; // Homogenize
        }
    }

    glm::vec4 position() const { return position_; }
    glm::vec3 normal() const { return normal_; }
    glm::vec2 texCoords() const { return texCoords_; }
    Color color() const { return color_; }
    
private:
    glm::vec4 position_;     // Position of the vertex in homogenous coordinates
    glm::vec3 normal_;       // Normal vector for the vertex
    glm::vec2 texCoords_;    // Texture coordinates
    Color color_;        // Color of the vertex
};

}