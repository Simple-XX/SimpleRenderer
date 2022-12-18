
/**
 * @file buffer_base.hpp
 * @brief 缓冲区基类
 * @author Zone.N (Zone.Niuzh@hotmail.com)
 * @version 1.0
 * @date 2022-12-08
 * @copyright MIT LICENSE
 * https://github.com/Simple-XX/SimpleRenderer
 * @par change log:
 * <table>
 * <tr><th>Date<th>Author<th>Description
 * <tr><td>2022-12-08<td>Zone.N<td>创建文件
 * </table>
 */

#ifndef BUFFER_BASE_HPP
#define BUFFER_BASE_HPP

#include "cstdint"
#include "mutex"
#include "type_traits"

#include "color.h"
#include "log.hpp"

/**
 * @brief 缓冲区，作为 framebuffer 或 zbuffer 的基类
 * @todo 大小不一样的赋值处理
 * @todo get() 方法能否优化
 */
template <class T_t>
class buffer_base_t {
private:
    /// @brief 窗口宽度
    uint32_t               width;
    /// @brief 窗口高度
    uint32_t               height;

    /// @brief 缓冲区锁
    std::mutex             buffer_mutex;
    /// @brief 缓冲数组
    std::shared_ptr<T_t[]> buffer_arr;

public:
    /**
     * @brief 空构造
     */
    buffer_base_t(void);

    /**
     * @brief 拷贝构造
     * @param  _buffer          另一个 buffer_base_t
     */
    buffer_base_t(const buffer_base_t& _buffer);

    /**
     * @brief 构造函数，整型默认为白色，浮点默认为最小值
     * @param  _width           宽度
     * @param  _height          高度
     */
    explicit buffer_base_t(uint32_t _width, uint32_t _height);

    /**
     * @brief 析构函数
     */
    virtual ~buffer_base_t(void);

    /**
     * @brief 赋值
     * @param  _buffer     另一个 buffer_base_t
     * @return buffer_base_t&   结果
     */
    buffer_base_t& operator=(const buffer_base_t& _buffer);

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
     * @brief 清空，如果 T_t 为 int，设置为黑色，否则视为深度缓冲，设为最小值
     */
    void           clear(void);

    /**
     * @brief () 重载，获取第 _x 行 _y 列的数据
     * @param  _x               行
     * @param  _y               列
     * @return T_t&             数据
     */
    T_t&           operator()(uint32_t _x, uint32_t _y);

    /**
     * @brief () 重载，获取第 _x 行 _y 列的数据
     * @param  _x               行
     * @param  _y               列
     * @return const T_t&       只读的数据
     */
    const T_t&     operator()(uint32_t _x, uint32_t _y) const;

    /**
     * @brief 转换为数组
     * @return T_t*             数组
     */
    T_t*           to_arr(void);

    /**
     * @brief 转换为数组
     * @return const T_t*       数组
     */
    const T_t*     to_arr(void) const;

    /**
     * @brief 获取缓冲区大小(字节数)
     * @return size_t           字节数
     */
    size_t         length(void) const;
};

template <class T_t>
buffer_base_t<T_t>::buffer_base_t(void) : width(0), height(0) {
    buffer_arr = nullptr;

    return;
}

template <class T_t>
buffer_base_t<T_t>::buffer_base_t(const buffer_base_t& _buffer)
    : width(_buffer.width), height(_buffer.height) {
    try {
        buffer_arr = std::make_shared<T_t[]>(width * height);
    } catch (const std::bad_alloc& e) {
        std::cerr << log(e.what()) << std::endl;
    }
    // 复制数据
    std::copy(_buffer.buffer_arr.get(),
              _buffer.buffer_arr.get() + _buffer.length(), buffer_arr.get());
    return;
}

template <class T_t>
buffer_base_t<T_t>::buffer_base_t(uint32_t _w, uint32_t _h)
    : width(_w), height(_h) {
    try {
        buffer_arr = std::make_shared<T_t[]>(width * height);
    } catch (const std::bad_alloc& e) {
        std::cerr << log(e.what()) << std::endl;
    }
    if constexpr (std::is_same_v<T_t, color_t>) {
        std::fill_n(buffer_arr.get(), width * height, color_t::BLACK);
    }
    else if constexpr ((std::is_same_v<T_t, float>)
                       || (std::is_same_v<T_t, double>)) {
        std::fill_n(buffer_arr.get(), width * height,
                    std::numeric_limits<T_t>::lowest());
    }

    return;
}

template <class T_t>
buffer_base_t<T_t>::~buffer_base_t(void) {
    width  = 0;
    height = 0;
    return;
}

template <class T_t>
buffer_base_t<T_t>&
buffer_base_t<T_t>::operator=(const buffer_base_t& _buffer) {
    if (&_buffer == this) {
        return *this;
    }

    if (width != _buffer.width || height != _buffer.height) {
        width  = _buffer.width;
        height = _buffer.height;
        try {
            buffer_arr = std::make_shared<T_t[]>(width * height);
        } catch (const std::bad_alloc& e) {
            std::cerr << log(e.what()) << std::endl;
        }
    }
    std::copy(_buffer.buffer_arr.get(),
              _buffer.buffer_arr.get() + width * height * sizeof(color_t),
              buffer_arr.get());

    return *this;
}

template <class T_t>
uint32_t buffer_base_t<T_t>::get_height(void) const {
    return height;
}

template <class T_t>
uint32_t buffer_base_t<T_t>::get_width(void) const {
    return width;
}

template <class T_t>
void buffer_base_t<T_t>::clear(void) {
    if constexpr (std::is_same_v<T_t, color_t>) {
        std::fill_n(buffer_arr.get(), width * height, color_t::BLACK);
    }
    else if ((std::is_same_v<T_t, float>) || (std::is_same_v<T_t, double>)) {
        std::fill_n(buffer_arr.get(), width * height,
                    std::numeric_limits<T_t>::lowest());
    }
    return;
}

template <class T_t>
T_t& buffer_base_t<T_t>::operator()(uint32_t _x, uint32_t _y) {
    return buffer_arr[_y * width + _x];
}

template <class T_t>
const T_t& buffer_base_t<T_t>::operator()(uint32_t _x, uint32_t _y) const {
    return buffer_arr[_y * width + _x];
}

template <class T_t>
T_t* buffer_base_t<T_t>::to_arr(void) {
    return buffer_arr.get();
}

template <class T_t>
const T_t* buffer_base_t<T_t>::to_arr(void) const {
    return buffer_arr.get();
}

template <class T_t>
size_t buffer_base_t<T_t>::length(void) const {
    return width * height * sizeof(T_t);
}

typedef buffer_base_t<color_t> color_buffer_t;
typedef buffer_base_t<float>   depth_buffer_t;

#endif /* BUFFER_BASE_HPP */
