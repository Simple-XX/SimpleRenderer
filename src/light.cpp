
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

light_t::light_t(void)
    : pos(DEFAULT_POS), dir(DEFAULT_DIR), color(DEFAULT_COLOR) {
  return;
}

light_t::light_t(const light_t &_light) {
  pos = _light.pos;
  dir = _light.dir;
  color = _light.color;

  return;
}

light_t::~light_t(void) { return; }

light_t &light_t::operator=(const light_t &_light) {
  pos = _light.pos;
  dir = _light.dir;
  color = _light.color;

  return *this;
}
