
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
     * @brief 计算重心坐标
     * @tparam _T 点类型
     * @param  _p0              三角形的第一个点
     * @param  _p1              三角形的第二个点
     * @param  _p2              三角形的第三个点
     * @param  _p               要判断的点
     * @return const std::pair<bool, vector3f_t>
     *  第一个返回为 _p 是否在三角形内，第二个为重心坐标
     * @see https://blog.csdn.net/wangjiangrong/article/details/115326930
     */
    template <class _T>
    static const std::pair<bool, vector3f_t>
    get_barycentric_coord(const vector2_t<_T> &_p0, const vector2_t<_T> &_p1,
                          const vector2_t<_T> &_p2, const vector2_t<_T> &_p);

public:
    /**
     * @brief 构造函数，不使用
     */
    draw2d_t(void) = delete;

    /**
     * @brief 构造函数
     * @param  _framebuffer     另一个 framebuffer
     */
    draw2d_t(framebuffer_t &_framebuffer);

    /**
     * @brief 析构函数
     */
    ~draw2d_t(void);

    /**
     * @brief 画直线 Bresenham 算法
     * @param  _x0              第一个点的 x 坐标
     * @param  _y0              第一个点的 y 坐标
     * @param  _x1              第二个点的 x 坐标
     * @param  _y1              第二个点的 y 坐标
     * @param  _color           直线颜色
     * @todo 多线程支持
     */
    void line(const int32_t _x0, const int32_t _y0, const int32_t _x1,
              const int32_t _y1, const framebuffer_t::color_t &_color);

    /**
     * @brief 直线重载
     * @param  _p0              第一个点
     * @param  _p1              第二个点
     * @param  _color           颜色
     */
    void line(const vector2i_t &_p0, const vector2i_t &_p1,
              const framebuffer_t::color_t &_color);

    /**
     * @brief 填充三角形
     * @param  _v0              第一个顶点
     * @param  _v1              第二个顶点
     * @param  _v2              第三个顶点
     * @param  _color           填充的颜色
     * @todo 多线程支持
     */
    void triangle(const vector2i_t &_v0, const vector2i_t &_v1,
                  const vector2i_t &_v2, const framebuffer_t::color_t &_color);
};

#endif /* _DRAW2D_H_ */
