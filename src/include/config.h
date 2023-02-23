
/**
 * @file config.h
 * @brief 配置文件
 * @author Zone.N (Zone.Niuzh@hotmail.com)
 * @version 1.0
 * @date 2022-10-25
 * @copyright MIT LICENSE
 * https://github.com/Simple-XX/SimpleRenderer
 * @par change log:
 * <table>
 * <tr><th>Date<th>Author<th>Description
 * <tr><td>2022-10-25<td>Zone.N<td>创建文件
 * </table>
 */

#ifndef SIMPLERENDER_CONFIG_H
#define SIMPLERENDER_CONFIG_H

#include "atomic"
#include "cstdint"
#include "omp.h"

/**
 * @brief 配置选项封装
 */
class config_t {
public:
    /// @brief 字体文件路径
    static constexpr const char* FONT_FILE_PATH
      = "../../src/3rd/wqy-zenhei/wqy-zenhei.ttc";
    /// @brief 控制是否绘制线框，false 时绘制整个图像
    std::atomic_bool          draw_wireframe = true;

    /// @brief 本机处理器数量，用于 omp
    uint8_t                   procs          = omp_get_num_procs();

    static constexpr uint32_t WIDTH          = 1920;
    static constexpr uint32_t HEIGHT         = 1080;

    config_t(void)                           = default;

    config_t(const config_t& _config) {
        draw_wireframe = _config.draw_wireframe;
        procs          = _config.procs;
        return;
    }

    ~config_t(void) = default;

    config_t& operator=(const config_t& _config) {
        // draw_wireframe = _config.draw_wireframe;
        procs = _config.procs;
        return *this;
    }
};

#endif /* SIMPLERENDER_CONFIG_H */
