
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
#include "log_math.hpp"

namespace simple_renderer {

/**
 * camera 抽象
 */
class Camera {
 public:
  /// 光照名称
  std::string name_ = "default light name";
  /// 位置
  glm::vec3 pos_;
  /// 方向
  glm::vec3 dir_;

  /**
   * 构造函数
   * @param _name camera 名称
   */
  explicit Camera(const std::string &_name);

  /// @name 默认构造/析构函数
  /// @{
  Camera() = default;
  Camera(const Camera &light) = default;
  Camera(Camera &&light) = default;
  auto operator=(const Camera &light) -> Camera & = default;
  auto operator=(Camera &&light) -> Camera & = default;
  ~Camera() = default;
  /// @}
};

}  // namespace simple_renderer

#endif /* SIMPLERENDER_SRC_INCLUDE_CAMERA_H_ */
