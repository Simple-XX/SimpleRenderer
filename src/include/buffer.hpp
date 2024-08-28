#ifndef SIMPLERENDER_SRC_INCLUDE_BUFFER_HPP_
#define SIMPLERENDER_SRC_INCLUDE_BUFFER_HPP_

#include <memory>

#include "math.hpp"

namespace simple_renderer {

class Buffer {
  // buffer + matrix
 public:
  // Default constructor
  // 默认构造函数
  Buffer() = default;

  // Copy constructor
  // 拷贝构造函数
  Buffer(const Buffer& Buffer) = default;
  // Copy assignment operator
  // 拷贝赋值操作符
  Buffer& operator=(const Buffer& Buffer) = default;

  // Move constructor
  // 移动构造函数
  Buffer(Buffer&& Buffer) = default;
  // Move assignment operator
  // 移动赋值操作符
  Buffer& operator=(Buffer&& Buffer) = default;

  // Destructor
  // 析构函数
  ~Buffer() = default;

  Buffer(size_t width, size_t height)
      : width_(width),
        height_(height),
        size_(width * height),
        framebuffer_(new uint32_t[size_](), std::default_delete<uint32_t[]>()),
        matrix_(1.0f) {}

  uint32_t* framebuffer() { return framebuffer_.get(); }

  uint32_t& operator[](size_t index) {
    if (index >= size_) {
      throw std::out_of_range("Index out of range");
    }
    return framebuffer_.get()[index];
  }

  const Matrix4f& matrix() const { return matrix_; }

  void setMatrix(const Matrix4f& matrix) { matrix_ = matrix; }

 private:
  size_t width_;
  size_t height_;
  size_t size_;
  std::shared_ptr<uint32_t[]> framebuffer_;
  Matrix4f matrix_;
};

}  // namespace simple_renderer

#endif