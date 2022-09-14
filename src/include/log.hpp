
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

#ifndef _LOG_HPP_
#define _LOG_HPP_

#include "string"
#include "source_location"
#include "ctime"

/**
 * @brief 构造日志字符串
 * @param  _msg             日志信息
 * @param  _location        位置
 * @return const std::string    日志信息，包括时间位置与信息
 * @todo    精确到毫秒
 */
inline const std::string
log(const std::string_view     &_msg,
    const std::source_location &_location = std::source_location::current()) {
    auto              tm         = time(0);
    auto              local_time = localtime(&tm);
    std::stringstream res;
    res << "[" << local_time->tm_hour << ":" << local_time->tm_min << ":"
        << local_time->tm_sec << "] " << _msg << ", file \'"
        << _location.file_name() << "\', line " << _location.line()
        << ", function \'" << _location.function_name() << "\'.";
    return res.str();
}

#endif /* _LOG_HPP_ */
