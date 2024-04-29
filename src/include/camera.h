
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

#ifndef SIMPLERENDER_CAMERA_H
#define SIMPLERENDER_CAMERA_H

#include <cstdint>
#include <string>

#include "color.h"
#include "vector.hpp"

namespace SimpleRenderer {

/**
 * camera 抽象
 */
class camera_t {
public:
  /// 光照名称
  std::string name = "default light name";
  /// 位置
  vector3f_t pos;
  /// 方向
  vector3f_t dir;

  /**
   * 构造函数
   * @param _name camera 名称
   */
  explicit camera_t(const std::string &_name);

  /// @name 默认构造/析构函数
  /// @{
  camera_t() = default;
  camera_t(const camera_t &_light) = default;
  camera_t(camera_t &&_light) = default;
  auto operator=(const camera_t &_light) -> camera_t & = default;
  auto operator=(camera_t &&_light) -> camera_t & = default;
  ~camera_t() = default;
  /// @}
};

} // namespace SimpleRenderer

#endif /* SIMPLERENDER_CAMERA_H */
