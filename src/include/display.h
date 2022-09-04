
/**
 * @file display.h
 * @brief 显示
 * @author Zone.N (Zone.Niuzh@hotmail.com)
 * @version 1.0
 * @date 2022-09-04
 * @copyright MIT LICENSE
 * https://github.com/Simple-XX/SimpleRenderer
 * @par change log:
 * <table>
 * <tr><th>Date<th>Author<th>Description
 * <tr><td>2022-09-04<td>Zone.N<td>创建文件
 * </table>
 */

#ifndef _DISPLAY_H_
#define _DISPLAY_H_

#include "framebuffer.h"

/**
 * @brief 创建重口
 * @param  _framebuffer     帧缓冲
 */
void show_window(const framebuffer_t &_framebuffer);

#endif /* _DISPLAY_H_ */
