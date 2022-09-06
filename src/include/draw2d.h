
/**
 * @file draw2d.h
 * @brief 二维绘制
 * @author Zone.N (Zone.Niuzh@hotmail.com)
 * @version 1.0
 * @date 2022-09-06
 * @copyright MIT LICENSE
 * https://github.com/Simple-XX/SimpleRenderer
 * @par change log:
 * <table>
 * <tr><th>Date<th>Author<th>Description
 * <tr><td>2022-09-06<td>Zone.N<td>创建文件
 * </table>
 */

#ifndef _DRAW2D_H_
#define _DRAW2D_H_

#include "framebuffer.h"
#include "vector.hpp"

class draw2d_t {
private:
    framebuffer_t &framebuffer;
    /// 窗口宽度
    uint32_t width;
    /// 窗口高度
    uint32_t height;

public:
    draw2d_t(void) = delete;
    draw2d_t(framebuffer_t &_framebuffer);
    ~draw2d_t(void);

    void line(const uint32_t _x0, const uint32_t _y0, const uint32_t _x1,
              const uint32_t _y1, const framebuffer_t::color_t &_color);
    void triangle(const vector2i_t &_v0, const vector2i_t &_v1,
                  const vector2i_t &_v2, const framebuffer_t::color_t &_color);
};

#endif /* _DRAW2D_H_ */
