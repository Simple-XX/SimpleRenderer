#include "material.hpp"

#include <string>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "log_system.h"



namespace simple_renderer {
Texture Texture::loadTextureFromFile(const std::string& path) {
  Texture texture;

  int width, height, channels;
  unsigned char* data = stbi_load(path.c_str(), &width, &height, &channels, 0);
  if (data) {
    texture.data = data;
    texture.width = width;
    texture.height = height;
    texture.channels = channels;
  } else {
    SPDLOG_ERROR("Failed to load texture: {}", path);
    throw std::runtime_error("Failed to load texture");
  }

  return texture;
}

void Texture::free() {
  if (data) {
    stbi_image_free(data);
    data = nullptr;
  }
}

Color Texture::getPixel(int x, int y) const {
  if (!data) {
    SPDLOG_ERROR("Texture data is nullptr");
    throw std::invalid_argument("Texture data is nullptr");
  }

  int index = (y * width + x) * channels;

  if (channels == 3) {
    return Color(data[index], data[index + 1], data[index + 2]);
  } else if (channels == 4) {
    return Color(data[index], data[index + 1], data[index + 2],
                 data[index + 3]);
  } else {
    SPDLOG_ERROR("Unsupported texture channels: {}", channels);
    throw std::invalid_argument("Unsupported texture channels");
  }
}
}  // namespace simple_renderer