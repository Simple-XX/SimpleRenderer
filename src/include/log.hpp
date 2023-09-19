
/**
 * @file log.hpp
 * @brief 简单日志
 * @author Zone.N (Zone.Niuzh@hotmail.com)
 * @version 1.0
 * @date 2022-09-14
 * @copyright MIT LICENSE
 * https://github.com/Simple-XX/SimpleRenderer
 * @par change log:
 * <table>
 * <tr><th>Date<th>Author<th>Description
 * <tr><td>2022-09-14<td>Zone.N<td>创建文件
 * </table>
 */

#ifndef SIMPLERENDER_LOG_HPP
#define SIMPLERENDER_LOG_HPP

#include "chrono"
#include "ctime"
#include "source_location"
#include "sstream"
#include "string"
#include "sys/time.h"

/// 微秒到秒
static constexpr uint32_t US2S = 1000000;

/**
 * 获取当前时间戳，精确到纳秒
 * @return std::string          时间戳
 */
static auto get_time_stamp(void) -> std::string {
  auto now = std::chrono::system_clock::now();
  auto now_time_t = std::chrono::system_clock::to_time_t(now);
  auto now_tm = std::localtime(&now_time_t);

  char buffer[128];
  strftime(buffer, sizeof(buffer), "%F %T", now_tm);

  std::ostringstream ss;
  ss.fill('0');

  auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                now.time_since_epoch()) %
            1000;
  auto cs = std::chrono::duration_cast<std::chrono::microseconds>(
                now.time_since_epoch()) %
            1000000;
  auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>(
                now.time_since_epoch()) %
            1000000000;
  ss << buffer << ":" << ms.count() << ":" << cs.count() % 1000 << ":"
     << ns.count() % 1000;

  return ss.str();
}

/**
 * 构造日志字符串
 * @param  _msg             日志信息
 * @param  _location        位置
 * @return std::string      日志信息，包括时间位置与信息
 */
inline std::string
log(const std::string_view &_msg,
    const std::source_location &_location = std::source_location::current()) {
  std::stringstream res;
  res << "[" << get_time_stamp() << "] " << _msg << ", function \'"
      << _location.function_name() << "\'"
      << ", file \'" << _location.file_name() << "\', line "
      << _location.line();
  return res.str();
}

/**
 * 获取当前微秒数，用于性能分析
 * @return auto                 当前微秒
 */
inline auto us(void) {
  struct timeval t = {};

  gettimeofday(&t, nullptr);
  return t.tv_sec * 1000000 + t.tv_usec;
}

#endif /* SIMPLERENDER_LOG_HPP */
