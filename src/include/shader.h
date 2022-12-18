
/**
 * @file shader.h
 * @brief 着色器头文件
 * @author Zone.N (Zone.Niuzh@hotmail.com)
 * @version 1.0
 * @date 2022-10-15
 * @copyright MIT LICENSE
 * https://github.com/Simple-XX/SimpleRenderer
 * @par change log:
 * <table>
 * <tr><th>Date<th>Author<th>Description
 * <tr><td>2022-10-15<td>Zone.N<td>创建文件
 * </table>
 */

#ifndef SIMPLERENDER_SHADER_H
#define SIMPLERENDER_SHADER_H

#include "framebuffer.h"
#include "matrix.hpp"
#include "model.h"
#include "vector.hpp"

/**
 * @brief 顶点着色器输入数据
 */
class shader_vertex_in_t {
public:
    /// @brief 面信息
    model_t::face_t face;

    /**
     * @brief 构造函数
     */
    shader_vertex_in_t(void);

    /**
     * @brief 构造函数
     * @param  _face            面信息
     */
    explicit shader_vertex_in_t(const model_t::face_t _face);

    /**
     * @brief 构造函数
     * @param  _shader_vertex_in  另一个 shader_vertex_in_t
     */
    explicit shader_vertex_in_t(const shader_vertex_in_t& _shader_vertex_in);

    /**
     * @brief 析构函数
     */
    ~shader_vertex_in_t(void);

    /**
     * @brief = 重载
     * @param  _shader_vertex_in  另一个 shader_vertex_in_t
     * @return shader_vertex_in_t&  结果
     */
    shader_vertex_in_t& operator=(const shader_vertex_in_t& _shader_vertex_in);
};

/**
 * @brief 顶点着色器输出数据
 */
class shader_vertex_out_t {
public:
    /// @brief 面信息
    model_t::face_t face;

    /**
     * @brief 构造函数
     */
    shader_vertex_out_t(void);

    /**
     * @brief 构造函数
     * @param  _face            面信息
     */
    explicit shader_vertex_out_t(const model_t::face_t _face);

    /**
     * @brief 构造函数
     * @param  _shader_vertex_out 另一个 shader_vertex_out_t
     */
    shader_vertex_out_t(const shader_vertex_out_t& _shader_vertex_out);

    /**
     * @brief 析构函数
     */
    ~shader_vertex_out_t(void);

    /**
     * @brief = 重载
     * @param  _shader_vertex_out   另一个 shader_vertex_out_t
     * @return shader_vertex_out_t& 结果
     */
    shader_vertex_out_t&
    operator=(const shader_vertex_out_t& _shader_vertex_out);
};

/**
 * @brief 片段着色器输入
 */
class shader_fragment_in_t {
public:
    /// @brief 重心坐标
    vector4f_t barycentric_coord;
    /// @brief 法线方向
    vector4f_t normal;
    /// @brief 光照方向
    vector4f_t light;

    color_t    color0;
    color_t    color1;
    color_t    color2;

    /**
     * @brief 构造函数
     */
    shader_fragment_in_t(void);

    /**
     * @brief 构造函数
     * @param  _barycentric_coord   重心坐标
     * @param  _normal              法线方向
     * @param  _light               光照方向
     */
    explicit shader_fragment_in_t(const vector4f_t& _barycentric_coord,
                                  const vector4f_t& _normal,
                                  const vector4f_t& _light,
                                  const color_t&    _color0,
                                  const color_t&    _color1,
                                  const color_t&    _color2);

    /**
     * @brief 构造函数
     * @param  _shader_fragment_in  另一个 shader_fragment_in_t
     */
    shader_fragment_in_t(const shader_fragment_in_t& _shader_fragment_in);

    /**
     * @brief 析构函数
     */
    ~shader_fragment_in_t(void);

    /**
     * @brief = 重载
     * @param  _shader_fragment_in  另一个 shader_fragment_in_t
     * @return shader_fragment_in_t&  结果
     */
    shader_fragment_in_t&
    operator=(const shader_fragment_in_t& _shader_fragment_in);
};

/**
 * @brief 片段着色器输出
 */
class shader_fragment_out_t {
public:
    /// @brief 是否需要绘制
    bool    is_need_draw;

    /// @brief 颜色
    color_t color;

    /**
     * @brief 构造函数
     */
    shader_fragment_out_t(void);

    /**
     * @brief 构造函数
     * @param  _is_need_draw        是否需要绘制
     * @param  _color               颜色
     */
    explicit shader_fragment_out_t(const bool&    _is_need_draw,
                                   const color_t& _color);

    /**
     * @brief 构造函数
     * @param  _shader_fragment_out 另一个 shader_fragment_out_t
     */
    shader_fragment_out_t(const shader_fragment_out_t& _shader_fragment_out);

    /**
     * @brief 析构函数
     */
    ~shader_fragment_out_t(void);

    /**
     * @brief = 重载
     * @param  _shader_fragment_out 另一个 shader_fragment_out_t
     * @return shader_fragment_out_t& 结果
     */
    shader_fragment_out_t&
    operator=(const shader_fragment_out_t& _shader_fragment_out);
};

/**
 * @brief 着色器数据
 */
class shader_data_t {
public:
    /// @brief 模型变换矩阵
    matrix4f_t model_matrix;
    /// @brief 视图变换矩阵
    matrix4f_t view_matrix;
    /// @brief 正交变换矩阵
    matrix4f_t project_matrix;

    /**
     * @brief 构造函数
     */
    shader_data_t(void);

    /**
     * @brief 构造函数
     * @param  _shader_data     另一个 shader_data_t
     */
    shader_data_t(const shader_data_t& _shader_data);

    /**
     * @brief 析构函数
     */
    ~shader_data_t(void);

    /**
     * @brief = 重载
     * @param  _shader_data     另一个 shader_data_t
     * @return shader_data_t&   结果
     */
    shader_data_t& operator=(const shader_data_t& _shader_data);
};

/**
 * @brief 着色器基类
 */
class shader_base_t {
public:
    /// @brief 着色器数据
    shader_data_t shader_data;

    /**
     * @brief 构造函数
     */
    shader_base_t(void);

    /**
     * @brief 析构函数
     */
    virtual ~shader_base_t(void);

    /**
     * @brief 顶点着色器
     * 处理顶点的着色器，所有顶点被载入管线后都会经由顶点着色器处理后载入管线的下一步。
     * 通过顶点着色器可以改变绘制内容的形状，包括形状大小、位置、角度、投影等等。
     * @param  _shader_vertex_in  顶点着色器输入
     * @return shader_vertex_out_t  顶点着色器输出
     */
    virtual shader_vertex_out_t
    vertex(const shader_vertex_in_t& _shader_vertex_in) const
      = 0;

    /**
     * @brief 片段着色器
     * 片段着色器负责处理绘制内容的颜色，所有顶点光栅化后经由片段着色器都会被赋值颜色，并且也会被插值处理。
     * @param  _shader_fragment_in  片段着色器输入
     * @return shader_fragment_out_t    片段着色器输出
     */
    virtual shader_fragment_out_t
    fragment(const shader_fragment_in_t& _shader_fragment_in) const
      = 0;
};

#endif /* SIMPLERENDER_SHADER_H */
