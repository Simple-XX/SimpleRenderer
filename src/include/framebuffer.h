
/**
 * @file framebuffer.h
 * @brief 缓冲区抽象
 * @author Zone.N (Zone.Niuzh@hotmail.com)
 * @version 1.0
 * @date 2022-09-03
 * @copyright MIT LICENSE
 * https://github.com/Simple-XX/SimpleRenderer
 * @par change log:
 * <table>
 * <tr><th>Date<th>Author<th>Description
 * <tr><td>2022-09-03<td>Zone.N<td>创建文件
 * </table>
 */

#ifndef _FRAMEBUFFER_H_
#define _FRAMEBUFFER_H_

#include "cstdint"

/**
 * @brief 缓冲
 */
class framebuffer_t {
public:
    /// 默认宽度
    static constexpr const uint32_t WIDTH = 1920;
    /// 默认高度
    static constexpr const uint32_t HEIGHT = 1080;

    /// 颜色类型，格式为 RGBA32
    typedef uint32_t color_t;

    /// 深度类型
    typedef float depth_buffer_t;

private:
    /// 窗口宽度
    uint32_t width;
    /// 窗口高度
    uint32_t height;

    /// 颜色缓存
    color_t *color_buffer;
    /// 每像素字节数
    static constexpr const uint8_t BPP = sizeof(color_t);

    /// 深度缓存
    depth_buffer_t *depth_buffer;

public:
    /**
     * @brief 构造函数
     */
    framebuffer_t(void);

    /**
     * @brief 构造函数
     * @param  _width           宽度
     * @param  _height          高度
     * @param  _color_type      颜色类型
     */
    framebuffer_t(uint32_t _width = WIDTH, uint32_t _height = HEIGHT);

    /**
     * @brief 拷贝构造
     * @param  _framebuffer     另一个 framebuffer_t
     */
    framebuffer_t(const framebuffer_t &_framebuffer);

    /**
     * @brief 析构函数
     */
    ~framebuffer_t(void);

    /**
     * @brief 赋值
     * @param  _framebuffer     另一个 framebuffer_t
     * @return framebuffer_t&     结果
     */
    framebuffer_t &operator=(const framebuffer_t &_framebuffer);

    /**
     * @brief 清空
     */
    void clear(void);

    /**
     * @brief 清空成指定颜色
     * @param  _color           指定颜色
     */
    void clear(const color_t &_color);

    /**
     * @brief 设置像素
     * @param  _x               横向坐标
     * @param  _y               纵向坐标
     * @param  _color           颜色
     * @note (0, 0) 在屏幕左上角
     */
    void pixel(int _x, int _y, const color_t &_color);

    /**
     * @brief 获取像素缓存
     * @return const color_t*   只读的像素缓存
     */
    const color_t *get_color_buffer(void) const;

    /**
     * @brief 生成 rgba
     * @param  _r               红
     * @param  _g               绿
     * @param  _b               蓝
     * @param  _a               alpha
     * @return color_t          rgba 颜色
     */
    static color_t RGBA(const uint8_t _r, const uint8_t _g, const uint8_t _b,
                        const uint8_t _a = UINT8_MAX);
};

#endif /* _FRAMEBUFFER_H_ */
