
/**
 * @file color.cpp
 * @brief 颜色实现
 * @author Zone.N (Zone.Niuzh@hotmail.com)
 * @version 1.0
 * @date 2022-11-02
 * @copyright MIT LICENSE
 * https://github.com/Simple-XX/SimpleRenderer
 * @par change log:
 * <table>
 * <tr><th>Date<th>Author<th>Description
 * <tr><td>2022-11-02<td>Zone.N<td>创建文件
 * </table>
 */

#include "color.h"

#include <limits>
#include <span>

#include "exception.hpp"
#include "log_system.h"

namespace simple_renderer {

const Color Color::kWhite = Color((uint8_t)0xFF, 0xFF, 0xFF);
const Color Color::kBlack = Color((uint8_t)0x00, 0x00, 0x00);
const Color Color::kRed = Color((uint8_t)0xFF, 0x00, 0x00);
const Color Color::kGreen = Color((uint8_t)0x00, 0xFF, 0x00);
const Color Color::kBlue = Color((uint8_t)0x00, 0x00, 0xFF);

Color::Color(uint32_t data) {
  auto data_ptr = std::span(reinterpret_cast<uint8_t *>(&data), 4);
  channel_r_ = data_ptr[0];
  channel_g_ = data_ptr[1];
  channel_b_ = data_ptr[2];
  channel_a_ = data_ptr[3];
}

Color::Color(uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha)
    : channel_r_(red), channel_g_(green), channel_b_(blue), channel_a_(alpha) {}

Color::Color(float red, float green, float blue, float alpha)
    : channel_r_(uint8_t(red * std::numeric_limits<uint8_t>::max())),
      channel_g_(uint8_t(green * std::numeric_limits<uint8_t>::max())),
      channel_b_(uint8_t(blue * std::numeric_limits<uint8_t>::max())),
      channel_a_(uint8_t(alpha * std::numeric_limits<uint8_t>::max())) {}

auto Color::operator[](uint8_t idx) -> uint8_t & {
  if (idx > 3) {
    throw Exception("idx > 3");
  }
  if (idx == 0) {
    return channel_r_;
  }
  if (idx == 1) {
    return channel_g_;
  }
  if (idx == 2) {
    return channel_b_;
  }
  return channel_a_;
}

auto Color::operator[](uint8_t idx) const -> uint8_t {
  if (idx > 3) {
    throw Exception("idx > 3");
  }
  if (idx == 0) {
    return channel_r_;
  }
  if (idx == 1) {
    return channel_g_;
  }
  if (idx == 2) {
    return channel_b_;
  }
  return channel_a_;
}

auto Color::operator*(float val) const -> Color {
  auto red = static_cast<uint8_t>(static_cast<float>(channel_r_) * val);
  auto green = static_cast<uint8_t>(static_cast<float>(channel_g_) * val);
  auto blue = static_cast<uint8_t>(static_cast<float>(channel_b_) * val);
  auto alpha = static_cast<uint8_t>(static_cast<float>(channel_a_) * val);
  return Color(red, green, blue, alpha);
}

auto Color::operator*=(float val) -> Color & {
  channel_r_ = static_cast<uint8_t>(static_cast<float>(channel_r_) * val);
  channel_g_ = static_cast<uint8_t>(static_cast<float>(channel_g_) * val);
  channel_b_ = static_cast<uint8_t>(static_cast<float>(channel_b_) * val);
  channel_a_ = static_cast<uint8_t>(static_cast<float>(channel_a_) * val);
  return *this;
}

Color::operator uint32_t() const {
  uint32_t ret = 0;
  auto ret_ptr = std::span(reinterpret_cast<uint8_t *>(&ret), 4);

  ret_ptr[0] = channel_r_;
  ret_ptr[1] = channel_g_;
  ret_ptr[2] = channel_b_;
  ret_ptr[3] = channel_a_;

  return ret;
}

[[maybe_unused]] auto Color::GetBpp() -> size_t { return kBpp; }

}  // namespace simple_renderer
