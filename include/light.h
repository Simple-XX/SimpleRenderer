
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

#ifndef SIMPLERENDER_SRC_INCLUDE_LIGHT_H_
#define SIMPLERENDER_SRC_INCLUDE_LIGHT_H_

#include <cstdint>
#include <string>

#include "color.h"
#include "log_math.hpp"

namespace simple_renderer {

/**
 * 光照抽象
 */
class Light {
 public:
  /// 光照名称
  std::string name_ = "default light name";
  /// 位置
  glm::vec3 pos = kDefaultPos;
  /// 方向
  glm::vec3 dir = kDefaultDir;
  /// 颜色
  Color color = kDefaultColor;

  /**
   * 构造函数
   * @param name 光照名称
   */
  explicit Light(const std::string &name);

  /// @name 默认构造/析构函数
  /// @{
  Light() = default;
  Light(const Light &light) = default;
  Light(Light &&light) = default;
  auto operator=(const Light &light) -> Light & = default;
  auto operator=(Light &&light) -> Light & = default;
  ~Light() = default;
  /// @}

 private:
  /// 默认位置
  static const glm::vec3 kDefaultPos;
  /// 默认方向，左手系，x 向右，y 向下，z 正方向为屏幕由内向外
  static const glm::vec3 kDefaultDir;
  /// 默认颜色
  static const Color kDefaultColor;
};

}  // namespace simple_renderer

#endif /* SIMPLERENDER_SRC_INCLUDE_LIGHT_H_ */
