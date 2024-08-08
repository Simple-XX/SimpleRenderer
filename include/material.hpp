#pragma once

#include <glm/glm.hpp>

class Material {
public:
    // Default constructor
    Material() = default;
    // Copy constructor
    Material(const Material& material) = default;
    Material& operator=(const Material& material) = default;
    // Move constructor
    Material(Material&& material) = default;
    Material& operator=(Material&& material) = default;
    // Destructor
    ~Material() = default;

    float shininess = 0.0f;
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;
};