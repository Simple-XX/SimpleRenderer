
/**
 * @file simple_renderer.cpp
 * @brief SimpleRenderer 实现
 * @author Zone.N (Zone.Niuzh@hotmail.com)
 * @version 1.0
 * @date 2023-10-23
 * @copyright MIT LICENSE
 * https://github.com/Simple-XX/SimpleRenderer
 * @par change log:
 * <table>
 * <tr><th>Date<th>Author<th>Description
 * <tr><td>2023-10-23<td>Zone.N<td>创建文件
 * </table>
 */

#include "simple_renderer.h"

#include <array>
#include <cstdint>
#include <string_view>
#include <vector>

#include "config.h"
#include "log_system.h"

namespace simple_renderer {

SimpleRenderer::SimpleRenderer(size_t width, size_t height)
    : log_system_(LogSystem(kLogFilePath, kLogFileMaxSize, kLogFileMaxCount)) {
  SPDLOG_INFO("SimpleRenderer init with {}, {}", width, height);
  ;
}

}  // namespace simple_renderer
