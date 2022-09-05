
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
#include "mutex"

/**
 * @brief 缓冲
 */
class framebuffer_t {
private:
    std::mutex color_buffer_mutex;
    std::mutex depth_buffer_mutex;

public:
    /// 颜色类型，格式为 ARGB32
    typedef uint32_t color_t;

    /// 深度类型
    typedef float depth_t;

    /// 窗口宽度
    uint32_t width;
    /// 窗口高度
    uint32_t height;

    /// 颜色缓存
    color_t *color_buffer;
    /// 每像素字节数
    static constexpr const uint8_t BPP = sizeof(color_t);
    /// 每像素深度大小
    static constexpr const uint8_t BPP_DEPTH = sizeof(depth_t);

    /// 深度缓存
    depth_t *depth_buffer;

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
    framebuffer_t(uint32_t _width, uint32_t _height);

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
     * @param  _depth           指定深度
     */
    void clear(const color_t &_color, const depth_t &_depth = 0);

    /**
     * @brief 设置像素
     * @param  _i               横向坐标
     * @param  _j               纵向坐标
     * @param  _color           颜色
     * @param  _depth           深度
     * @note (0, 0) 在屏幕左上角
     */
    void pixel(const uint32_t _i, const uint32_t _j, const color_t &_color,
               const depth_t &_depth = 0);

    /**
     * @brief 获取像素缓存
     * @return const color_t*   只读的像素缓存
     */
    const color_t *get_color_buffer(void) const;

    /**
     * @brief 生成 argb
     * @param  _a               alpha
     * @param  _r               红
     * @param  _g               绿
     * @param  _b               蓝
     * @return color_t          argb 颜色
     */
    static color_t ARGB(const uint8_t _a, const uint8_t _r, const uint8_t _g,
                        const uint8_t _b);
};

#endif /* _FRAMEBUFFER_H_ */
