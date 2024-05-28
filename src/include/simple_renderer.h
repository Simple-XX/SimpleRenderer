
/**
 * @file simple_renderer.h
 * @brief simple_renderer 接口头文件
 * @author Zone.N (Zone.Niuzh@hotmail.com)
 * @version 1.0
 * @date 2023-10-23
 * @copyright MIT LICENSE
 * https://github.com/Simple-XX/SimpleRenderer
 * @par change log:
 * <table>
 * <tr><th>Date<th>Author<th>Description
 * <tr><td>2023-10-23<td>Zone.N<td>创建文件
 * </table>
 */

#ifndef SIMPLERENDER_SRC_INCLUDE_SIMPLE_RENDER_H_
#define SIMPLERENDER_SRC_INCLUDE_SIMPLE_RENDER_H_

#include <cstdint>

#include "log_system.h"

namespace simple_renderer {

class SimpleRenderer {
 public:
  SimpleRenderer(size_t width, size_t height);

  /// @name 默认构造/析构函数
  /// @{
  SimpleRenderer(const SimpleRenderer &_simplerenderer) = default;
  SimpleRenderer(SimpleRenderer &&_simplerenderer) = default;
  auto operator=(const SimpleRenderer &_simplerenderer) -> SimpleRenderer & =
                                                               default;
  auto operator=(SimpleRenderer &&_simplerenderer) -> SimpleRenderer & =
                                                          default;
  virtual ~SimpleRenderer() = default;
  /// @}

 private:
  LogSystem log_system_;
};

}  // namespace simple_renderer

#endif /* SIMPLERENDER_SRC_INCLUDE_SIMPLE_RENDER_H_ */
