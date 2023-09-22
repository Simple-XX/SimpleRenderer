
/**
 * @file log.cpp
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

#include <chrono>
#include <memory>
#include <thread>

#include <spdlog/async.h>
#include <spdlog/spdlog.h>

#include <log.h>

std::shared_ptr<spdlog::async_logger> logger;

void log_thread() {
  spdlog::apply_all(
      [&](auto &logger) { logger->flush_on(spdlog::level::err); });
  spdlog::set_default_logger(logger);
  spdlog::flush_every(std::chrono::seconds(1));
  spdlog::shutdown();
}

std::thread log_thread(log_thread);

void log_init() {
  auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
  auto file_sink =
      std::make_shared<spdlog::sinks::basic_file_sink_mt>("example.log", true);
  spdlog::init_thread_pool(8192, 1);
  logger = std::make_shared<spdlog::async_logger>(
      "async_logger", spdlog::sinks_init_list{console_sink, file_sink},
      spdlog::thread_pool(), spdlog::async_overflow_policy::block);
  logger->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] %v");

  for (int i = 0; i < 100; ++i) {
    // 在其他线程中记录日志
    logger->info("This is a log message from another thread. Index: {}", i);
  }
  // 等待所有线程结束
  log_thread.join();
}
