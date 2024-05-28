
/**
 * @file light.cpp
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

#include "light.h"

#include "color.h"
#include "log_system.h"
#include "vector.hpp"

namespace simple_renderer {

const Vector3f Light::kDefaultPos = Vector3f(0, 0, 0);
const Vector3f Light::kDefaultDir = Vector3f(0, 0, -1);
const Color Light::kDefaultColor = Color::kWhite;

Light::Light(const std::string &name) : name_(name) {
  SPDLOG_INFO("Light: {}", name_);
}

}  // namespace simple_renderer
