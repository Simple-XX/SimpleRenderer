
/**
 * @file draw3d.h
 * @brief 三维绘制
 * @author Zone.N (Zone.Niuzh@hotmail.com)
 * @version 1.0
 * @date 2022-09-14
 * @copyright MIT LICENSE
 * https://github.com/Simple-XX/SimpleRenderer
 * @par change log:
 * <table>
 * <tr><th>Date<th>Author<th>Description
 * <tr><td>2022-09-14<td>Zone.N<td>创建文件
 * </table>
 */

#ifndef _DRAW3D_H_
#define _DRAW3D_H_

#include "framebuffer.h"
#include "vector.hpp"

class draw3d_t {
private:
    framebuffer_t &framebuffer;
    /// 窗口宽度
    int32_t width;
    /// 窗口高度
    int32_t height;

    /**
     * @brief 计算重心坐标
     * @param  _p0              三角形的第一个点
     * @param  _p1              三角形的第二个点
     * @param  _p2              三角形的第三个点
     * @param  _p               要判断的点
     * @return const std::pair<bool, vector4f_t>
     *  第一个返回为 _p 是否在三角形内，第二个为重心坐标
     */
    static const std::pair<bool, vector4f_t>
    get_barycentric_coord(const vector4f_t &_p0, const vector4f_t &_p1,
                          const vector4f_t &_p2, const vector4f_t &_p);

public:
    /**
     * @brief 构造函数，不使用
     */
    draw3d_t(void) = delete;

    /**
     * @brief 构造函数
     * @param  _framebuffer     另一个 framebuffer
     */
    draw3d_t(framebuffer_t &_framebuffer);

    /**
     * @brief 析构函数
     */
    ~draw3d_t(void);

    /**
     * @brief 填充三角形
     * @param  _v0              第一个顶点
     * @param  _v1              第二个顶点
     * @param  _v2              第三个顶点
     * @param  _color           填充的颜色
     * @todo 多线程支持
     */
    void triangle(const vector4f_t &_v0, const vector4f_t &_v1,
                  const vector4f_t &_v2, const framebuffer_t::color_t &_color);
};

#endif /* _DRAW3D_H_ */
