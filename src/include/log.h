
/**
 * @file log.h
 * @brief 日志封装
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

#ifndef SIMPLERENDER_LOG_H
#define SIMPLERENDER_LOG_H

#include "chrono"
#include "ctime"
#include "source_location"
#include "sstream"
#include "string"
#include "sys/time.h"

#include <spdlog/spdlog.h>

extern std::shared_ptr<spdlog::logger> SRLOG;

/// @todo 修复 clang-tidy

/// 微秒到秒
static constexpr uint32_t US2S = 1000000;

/**
 * 构造日志字符串
 * @param  _msg             日志信息
 * @param  _location        位置
 * @return std::string      日志信息，包括时间位置与信息
 */
inline auto
log(const std::string_view &_msg,
    const std::source_location &_location = std::source_location::current())
    -> std::string {
  return std::string(_msg);
}

/**
 * 获取当前微秒数，用于性能分析
 * @return auto                 当前微秒
 */
inline auto us() {
  struct timeval t = {};

  gettimeofday(&t, nullptr);
  return t.tv_sec * 1000000 + t.tv_usec;
}

void log_init(void);

#endif /* SIMPLERENDER_LOG_H */
