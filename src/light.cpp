
/**
 * @file light.cpp
 * @brief 光照抽象
 * @author Zone.N (Zone.Niuzh@hotmail.com)
 * @version 1.0
 * @date 2022-12-15
 * @copyright MIT LICENSE
 * https://github.com/Simple-XX/SimpleRenderer
 * @par change log:
 * <table>
 * <tr><th>Date<th>Author<th>Description
 * <tr><td>2022-12-15<td>Zone.N<td>创建文件
 * </table>
 */

#include "light.h"

const vector3f_t light_t::DEFAULT_POS = vector3f_t(0, 0, 0);
const vector3f_t light_t::DEFAULT_DIR = vector3f_t(0, 0, -1);
const color_t light_t::DEFAULT_COLOR = color_t::WHITE;
