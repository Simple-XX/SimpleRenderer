
/**
 * @file exception.hpp
 * @brief 异常封装
 * @author Zone.N (Zone.Niuzh@hotmail.com)
 * @version 1.0
 * @date 2022-10-16
 * @copyright MIT LICENSE
 * https://github.com/Simple-XX/SimpleRenderer
 * @par change log:
 * <table>
 * <tr><th>Date<th>Author<th>Description
 * <tr><td>2023-10-16<td>Zone.N<td>创建文件
 * </table>
 */

#ifndef SIMPLERENDER_SRC_INCLUDE_EXCEPTION_HPP_
#define SIMPLERENDER_SRC_INCLUDE_EXCEPTION_HPP_

#include <exception>
#include <string>

#include "log_system.h"

namespace simple_renderer {

class Exception : public std::exception {
 public:
  Exception() : message_("") { SPDLOG_ERROR(""); }
  explicit Exception(std::string _str) : message_(_str) { SPDLOG_ERROR(_str); }

  /// @name 默认构造/析构函数
  /// @{
  Exception(const Exception &_color) = default;
  Exception(Exception &&_color) = default;
  auto operator=(const Exception &_color) -> Exception & = default;
  auto operator=(Exception &&_color) -> Exception & = default;
  ~Exception() throw() = default;
  /// @}

  virtual const char *what() const throw() { return message_.c_str(); }

 private:
  std::string message_;
};

}  // namespace simple_renderer

#endif /* SIMPLERENDER_SRC_INCLUDE_EXCEPTION_HPP_ */
