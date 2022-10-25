
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

#ifndef _CONFIG_H_
#define _CONFIG_H_

class config_t {
public:
    /// @brief 控制是否绘制线框，false 时绘制整个图像
    bool draw_wireframe = true;
};

/// 在 main.cpp 中定义
extern config_t config;

#endif /* _CONFIG_H_ */
