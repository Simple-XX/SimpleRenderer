
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

#ifndef SIMPLERENDER_SRC_INCLUDE_COLOR_H_
#define SIMPLERENDER_SRC_INCLUDE_COLOR_H_

#include <cstdint>
#include <iostream>
#include <limits>

namespace simple_renderer {

/**
 * 32 位颜色 RGBA
 * @note 不要进行会影响内存的修改
 */
class Color {
 public:
  enum ColorIndex {
    kColorIndexRed [[maybe_unused]] = 0,
    kColorIndexGreen [[maybe_unused]] = 1,
    kColorIndexBlue [[maybe_unused]] = 2,
    kColorIndexAlpha [[maybe_unused]] = 3,
  };

  /// @name 颜色常量
  /// @{
  static const Color kWhite;
  static const Color kBlack;
  static const Color kRed;
  static const Color kGreen;
  static const Color kBlue;
  /// @}

  /**
   * 构造函数
   * @param data 颜色数据
   */
  explicit Color(uint32_t data);

  /**
   * 构造函数
   * @param red 红色
   * @param green 绿色
   * @param blue 蓝色
   * @param alpha alpha
   */
  explicit Color(uint8_t red, uint8_t green, uint8_t blue,
                 uint8_t alpha = std::numeric_limits<uint8_t>::max());

  /**
   * 构造函数，从 [0, 1] 构建
   * @param red 红色
   * @param green 绿色
   * @param blue 蓝色
   * @param alpha alpha
   * @todo 添加参数检查
   */
  explicit Color(float red, float green, float blue,
                 float alpha = std::numeric_limits<uint8_t>::max());

  /// @name 默认构造/析构函数
  /// @{
  Color() = default;
  Color(const Color &color) = default;
  Color(Color &&color) = default;
  auto operator=(const Color &color) -> Color & = default;
  auto operator=(Color &&color) -> Color & = default;
  ~Color() = default;
  /// @}

  /**
   * 下标重载
   * @param idx 索引
   * @return 对应颜色
   */
  auto operator[](uint8_t idx) -> uint8_t &;

  /**
   * 下标重载
   * @param idx 索引
   * @return 对应颜色
   */
  auto operator[](uint8_t idx) const -> uint8_t;

  /**
   * float * 重载
   * @param val 值
   * @return 颜色
   */
  auto operator*(float val) const -> Color;

  /**
   * float *= 重载
   * @param val 值
   * @return 颜色
   */
  auto operator*=(float val) -> Color &;

  /**
   * 颜色相加
   * @param color 颜色
   * @return 颜色
   */
  auto operator+(const Color &color) const -> Color;

  /**
   * 获取每像素大小
   * @return size_t           像素大小，单位为字节
   */
  [[maybe_unused]] static auto GetBpp() -> size_t;

  /**
   * uint32_t 类型转换重载
   */
  operator uint32_t() const;

  friend auto operator<<(std::ostream &_os,
                         const Color &color) -> std::ostream & {
    _os << std::hex;
    _os << "RGBA[ 0x" << +color.channel_r_ << ", 0x" << +color.channel_g_
        << ", 0x" << +color.channel_b_ << ", 0x" << +color.channel_a_ << " ]";
    _os << std::dec;

    return _os;
  }

 private:
  /// 颜色深度
  static constexpr const uint8_t kDepth = 4;

  /// 每像素字节数
  static constexpr const size_t kBpp = sizeof(uint8_t) * kDepth;

  /// @name 颜色数据，rgba
  /// @{
  uint8_t channel_r_ = 0;
  uint8_t channel_g_ = 0;
  uint8_t channel_b_ = 0;
  uint8_t channel_a_ = std::numeric_limits<uint8_t>::max();
  /// @}
};

}  // namespace simple_renderer

#endif /* SIMPLERENDER_SRC_INCLUDE_COLOR_H_ */
