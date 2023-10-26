
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

#ifndef SIMPLERENDER_EXCEPTION_HPP
#define SIMPLERENDER_EXCEPTION_HPP

#include <exception>
#include <string>

#include "log.h"

namespace SimpleRenderer {

class exception : public std::exception {
public:
  exception() : message("") { SRLOG->error(""); }
  explicit exception(std::string _str) : message(_str) { SRLOG->error(_str); }

  /// @name 默认构造/析构函数
  /// @{
  exception(const exception &_color) = default;
  exception(exception &&_color) = default;
  auto operator=(const exception &_color) -> exception & = default;
  auto operator=(exception &&_color) -> exception & = default;
  ~exception() throw() = default;
  /// @}

  virtual const char *what() const throw() { return message.c_str(); }

private:
  std::string message;
};

} // namespace SimpleRenderer

#endif /* SIMPLERENDER_EXCEPTION_HPP */
