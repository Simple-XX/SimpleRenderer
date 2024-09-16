
/**
 * @file camera.h
 * @brief camera 抽象
 * @author Zone.N (Zone.Niuzh@hotmail.com)
 * @version 1.0
 * @date 2024-04-29
 * @copyright MIT LICENSE
 * https://github.com/Simple-XX/SimpleRenderer
 * @par change log:
 * <table>
 * <tr><th>Date<th>Author<th>Description
 * <tr><td>2024-04-29<td>Zone.N<td>创建文件
 * </table>
 */

#ifndef SIMPLERENDER_SRC_INCLUDE_CAMERA_H_
#define SIMPLERENDER_SRC_INCLUDE_CAMERA_H_

#include <cstdint>
#include <string>

#include "color.h"
#include "math.hpp"

namespace simple_renderer {

#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <string>

class Camera {
 public:
  Camera(Vector3f position)
      : position_(position), front_(0.0f, 0.0f, -1.0f), up_(0.0f, 1.0f, 0.0f) {}

  Camera(const Camera& camera) = default;
  Camera(Camera&& camera) = default;
  auto operator=(const Camera& camera) -> Camera& = default;
  auto operator=(Camera&& camera) -> Camera& = default;
  ~Camera() = default;

  // Getters and Setters
  void SetPosition(const glm::vec3& pos) { position_ = pos; }
  void SetFront(const glm::vec3& frontVec) {
    front_ = glm::normalize(frontVec);
  }
  void SetUp(const glm::vec3& upVec) { up_ = glm::normalize(upVec); }

  glm::vec3 GetPosition() const { return position_; }
  glm::vec3 GetFront() const { return front_; }
  glm::vec3 GetUp() const { return up_; }

  // Methods to get view and projection matrices
  glm::mat4 GetViewMatrix() const {
    return glm::lookAt(position_, position_ + front_, up_);
  }

  glm::mat4 GetProjectionMatrix(float fov, float aspectRatio, float nearPlane,
                                float farPlane) const {
    return glm::perspective(glm::radians(fov), aspectRatio, nearPlane,
                            farPlane);
  }

  // Additional methods for camera movement (optional)
  void MoveForward(float distance) { position_ += front_ * distance; }

  void MoveRight(float distance) {
    position_ += glm::normalize(glm::cross(front_, up_)) * distance;
  }

  void MoveUp(float distance) { position_ += up_ * distance; }

  void Rotate(float yaw, float pitch) {
    glm::vec3 newFront;
    newFront.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    newFront.y = sin(glm::radians(pitch));
    newFront.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    front_ = glm::normalize(newFront);
  }

 private:
  Vector3f position_;
  Vector3f front_;
  Vector3f up_;
};

}  // namespace simple_renderer

#endif /* SIMPLERENDER_SRC_INCLUDE_CAMERA_H_ */
