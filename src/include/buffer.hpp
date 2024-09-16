#ifndef SIMPLERENDER_SRC_INCLUDE_BUFFER_HPP_
#define SIMPLERENDER_SRC_INCLUDE_BUFFER_HPP_

#include <algorithm>
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
        size_(width_ * height_),
        framebuffer_1_(new uint32_t[size_](),
                       std::default_delete<uint32_t[]>()),
        framebuffer_2_(new uint32_t[size_](),
                       std::default_delete<uint32_t[]>()) {
    ClearBuffer(framebuffer_1_.get());
    ClearBuffer(framebuffer_2_.get());
    drawBuffer_ = framebuffer_1_.get();
    displayBuffer_ = framebuffer_2_.get();
  }

  void ClearDrawBuffer(Color color) { ClearBuffer(drawBuffer_, color); }

  void SwapBuffer() { std::swap(drawBuffer_, displayBuffer_); }

  uint32_t* GetDisplayBuffer() { return displayBuffer_; }
  uint32_t* GetDrawBuffer() { return drawBuffer_; }

  uint32_t& operator[](size_t index) {
    if (index >= size_) {
      throw std::out_of_range("Index out of range");
    }
    return drawBuffer_[index];
  }

 private:
  size_t width_;
  size_t height_;
  size_t size_;
  std::unique_ptr<uint32_t[]> framebuffer_1_;
  std::unique_ptr<uint32_t[]> framebuffer_2_;
  uint32_t* drawBuffer_;
  uint32_t* displayBuffer_;

  void ClearBuffer(uint32_t* buffer, Color color) {
    std::fill(buffer, buffer + size_, uint32_t(color));
  }

  void ClearBuffer(uint32_t* buffer) { std::fill(buffer, buffer + size_, 0); }
};

}  // namespace simple_renderer

#endif