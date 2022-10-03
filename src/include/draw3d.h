
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
#include "matrix.hpp"
#include "model.h"
#include "vector.hpp"

class draw3d_t {
private:
    std::shared_ptr<framebuffer_t> framebuffer;
    /// 窗口宽度
    uint32_t                       width;
    /// 窗口高度
    uint32_t                       height;

    /// 光照方向
    vector4f_t                     light = vector4f_t(0, 0, -1);

    /**
     * @brief 计算重心坐标
     * @param  _p0              三角形的第一个点
     * @param  _p1              三角形的第二个点
     * @param  _p2              三角形的第三个点
     * @param  _p               要判断的点
     * @return const std::pair<bool, vector4f_t>
     *  第一个返回为 _p 是否在三角形内，第二个为重心坐标
     * @see http://blackpawn.com/texts/pointinpoly/
     * solve
     *     P = A + s * AB + t * AC  -->  AP = s * AB + t * AC
     * then
     *     s = (AC.y * AP.x - AC.x * AP.y) / (AB.x * AC.y - AB.y * AC.x)
     *     t = (AB.x * AP.y - AB.y * AP.x) / (AB.x * AC.y - AB.y * AC.x)
     *
     * notice
     *     P = A + s * AB + t * AC
     *       = A + s * (B - A) + t * (C - A)
     *       = (1 - s - t) * A + s * B + t * C
     * then
     *     weight_A = 1 - s - t
     *     weight_B = s
     *     weight_C = t
     */
    static const std::pair<bool, vector4f_t>
    get_barycentric_coord(const vector4f_t& _p0, const vector4f_t& _p1,
                          const vector4f_t& _p2, const vector4f_t& _p);

    /**
     * @brief 计算变换矩阵，缩放 + 移动到 (0, 0)，屏幕左上角
     * @param  _model           要被应用的模型
     * @bug 变换可能有问题
     * @param  _rotate          在默认变换的基础上进行变换的旋转矩阵，默认为 1
     * @param  _scale           在默认变换的基础上进行变换的缩放矩阵，默认为 1
     * @param  _translate       在默认变换的基础上进行变换的平移矩阵，默认为 1
     * @return const matrix4f_t 变换矩阵
     */
    const matrix4f_t
    model2world_tran(const model_t&    _model,
                     const matrix4f_t& _rotate    = matrix4f_t(),
                     const matrix4f_t& _scale     = matrix4f_t(),
                     const matrix4f_t& _translate = matrix4f_t()) const;

    /**
     * @brief 填充三角形，传入的顶点包含更多信息
     * @param  _v0              第一个顶点
     * @param  _v1              第二个顶点
     * @param  _v2              第三个顶点
     * @param  _normal          面的法向量
     * @todo 多线程支持
     */
    void
         triangle(const model_t::vertex_t& _v0, const model_t::vertex_t& _v1,
                  const model_t::vertex_t& _v2, const model_t::normal_t& _normal);

    /**
     * @brief 填充三角形，传入面信息
     * @param  _face            面
     */
    void triangle(const model_t::face_t& _face);

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
     * @brief 画直线 Bresenham 算法
     * @param  _x0              第一个点的 x 坐标
     * @param  _y0              第一个点的 y 坐标
     * @param  _x1              第二个点的 x 坐标
     * @param  _y1              第二个点的 y 坐标
     * @param  _color           直线颜色
     * @todo 多线程支持
     */
    void line(const int32_t _x0, const int32_t _y0, const int32_t _x1,
              const int32_t _y1, const framebuffer_t::color_t& _color);

    /**
     * @brief 直线重载
     * @param  _p0              第一个点
     * @param  _p1              第二个点
     * @param  _color           颜色
     */
    void line(const vector4f_t& _p0, const vector4f_t& _p1,
              const framebuffer_t::color_t& _color);

    /**
     * @brief 填充三角形
     * @param  _v0              第一个顶点
     * @param  _v1              第二个顶点
     * @param  _v2              第三个顶点
     * @param  _color           填充的颜色
     * @todo 多线程支持
     */
    void
         triangle2d(const vector4f_t& _v0, const vector4f_t& _v1,
                    const vector4f_t& _v2, const framebuffer_t::color_t& _color);

    /**
     * @brief 填充三角形
     * @param  _v0              第一个顶点
     * @param  _v1              第二个顶点
     * @param  _v2              第三个顶点
     * @param  _color           填充的颜色
     * @todo 多线程支持
     */
    void triangle(const vector4f_t& _v0, const vector4f_t& _v1,
                  const vector4f_t& _v2, const framebuffer_t::color_t& _color);

    /**
     * @brief 绘制整个模型，指定变换矩阵
     * @param  _model           模型信息
     * @todo
     * @param  _rotate          在默认变换的基础上进行变换的旋转矩阵，默认为 1
     * @param  _scale           在默认变换的基础上进行变换的缩放矩阵，默认为 1
     * @param  _translate       在默认变换的基础上进行变换的平移矩阵，默认为 1
     */
    void model(const model_t& _model, const matrix4f_t& _rotate = matrix4f_t(),
               const matrix4f_t& _scale     = matrix4f_t(),
               const matrix4f_t& _translate = matrix4f_t());
};

#endif /* _DRAW3D_H_ */
