
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
#include "model.h"
#include "matrix.hpp"

class draw3d_t {
private:
    std::shared_ptr<framebuffer_t> framebuffer;
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
    draw3d_t(std::shared_ptr<framebuffer_t> _framebuffer);

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

    /**
     * @brief 填充三角形，传入的顶点包含更多信息
     * @param  _v0              第一个顶点
     * @param  _v1              第二个顶点
     * @param  _v2              第三个顶点
     * @param  _normal          面的法向量
     * @todo 多线程支持
     */
    void triangle(const model_t::vertex_t &_v0, const model_t::vertex_t &_v1,
                  const model_t::vertex_t &_v2,
                  const model_t::normal_t &_normal);

    void triangle(const model_t::face_t &_face);

    void model(const model_t &_model, const matrix4f_t &_tran);
};

#endif /* _DRAW3D_H_ */
