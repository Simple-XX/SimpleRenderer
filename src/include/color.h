
/**
 * @file color.h
 * @brief 颜色抽象
 * @author Zone.N (Zone.Niuzh@hotmail.com)
 * @version 1.0
 * @date 2022-1-02
 * @copyright MIT LICENSE
 * https://github.com/Simple-XX/SimpleRenderer
 * @par change log:
 * <table>
 * <tr><th>Date<th>Author<th>Description
 * <tr><td>2022-1-02<td>Zone.N<td>创建文件
 * </table>
 */

#ifndef SIMPLERENDER_COLOR_H
#define SIMPLERENDER_COLOR_H

#include <cstdint>
#include <iostream>

#include "vector.hpp"

namespace SimpleRenderer {

/**
 * 32 位颜色 RGBA
 * @note 不要进行会影响内存的修改
 */
class color_t {
public:
  enum color_idx_t {
    COLOR_IDX_R [[maybe_unused]] = 0,
    COLOR_IDX_G [[maybe_unused]] = 1,
    COLOR_IDX_B [[maybe_unused]] = 2,
    COLOR_IDX_A [[maybe_unused]] = 3,
  };

  /// @name 颜色常量
  /// @{
  static const color_t WHITE;
  static const color_t BLACK;
  static const color_t RED;
  static const color_t GREEN;
  static const color_t BLUE;
  /// @}

  /**
   * 构造函数
   * @param _data 颜色数据
   */
  explicit color_t(uint32_t _data);

  /**
   * 构造函数
   * @param _red 红色
   * @param _green 绿色
   * @param _blue 蓝色
   * @param _alpha alpha
   */
  explicit color_t(uint8_t _red, uint8_t _green, uint8_t _blue,
                   uint8_t _alpha = std::numeric_limits<uint8_t>::max());

  /**
   * 构造函数，从 [0, 1] 构建
   * @param _red 红色
   * @param _green 绿色
   * @param _blue 蓝色
   * @param _alpha alpha
   */
  explicit color_t(float _red, float _green, float _blue,
                   float _alpha = std::numeric_limits<uint8_t>::max());

  /// @name 默认构造/析构函数
  /// @{
  color_t() = default;
  color_t(const color_t &_color) = default;
  color_t(color_t &&_color) = default;
  auto operator=(const color_t &_color) -> color_t & = default;
  auto operator=(color_t &&_color) -> color_t & = default;
  ~color_t() = default;
  /// @}

  /**
   * 下标重载
   * @param _idx 索引
   * @return 对应颜色
   */
  auto operator[](uint8_t _idx) -> uint8_t &;

  /**
   * 下标重载
   * @param _idx 索引
   * @return 对应颜色
   */
  auto operator[](uint8_t _idx) const -> uint8_t;

  /**
   * float * 重载
   * @param _val 值
   * @return 颜色
   */
  auto operator*(float _val) const -> color_t;

  /**
   * float *= 重载
   * @param _val 值
   * @return 颜色
   */
  auto operator*=(float _val) -> color_t &;

  /**
   * 获取每像素大小
   * @return size_t           像素大小，单位为字节
   */
  [[maybe_unused]] static auto bpp() -> size_t;

  /**
   * 转换为 uint32_t
   * @return uint32_t         结果
   */
  [[maybe_unused]] [[nodiscard]] auto to_uint32() const -> uint32_t;

  friend auto operator<<(std::ostream &_os, const color_t &_color)
      -> std::ostream & {
    _os << std::hex;
    _os << "RGBA[ 0x" << +_color.channel_r << ", 0x" << +_color.channel_g
        << ", 0x" << +_color.channel_b << ", 0x" << +_color.channel_a << " ]";
    _os << std::dec;

    return _os;
  }

private:
  /// 颜色深度
  static constexpr const uint8_t DEPTH = 4;

  /// 每像素字节数
  static constexpr const size_t BPP = sizeof(uint8_t) * DEPTH;

  /// @name 颜色数据，rgba
  /// @{
  uint8_t channel_r = 0;
  uint8_t channel_g = 0;
  uint8_t channel_b = 0;
  uint8_t channel_a = std::numeric_limits<uint8_t>::max();
  /// @}
};

} // namespace SimpleRenderer

#endif /* SIMPLERENDER_COLOR_H */
