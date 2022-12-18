
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

#ifndef SIMPLERENDER_FRAMEBUFFER_H
#define SIMPLERENDER_FRAMEBUFFER_H

#include "array"
#include "cstdint"
#include "mutex"

#include "buffer_base.hpp"
#include "color.h"

/**
 * @brief 帧缓冲
 * @todo 大小不一样的赋值处理
 */
class framebuffer_t {
public:
    /// @brief 深度类型
    /// @note 不要进行会影响内存的修改
    typedef float depth_t;

private:
    /// @brief 窗口宽度
    uint32_t       width;
    /// @brief 窗口高度
    uint32_t       height;

    /// @brief 颜色缓冲区锁
    std::mutex     color_buffer_mutex;
    /// @brief 颜色缓冲区
    color_buffer_t color_buffer;

    /// @brief 深度缓冲区锁
    std::mutex     depth_buffer_mutex;
    /// @brief 深度缓冲区

    depth_buffer_t depth_buffer;

public:
    /**
     * @brief 不使用空构造
     */
    framebuffer_t(void) = delete;

    /**
     * @brief 构造函数
     * @param  _width           宽度
     * @param  _height          高度
     */
    framebuffer_t(uint32_t _width, uint32_t _height);

    /**
     * @brief 拷贝构造
     * @param  _framebuffer     另一个 framebuffer_t
     */
    framebuffer_t(const framebuffer_t& _framebuffer);

    /**
     * @brief 析构函数
     */
    ~framebuffer_t(void);

    /**
     * @brief 赋值
     * @param  _framebuffer     另一个 framebuffer_t
     * @return framebuffer_t&   结果
     */
    framebuffer_t& operator=(const framebuffer_t& _framebuffer);

    /**
     * @brief 获取缓冲区宽度
     * @return uint32_t         缓冲区宽度
     */
    uint32_t       get_width(void) const;

    /**
     * @brief 获取缓冲区高度
     * @return uint32_t         缓冲区高度
     */
    uint32_t       get_height(void) const;

    /**
     * @brief 清空成指定颜色
     * @param  _color           指定颜色
     * @param  _depth           指定深度
     */
    void           clear(const color_t& _color = color_t(),
                         const depth_t& _depth = std::numeric_limits<depth_t>::lowest());

    /**
     * @brief 设置像素
     * @param  _x               横向坐标
     * @param  _y               纵向坐标
     * @param  _color           颜色
     * @param  _depth           深度
     * @note (0, 0) 在屏幕左上角
     */
    void                  pixel(uint32_t _x, uint32_t _y, const color_t& _color,
                                const depth_t& _depth = std::numeric_limits<depth_t>::lowest());

    /**
     * @brief 获取颜色缓存
     * @return color_buffer_t&  颜色缓存
     */
    color_buffer_t&       get_color_buffer(void);

    /**
     * @brief 获取颜色缓存
     * @return const color_buffer_t&    只读的颜色缓存
     */
    const color_buffer_t& get_color_buffer(void) const;

    /**
     * @brief 获取深度缓存
     * @return depth_buffer_t&  深度缓存
     */
    depth_buffer_t&       get_depth_buffer(void);

    /**
     * @brief 获取深度值
     * @return depth_t&         深度值
     */
    depth_t&              get_depth_buffer(uint32_t _x, uint32_t _y);

    /**
     * @brief 获取深度缓存
     * const depth_buffer_t&    只读的深度缓存
     */
    const depth_buffer_t& get_depth_buffer(void) const;

    /**
     * @brief 获取深度值
     * @return depth_t          深度值
     */
    depth_t               get_depth_buffer(uint32_t _x, uint32_t _y) const;
};

#endif /* SIMPLERENDER_FRAMEBUFFER_H */
