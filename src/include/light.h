
/**
 * @file light.h
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

#ifndef SIMPLERENDER_LIGHT_H
#define SIMPLERENDER_LIGHT_H

#include "cstdint"

#include "color.h"
#include "vector.hpp"

/**
 * @brief 光照抽象
 */
class light_t {
private:
    /// @brief 默认位置
    const vector4f_t DEFAULT_POS   = vector4f_t(0, 0, 0);
    /// @brief 默认方向，左手系，x 向右，y 向下，z 正方向为屏幕由内向外
    const vector4f_t DEFAULT_DIR   = vector4f_t(0, 0, -1);
    /// @brief 默认颜色
    const color_t    DEFAULT_COLOR = color_t(color_t::WHITE);

public:
    /// @brief 位置
    vector4f_t pos;
    /// @brief 方向
    vector4f_t dir;
    /// @brief 颜色
    color_t    color;

    /**
     * @brief 空构造函数
     */
    light_t(void);

    /**
     * @brief 构造函数
     */
    light_t(const light_t& _light);

    /**
     * @brief 析构函数
     */
    ~light_t(void);

    /**
     * @brief 赋值
     * @param  _light            另一个 light
     * @return light_t&          结果
     */
    light_t& operator=(const light_t& _light);
};

#endif /* SIMPLERENDER_LIGHT_H */
