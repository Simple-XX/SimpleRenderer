
/**
 * @file log_system.cpp
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

#include "log_system.h"

#include <iostream>

#include <spdlog/async.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

namespace simple_renderer {

LogSystem::LogSystem(const std::string &log_file_path, size_t lig_file_max_size,
                     size_t log_file_max_count) {
  spdlog::init_thread_pool(65536, 1);
//   std::string log_file_paths = "./logs/simple_renderer.log";
  std::cout << "-1: " << log_file_path << std::endl;
  auto stdout_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
  std::cout << "0: " << log_file_path << std::endl;
  auto rotating_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
      log_file_path, lig_file_max_size, log_file_max_count);
    std::cout << "1: " << log_file_path << std::endl;
  std::vector<spdlog::sink_ptr> sinks{stdout_sink, rotating_sink};
  logger_ = std::make_shared<spdlog::async_logger>(
      "multi_sink", sinks.begin(), sinks.end(), spdlog::thread_pool(),
      spdlog::async_overflow_policy::block);

    std::cout << "2: " << log_file_path << std::endl;

  // [年-月-日 时:分:秒.毫秒] [文件名:行号] [日志级别以彩色大写输出 8
  // 字符右对齐] 内容
  logger_->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%s:%# %!] [%^%l%$] %v");
  logger_->set_level(spdlog::level::trace);

  spdlog::register_logger(logger_);
  spdlog::flush_on(spdlog::level::trace);

  spdlog::set_default_logger(logger_);
}

LogSystem::~LogSystem() {
  logger_->flush();
  spdlog::drop_all();
  spdlog::shutdown();
}

}  // namespace simple_renderer
