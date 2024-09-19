
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
  void SetPosition(const Vector3f& pos) { position_ = pos; }
  void SetFront(const Vector3f& frontVec) { front_ = glm::normalize(frontVec); }
  void SetUp(const Vector3f& upVec) { up_ = glm::normalize(upVec); }

  Vector3f GetPosition() const { return position_; }
  Vector3f GetFront() const { return front_; }
  Vector3f GetUp() const { return up_; }

  Matrix4f LookAt(const Vector3f& position, const Vector3f& target,
                  const Vector3f up) {
    Matrix4f rotationMat = Matrix4f(1.0f);
    Matrix4f translationMat = Matrix4f(1.0f);
    Vector3f z = glm::normalize(position - target);
    Vector3f x = glm::normalize(glm::cross(up, z));
    Vector3f y = glm::normalize(glm::cross(z, x));
    rotationMat[0][0] = x.x;
    rotationMat[1][0] = x.y;
    rotationMat[2][0] = x.z;
    rotationMat[0][1] = y.x;
    rotationMat[1][1] = y.y;
    rotationMat[2][1] = y.z;
    rotationMat[0][2] = z.x;
    rotationMat[1][2] = z.y;
    rotationMat[2][2] = z.z;
    translationMat[3][0] = -position.x;
    translationMat[3][1] = -position.y;
    translationMat[3][2] = -position.z;
    return rotationMat * translationMat;
  }

  // Methods to get view and projection matrices
  Matrix4f GetViewMatrix() {
    // return LookAt(position_, position_ + front_, up_);
    return glm::lookAt(position_, position_ + front_, up_);
  }

  Matrix4f GetProjectionMatrix(float fov, float aspectRatio, float nearPlane,
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
    Vector3f newFront;
    newFront.x = std::cos(glm::radians(yaw)) * std::cos(glm::radians(pitch));
    newFront.y = std::sin(glm::radians(pitch));
    newFront.z = std::sin(glm::radians(yaw)) * std::cos(glm::radians(pitch));
    front_ = glm::normalize(newFront);
  }

 private:
  Vector3f position_;
  Vector3f front_;
  Vector3f up_;
};

}  // namespace simple_renderer

#endif /* SIMPLERENDER_SRC_INCLUDE_CAMERA_H_ */
