
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
    int32_t width;
    /// 窗口高度
    int32_t height;

    /**
     * @brief 判断 _p 点是否在由 _p0 _p1 _p2 三点构成的三角形内
     * @tparam _T 点类型
     * @param  _p0              三角形的第一个点
     * @param  _p1              三角形的第二个点
     * @param  _p2              三角形的第三个点
     * @param  _p               要判断的点
     * @return true             在内
     * @return false            不在
     */
    template <class _T>
    static bool is_inside(const vector2_t<_T> &_p0, const vector2_t<_T> &_p1,
                          const vector2_t<_T> &_p2, const vector2_t<_T> &_p);

public:
    draw2d_t(void) = delete;
    draw2d_t(framebuffer_t &_framebuffer);
    ~draw2d_t(void);

    void line(const int32_t _x0, const int32_t _y0, const int32_t _x1,
              const int32_t _y1, const framebuffer_t::color_t &_color);
    void triangle(const vector2i_t &_v0, const vector2i_t &_v1,
                  const vector2i_t &_v2, const framebuffer_t::color_t &_color);
};

#endif /* _DRAW2D_H_ */
