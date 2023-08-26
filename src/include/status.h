
/**
 * @file status.h
 * @brief 运行状态
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

#ifndef SIMPLERENDER_STATUS_H
#define SIMPLERENDER_STATUS_H

#include <atomic>

/// 运行状态
struct state_t {
  enum status_t : uint8_t {
    /// 停止
    STOP = 0,
    /// 正在运行
    RUNNING = 1,
    /// 等待绘制
    WAIT_FRAMEBUFFER,
    /// 等待显示
    WAIT_DISPLAY
  };

  /// 是否还在运行，为 false 时退出
  std::atomic_uint8_t status = RUNNING;
};

#endif /* SIMPLERENDER_STATUS_H */
