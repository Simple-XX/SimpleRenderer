
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

#ifndef _SHADER_H_
#define _SHADER_H_

#include "string"

#include "framebuffer.h"
#include "matrix.hpp"
#include "model.h"
#include "vector.hpp"

// 着色器基类
class shader_base_t {
private:

public:
    matrix4f_t model_matrix;
    matrix4f_t view_matrix;
    matrix4f_t project_matrix;

    shader_base_t(void);
    virtual ~shader_base_t(void);

    // 顶点着色器
    // 处理顶点的着色器，所有顶点被载入管线后都会经由顶点着色器处理后载入管线的下一步。
    // 通过顶点着色器可以改变绘制内容的形状，包括形状大小、位置、角度、投影等等。
    virtual const model_t::face_t vertex(const model_t::face_t& _face) = 0;

    // 片段着色器
    // 片段着色器负责处理绘制内容的颜色，所有顶点光栅化后经由片段着色器都会被赋值颜色，并且也会被插值处理。
    // 返回是否需要绘制
    virtual bool
    fragment(const vector4f_t&             _barycentric_coord,
             const framebuffer_t::color_t& _color, const vector4f_t& _light_dir)
      = 0;
};

#endif /* _SHADER_H_ */
