
/**
 * @file log_system.h
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

#ifndef SIMPLERENDER_SRC_INCLUDE_LOG_SYSTEM_H_
#define SIMPLERENDER_SRC_INCLUDE_LOG_SYSTEM_H_

#ifndef SPDLOG_ACTIVE_LEVEL
#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_INFO
#endif
#include <spdlog/spdlog.h>

namespace simple_renderer {

/**
 * 日志系统
 */
class LogSystem {
 public:
  /**
   * 构造函数
   * @param log_file_path 日志文件路径
   * @param lig_file_max_size 日志文件最大大小
   * @param log_file_max_count 日志文件最大数量
   */
  explicit LogSystem(const std::string &log_file_path, size_t lig_file_max_size,
                     size_t log_file_max_count);

  /**
   * 析构函数
   */
  ~LogSystem();

  /// @name 默认构造/析构函数
  /// @{
  LogSystem() = delete;
  LogSystem(const LogSystem &_scene) = delete;
  LogSystem(LogSystem &&_scene) = delete;
  auto operator=(const LogSystem &_scene) -> LogSystem & = delete;
  auto operator=(LogSystem &&_scene) -> LogSystem & = delete;
  /// @}

  /**
   * @name 日志打印
   * @brief 各个级别的日志
   * @tparam TARGS 参数类型
   * @param args 参数
   */
  /// @{
  template <typename... TARGS>
  void info(TARGS &&...args) {
    SPDLOG_LOGGER_INFO(logger_, std::forward<TARGS>(args)...);
  }

  template <typename... TARGS>
  void warn(TARGS &&...args) {
    SPDLOG_LOGGER_WARN(logger_, std::forward<TARGS>(args)...);
  }

  template <typename... TARGS>
  void err(TARGS &&...args) {
    SPDLOG_LOGGER_ERROR(logger_, std::forward<TARGS>(args)...);
  }

  template <typename... TARGS>
  void debug(TARGS &&...args) {
    SPDLOG_LOGGER_DEBUG(logger_, std::forward<TARGS>(args)...);
  }

  template <typename... TARGS>
  void fatal(TARGS &&...args) {
    SPDLOG_LOGGER_CRITICAL(logger_, std::forward<TARGS>(args)...);
    fatalCallback(std::forward<TARGS>(args)...);
  }

  template <typename... TARGS>
  void fatalCallback(TARGS &&...args) {
    const std::string format_str = fmt::format(std::forward<TARGS>(args)...);
    throw std::runtime_error(format_str);
  }
  /// @}

 private:
  /// 日志指针
  std::shared_ptr<spdlog::logger> logger_;
};

}  // namespace simple_renderer

#endif /* SIMPLERENDER_SRC_INCLUDE_LOG_SYSTEM_H_ */
