
/**
 * @file default_shader.h
 * @brief 默认着色器
 * @author Zone.N (Zone.Niuzh@hotmail.com)
 * @version 1.0
 * @date 2022-11-01
 * @copyright MIT LICENSE
 * https://github.com/Simple-XX/SimpleRenderer
 * @par change log:
 * <table>
 * <tr><th>Date<th>Author<th>Description
 * <tr><td>2022-11-01<td>Zone.N<td>创建文件
 * </table>
 */

#ifndef _DEFAULT_SHADER_H_
#define _DEFAULT_SHADER_H_

#include "shader.h"

/**
 * @brief 默认着色器
 */
class default_shader_t : public shader_base_t {
private:
    /**
     * @brief 计算重心坐标
     * @param  _p0              三角形的第一个点
     * @param  _p1              三角形的第二个点
     * @param  _p2              三角形的第三个点
     * @param  _p               要判断的点
     * @return const std::pair<bool, const vector4f_t>
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
    static const std::pair<bool, const vector4f_t>
    get_barycentric_coord(const vector4f_t& _p0, const vector4f_t& _p1,
                          const vector4f_t& _p2, const vector4f_t& _p);

    /**
     * @brief 深度插值，由重心坐标计算出对应点的深度值
     * @param  _v0              点 1
     * @param  _v1              点 2
     * @param  _v2              点 3
     * @param  _barycentric_coord   重心坐标
     * @return framebuffer_t::depth_t 深度值
     */
    static framebuffer_t::depth_t
    interpolate_depth(const framebuffer_t::depth_t& _v0,
                      const framebuffer_t::depth_t& _v1,
                      const framebuffer_t::depth_t& _v2,
                      const vector4f_t&             _barycentric_coord);

    /**
     * @brief 颜色插值，由重心坐标计算出对应点的颜色，同时会处理光照强度
     * @param  _c0              第一个点的颜色
     * @param  _c1              第二个点的颜色
     * @param  _c2              第三个点的颜色
     * @param  _barycentric_coord   重心坐标
     * @param  _normal          当前点的法向量
     * @return framebuffer_t::color_t 颜色值
     */
    static framebuffer_t::color_t
    interpolate_color(const model_t::color_t& _c0, const model_t::color_t& _c1,
                      const model_t::color_t&  _c2,
                      const vector4f_t&        _barycentric_coord,
                      const model_t::normal_t& _normal);

public:
    /**
     * @brief 构造函数
     */
    default_shader_t(void);

    /**
     * @brief 析构函数
     */
    ~default_shader_t(void);

    /**
     * @brief 顶点着色器
     * @param  _shader_vertex_in    输入
     * @return const shader_vertex_out_t    输出
     */
    const shader_vertex_out_t
    vertex(const shader_vertex_in_t& _shader_vertex_in) override;

    /**
     * @brief 片段着色器
     * @param  _shader_fragment_in  输入
     * @return const shader_fragment_out_t  输出
     */
    const shader_fragment_out_t
    fragment(const shader_fragment_in_t& _shader_fragment_in) override;
};

#endif /* _DEFAULT_SHADER_H_ */
