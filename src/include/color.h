
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

/**
 * @brief 32 位颜色
 */
class color_t {
public:
    /// @brief 颜色顺序
    enum color_order_t : uint8_t {
        COLOR_ORDER_RGBA = 1,
        COLOR_ORDER_ARGB = 0,
    };

private:
    /// @brief 颜色数据，4个 uint8_t
    uint32_t      color_data;

    /// @brief 颜色顺序
    color_order_t color_order;

public:
    /// @brief 颜色深度
    static constexpr const uint8_t DEPTH = 4;

    /**
     * @brief 构造函数
     */
    color_t(void);

    /**
     * @brief 构造函数
     * @param  _data            颜色数据
     * @param  _color_order     颜色顺序，默认为 RGBA
     */
    explicit color_t(const uint32_t       _data,
                     const color_order_t& _color_order = COLOR_ORDER_RGBA);

    /**
     * @brief 构造函数
     * @param  _r               红色
     * @param  _g               绿色
     * @param  _b               蓝色
     * @param  _a               alpha，默认为 0xFF
     * @param  _color_order     颜色顺序，默认为 RGBA
     */
    explicit color_t(const uint8_t _r, const uint8_t _g, const uint8_t _b,
                     const uint8_t        _a           = 0xFF,
                     const color_order_t& _color_order = COLOR_ORDER_RGBA);

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
    size_t               size(void) const;

    /**
     * @brief 转换为 argb 顺序
     * @return const color_t    结果
     */
    const color_t        to_argb(void) const;

    /**
     * @brief 转换为 rgba 顺序
     * @return const color_t    结果
     */
    const color_t        to_rgba(void) const;

    /**
     * @brief 转换为指针
     * @return const uint8_t*   结果
     */
    const uint8_t*       to_arr(void) const;

    friend std::ostream& operator<<(std::ostream& _os, const color_t& _color) {
        if (_color.color_order == COLOR_ORDER_ARGB) {
            _os << "ARGB[ 0x";
        }
        else if (_color.color_order == COLOR_ORDER_RGBA) {
            _os << "RGBA[ 0x";
        }
        _os << std::hex;
        _os << std::hex << +_color[0] << ", 0x" << +_color[1] << ", 0x"
            << +_color[2] << ", 0x" << +_color[3] << " ]";
        _os << std::dec;

        return _os;
    }
};

#endif /* _COLOR_H_ */
