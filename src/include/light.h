
/**
 * @file light.h
 * @brief 光照抽象
 * @author Zone.N (Zone.Niuzh@hotmail.com)
 * @version 1.0
 * @date 2022-12-15
 * @copyright MIT LICENSE
 * https://github.com/Simple-XX/SimpleRenderer
 * @par change log:
 * <table>
 * <tr><th>Date<th>Author<th>Description
 * <tr><td>2022-12-15<td>Zone.N<td>创建文件
 * </table>
 */

#ifndef SIMPLERENDER_LIGHT_H
#define SIMPLERENDER_LIGHT_H

#include <cstdint>
#include <string>

#include "color.h"
#include "vector.hpp"

namespace SimpleRenderer {

/**
 * 光照抽象
 */
class light_t {
public:
  /// 光照名称
  std::string name = "default light name";
  /// 位置
  vector3f_t pos = DEFAULT_POS;
  /// 方向
  vector3f_t dir = DEFAULT_DIR;
  /// 颜色
  color_t color = DEFAULT_COLOR;

  /**
   * 构造函数
   * @param _name 光照名称
   */
  explicit light_t(const std::string &_name);

  /// @name 默认构造/析构函数
  /// @{
  light_t() = default;
  light_t(const light_t &_light) = default;
  light_t(light_t &&_light) = default;
  auto operator=(const light_t &_light) -> light_t & = default;
  auto operator=(light_t &&_light) -> light_t & = default;
  ~light_t() = default;
  /// @}

private:
  /// 默认位置
  static const vector3f_t DEFAULT_POS;
  /// 默认方向，左手系，x 向右，y 向下，z 正方向为屏幕由内向外
  static const vector3f_t DEFAULT_DIR;
  /// 默认颜色
  static const color_t DEFAULT_COLOR;
};

} // namespace SimpleRenderer

#endif /* SIMPLERENDER_LIGHT_H */
