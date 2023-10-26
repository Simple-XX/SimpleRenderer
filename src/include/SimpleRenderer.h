
/**
 * @file SimpleRenderer.h
 * @brief SimpleRenderer 接口头文件
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

#ifndef SIMPLERENDER_SIMPLERENDER_H
#define SIMPLERENDER_SIMPLERENDER_H

#include <array>
#include <cstdint>
#include <string_view>
#include <vector>

namespace SimpleRenderer {

template <size_t _W, size_t _H> class SimpleRenderer {
public:
  SimpleRenderer();

  /// @name 默认构造/析构函数
  /// @{
  SimpleRenderer(const SimpleRenderer &_simplerenderer) = default;
  SimpleRenderer(SimpleRenderer &&_simplerenderer) = default;
  auto operator=(const SimpleRenderer &_simplerenderer)
      -> SimpleRenderer & = default;
  auto operator=(SimpleRenderer &&_simplerenderer)
      -> SimpleRenderer & = default;
  virtual ~SimpleRenderer() = default;
  /// @}

  void add_model(const std::string_view &_obj_path);
  auto get_buffer() const -> const std::array<uint32_t, _W * _H> &;
  void loop();
};

} // namespace SimpleRenderer

#endif /* SIMPLERENDER_SIMPLERENDER_H */
