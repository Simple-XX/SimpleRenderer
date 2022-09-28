
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
 * @todo 大小不一样的赋值处理
 */
class framebuffer_t {
public:
    /// 颜色类型，格式为 ARGB32
    typedef uint32_t color_t;
    /// 每像素字节数
    static constexpr const uint8_t BPP = sizeof(color_t);
    /**
     * @brief 颜色缓冲抽象
     * @tparam _T 颜色缓冲元素类型
     */
    template <class _T = color_t>
    class color_buffer_t {
    private:
        /// @brief 颜色缓冲数组
        _T *color_arr;
        /// @brief 宽度
        int32_t width;
        /// @brief 高度
        int32_t height;

    public:
        /**
         * @brief 空构造函数，不用
         */
        color_buffer_t(void) = delete;

        /**
         * @brief 构造函数
         * @param  _color_buffer    另一个 color_buffer_t
         */
        color_buffer_t(const color_buffer_t<_T> &_color_buffer);

        /**
         * @brief 构造函数
         * @param  _w               宽度
         * @param  _h               高度
         * @param  _color           要填充的颜色
         */
        color_buffer_t(const int32_t _w, const int32_t _h,
                       const _T &_color = 0x00000000);

        /**
         * @brief 析构函数
         */
        ~color_buffer_t(void);
        /**
         * @brief 赋值
         * @param  _color_buffer    另一个 color_buffer_t
         * @return color_buffer_t<_T>&  结果
         */
        color_buffer_t<_T> &operator=(const color_buffer_t<_T> &_color_buffer);

        /**
         * @brief () 重载，获取第 _x 行 _y 列的数据
         * @param  _x               行
         * @param  _y               列
         * @return _T&              数据
         */
        _T &operator()(const int32_t _x, const int32_t _y);
        /**
         * @brief () 重载，获取第 _x 行 _y 列的数据
         * @param  _x               行
         * @param  _y               列
         * @return const _T         只读的数据
         */
        const _T operator()(const int32_t _x, const int32_t _y) const;
        /**
         * @brief 清空，设置为指定值，默认为 0x00000000
         * @param  _color           颜色
         */
        void clear(const _T &_color = 0x00000000);
        /**
         * @brief 获取缓冲区大小(字节数)
         * @return size_t           字节数
         */
        size_t length(void) const;
    };

    /// 深度类型
    typedef float depth_t;

    /**
     * @brief 深度缓冲抽象
     * @tparam _T 深度缓冲元素类型
     */
    template <class _T = depth_t>
    class depth_buffer_t {
    private:
        /// @brief 深度缓冲数组
        _T *depth_arr;
        /// @brief 宽度
        int32_t width;
        /// @brief 高度
        int32_t height;

    public:
        /**
         * @brief 空构造函数，不用
         */
        depth_buffer_t(void) = delete;

        /**
         * @brief 构造函数
         * @param  _depth_buffer    另一个 depth_buffer_t
         */
        depth_buffer_t(const depth_buffer_t<_T> &_depth_buffer);

        /**
         * @brief 构造函数
         * @param  _w               宽度
         * @param  _h               高度
         * @param  _depth           要填充的深度
         */
        depth_buffer_t(const int32_t _w, const int32_t _h,
                       const _T &_depth = std::numeric_limits<_T>::lowest());

        /**
         * @brief 析构函数
         */
        ~depth_buffer_t(void);
        /**
         * @brief 赋值
         * @param  _depth_buffer    另一个 depth_buffer_t
         * @return depth_buffer_t<_T>&  结果
         */
        depth_buffer_t<_T> &operator=(const depth_buffer_t<_T> &_depth_buffer);

        /**
         * @brief () 重载，获取第 _x 行 _y 列的数据
         * @param  _x               行
         * @param  _y               列
         * @return _T&              数据
         */
        _T &operator()(const int32_t _x, const int32_t _y);
        /**
         * @brief () 重载，获取第 _x 行 _y 列的数据
         * @param  _x               行
         * @param  _y               列
         * @return const _T         只读的数据
         */
        const _T operator()(const int32_t _x, const int32_t _y) const;
        /**
         * @brief 清空，设置为指定值，默认为 std::numeric_limits<_T>::lowest()
         * @param  _depth           值
         */
        void clear(const _T &_depth = std::numeric_limits<_T>::lowest());
        /**
         * @brief 获取缓冲区大小(字节数)
         * @return size_t           字节数
         */
        size_t length(void) const;
    };

private:
    /// 窗口宽度
    int32_t width;
    /// 窗口高度
    int32_t height;

    /// 颜色缓存锁
    std::mutex color_buffer_mutex;

    /// 颜色缓存
    color_buffer_t<color_t> color_buffer;

    /// 深度缓存锁
    std::mutex depth_buffer_mutex;

    /// 深度缓存
    depth_buffer_t<depth_t> depth_buffer;

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
    framebuffer_t(const int32_t _width, const int32_t _height);

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
     * @return framebuffer_t&   结果
     */
    framebuffer_t &operator=(const framebuffer_t &_framebuffer);

    /**
     * @brief 获取缓冲区宽度
     * @return int32_t          缓冲区宽度
     */
    int32_t get_width(void) const;

    /**
     * @brief 获取缓冲区高度
     * @return int32_t          缓冲区高度
     */
    int32_t get_height(void) const;

    /**
     * @brief 清空
     */
    void clear(void);

    /**
     * @brief 清空成指定颜色
     * @param  _color           指定颜色
     * @param  _depth           指定深度
     */
    void clear(const color_t &_color,
               const depth_t &_depth = std::numeric_limits<depth_t>::lowest());

    /**
     * @brief 设置像素
     * @param  _x               横向坐标
     * @param  _y               纵向坐标
     * @param  _color           颜色
     * @param  _depth           深度
     * @note (0, 0) 在屏幕左上角
     */
    void pixel(const int32_t _x, const int32_t _y, const color_t &_color,
               const depth_t &_depth = std::numeric_limits<depth_t>::lowest());

    /**
     * @brief 获取颜色缓存
     * color_buffer_t<color_t> &颜色缓存
     */
    color_buffer_t<color_t> &get_color_buffer(void);

    /**
     * @brief 获取颜色缓存
     * const color_buffer_t<color_t> &  只读的颜色缓存
     */
    const color_buffer_t<color_t> &get_color_buffer(void) const;

    /**
     * @brief 获取深度缓存
     * depth_buffer_t<depth_t> &深度缓存
     */
    depth_buffer_t<depth_t> &get_depth_buffer(void);

    /**
     * @brief 获取深度缓存
     * const depth_buffer_t<depth_t> &  只读的深度缓存
     */
    const depth_buffer_t<depth_t> &get_depth_buffer(void) const;

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
