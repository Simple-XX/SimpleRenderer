
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

#include "limits"

#include "color.h"

color_t::color_t(void) {
  channel_r = 0;
  channel_g = 0;
  channel_b = 0;
  channel_a = std::numeric_limits<uint8_t>::max();
  return;
}

color_t::color_t(uint32_t _data) {
  auto data_ptr = (uint8_t *)&_data;
  channel_r = data_ptr[0];
  channel_g = data_ptr[1];
  channel_b = data_ptr[2];
  channel_a = data_ptr[3];
  return;
}

color_t::color_t(uint8_t _r, uint8_t _g, uint8_t _b, uint8_t _a) {
  channel_r = _r;
  channel_g = _g;
  channel_b = _b;
  channel_a = _a;
  return;
}

color_t::color_t(float _r, float _g, float _b, uint8_t _a) {
  channel_r = uint8_t(_r * std::numeric_limits<uint8_t>::max());
  channel_g = uint8_t(_g * std::numeric_limits<uint8_t>::max());
  channel_b = uint8_t(_b * std::numeric_limits<uint8_t>::max());
  channel_a = _a;
  return;
}

color_t::color_t(const color_t &_color) {
  channel_r = _color.channel_r;
  channel_g = _color.channel_g;
  channel_b = _color.channel_b;
  channel_a = _color.channel_a;
  return;
}

color_t::~color_t(void) { return; }

color_t &color_t::operator=(const color_t &_color) {
  if (this == &_color) {
    throw std::runtime_error(log("this == &_color"));
  }
  channel_r = _color.channel_r;
  channel_g = _color.channel_g;
  channel_b = _color.channel_b;
  channel_a = _color.channel_a;
  return *this;
}

color_t color_t::operator*(float _f) const {
  auto r = static_cast<uint8_t>(static_cast<float>(channel_r) * _f);
  auto g = static_cast<uint8_t>(static_cast<float>(channel_g) * _f);
  auto b = static_cast<uint8_t>(static_cast<float>(channel_b) * _f);
  auto a = static_cast<uint8_t>(static_cast<float>(channel_a) * _f);
  return color_t(r, g, b, a);
}

color_t color_t::operator*(const vector4f_t &_vector) const {
  auto r = static_cast<uint8_t>(static_cast<float>(channel_r) * _vector.vector.x());
  auto g = static_cast<uint8_t>(static_cast<float>(channel_g) * _vector.vector.y());
  auto b = static_cast<uint8_t>(static_cast<float>(channel_b) * _vector.vector.z());
  auto a = static_cast<uint8_t>(static_cast<float>(channel_a) * _vector.vector.w());
  return color_t(r, g, b, a);
}

color_t &color_t::operator*=(float _f) {
  channel_r = static_cast<uint8_t>(static_cast<float>(channel_r) * _f);
  channel_g = static_cast<uint8_t>(static_cast<float>(channel_g) * _f);
  channel_b = static_cast<uint8_t>(static_cast<float>(channel_b) * _f);
  channel_a = static_cast<uint8_t>(static_cast<float>(channel_a) * _f);
  return *this;
}

color_t &color_t::operator*=(const vector4f_t &_vector) {
  channel_r = static_cast<uint8_t>(static_cast<float>(channel_r) * _vector.vector.x());
  channel_g = static_cast<uint8_t>(static_cast<float>(channel_g) * _vector.vector.y());
  channel_b = static_cast<uint8_t>(static_cast<float>(channel_b) * _vector.vector.z());
  channel_a = static_cast<uint8_t>(static_cast<float>(channel_a) * _vector.vector.w());
  return *this;
}

uint8_t &color_t::operator[](uint8_t _idx) {
  if (_idx > 3) {
    throw std::invalid_argument(log("_idx > 3"));
  } else if (_idx == 0) {
    return channel_r;
  } else if (_idx == 1) {
    return channel_g;
  } else if (_idx == 2) {
    return channel_b;
  } else {
    return channel_a;
  }
}

uint8_t color_t::operator[](uint8_t _idx) const {
  if (_idx > 3) {
    throw std::invalid_argument(log("_idx > 3"));
  } else if (_idx == 0) {
    return channel_r;
  } else if (_idx == 1) {
    return channel_g;
  } else if (_idx == 2) {
    return channel_b;
  } else {
    return channel_a;
  }
}

size_t color_t::bpp(void) { return BPP; }

uint32_t color_t::to_uint32(void) const {
  uint32_t ret = 0;
  auto ret_ptr = (uint8_t *)&ret;
  ret_ptr[0] = channel_r;
  ret_ptr[1] = channel_g;
  ret_ptr[2] = channel_b;
  ret_ptr[3] = channel_a;

  return ret;
}

color_t color_t::WHITE = color_t((uint8_t)0xFF, 0xFF, 0xFF);
color_t color_t::BLACK = color_t((uint8_t)0x00, 0x00, 0x00);
color_t color_t::RED = color_t((uint8_t)0xFF, 0x00, 0x00);
color_t color_t::GREEN = color_t((uint8_t)0x00, 0xFF, 0x00);
color_t color_t::BLUE = color_t((uint8_t)0x00, 0x00, 0xFF);
