
/**
 * @file color.h
 * @brief 颜色抽象
 * @author Zone.N (Zone.Niuzh@hotmail.com)
 * @version 1.0
 * @date 2022-1-02
 * @copyright MIT LICENSE
 * https://github.com/Simple-XX/SimpleRenderer
 * @par change log:
 * <table>
 * <tr><th>Date<th>Author<th>Description
 * <tr><td>2022-1-02<td>Zone.N<td>创建文件
 * </table>
 */

#ifndef _COLOR_H_
#define _COLOR_H_

#include "cstdint"
#include "iostream"

#include "vector.hpp"

/**
 * @brief 32 位颜色 RGBA
 */
class color_t {
public:
    /// @brief 颜色深度
    static constexpr const uint8_t DEPTH = 4;

public:
    /// @brief 颜色数据，rgba
    uint8_t color_data[DEPTH];

public:
    static color_t WHITE;
    static color_t BLACK;
    static color_t RED;
    static color_t GREEN;
    static color_t BLUE;

    enum color_idx_t {
        COLOR_IDX_R = 0,
        COLOR_IDX_G = 1,
        COLOR_IDX_B = 2,
        COLOR_IDX_A = 3,
    };

    /**
     * @brief 构造函数
     */
    color_t(void);

    /**
     * @brief 构造函数
     * @param  _data            颜色数据
     */
    explicit color_t(const uint32_t _data);

    /**
     * @brief 构造函数
     * @param  _r               红色
     * @param  _g               绿色
     * @param  _b               蓝色
     * @param  _a               alpha
     * @param  _color_order     颜色顺序，默认为 RGBA
     */
    explicit color_t(const uint8_t _r, const uint8_t _g, const uint8_t _b,
                     const uint8_t _a = std::numeric_limits<uint8_t>::max());

    /**
     * @brief 构造函数，从 [0, 1] 构建
     * @param  _r               红色
     * @param  _g               绿色
     * @param  _b               蓝色
     * @param  _a               alpha
     */
    explicit color_t(const float& _r, const float& _g, const float& _b,
                     const uint8_t _a = std::numeric_limits<uint8_t>::max());

    /**
     * @brief 构造函数
     * @param  _color  另一个 color_t
     */
    explicit color_t(const color_t& _color);

    /**
     * @brief 析构函数
     */
    ~color_t(void);

    /**
     * @brief = 重载
     * @param  _color  另一个 color_t
     * @return color_t& 结果
     */
    color_t&             operator=(const color_t& _color);

    const color_t        operator*(const float& _f) const;

    const color_t        operator*(const vector4f_t& _vector4f) const;

    color_t&             operator*=(const float& _f);

    color_t&             operator*=(const vector4f_t& _vector4f);

    /**
     * @brief 下标重载
     * @param  _idx             索引
     * @return uint8_t&         对应颜色
     */
    uint8_t&             operator[](const uint8_t _idx);

    /**
     * @brief 下标重载
     * @param  _idx             索引
     * @return uint8_t          对应颜色
     */
    uint8_t              operator[](const uint8_t _idx) const;

    /**
     * @brief 获取颜色大小
     * @param  _idx             索引
     * @return size_t           颜色大小
     */
    static size_t        size(void);

    /**
     * @brief 转换为 uint32_t
     * @return uint32_t         结果
     */
    uint32_t             to_uint32(void) const;

    /**
     * @brief 转换为指针
     * @return const uint8_t*   结果
     */
    const uint8_t*       to_arr(void) const;

    friend std::ostream& operator<<(std::ostream& _os, const color_t& _color) {
        _os << std::hex;
        _os << "RGBA[ 0x" << +_color[0] << ", 0x" << +_color[1] << ", 0x"
            << +_color[2] << ", 0x" << +_color[3] << " ]";
        _os << std::dec;

        return _os;
    }
};

#endif /* _COLOR_H_ */
