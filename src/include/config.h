
/**
 * @file config.h
 * @brief 项目配置
 * @author Zone.N (Zone.Niuzh@hotmail.com)
 * @version 1.0
 * @date 2023-08-24
 * @copyright MIT LICENSE
 * https://github.com/Simple-XX/SimpleRenderer
 * @par change log:
 * <table>
 * <tr><th>Date<th>Author<th>Description
 * <tr><td>2023-08-24<td>Zone.N<td>创建文件
 * </table>
 */

#ifndef SIMPLERENDER_SRC_INCLUDE_CONFIG_H_
#define SIMPLERENDER_SRC_INCLUDE_CONFIG_H_

#include <string>
#include <string_view>

namespace simple_renderer {

/// 线程数
static constexpr const size_t kNProc = 8;

/// 日志文件路径
static const std::string kLogFilePath =
    std::string("/Users/hezhohao/Programming/GitRepo/SimpleRenderer/build/bin/logs/SimpleRendererLog.log");
/// 日志文件大小
static constexpr const size_t kLogFileMaxSize = 1024*1024*4;
/// 日志文件数量
static constexpr const size_t kLogFileMaxCount = 8;

} // namespace simple_renderer

#endif /* SIMPLERENDER_SRC_INCLUDE_CONFIG_H_ */
