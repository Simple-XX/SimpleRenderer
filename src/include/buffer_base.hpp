
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

#include <cstddef>
#include <cstdint>
#include <immintrin.h>
#include <memory>
#include <mutex>
#include <type_traits>

#include "color.h"
#include "log.hpp"

/**
 * 缓冲区，作为 framebuffer 或 zbuffer 的基类
 * @todo 大小不一样的赋值处理
 * @todo get() 方法能否优化
 * @todo 多线程安全
 */
template <class T_t> class buffer_base_t {
public:
  /// 每个像素的字节数
  static constexpr const size_t BPP = sizeof(T_t);

  /**
   * 构造函数，整型默认为白色，浮点默认为最小值
   * @param _width 宽度
   * @param _height 高度
   * @param _value 值
   */
  explicit buffer_base_t(uint32_t _width, uint32_t _height,
                         const T_t &_value = T_t())
      : width(_width), height(_height),
        buffer(std::vector<T_t>(width * height, _value)) {}

  virtual ~buffer_base_t() {
    width = 0;
    height = 0;
  }

  /// @name 默认构造/析构函数
  /// @{
  buffer_base_t() = default;
  buffer_base_t(const buffer_base_t &_buffer_base) = default;
  buffer_base_t(buffer_base_t &&_buffer_base) = default;
  auto operator=(const buffer_base_t &_buffer_base)
      -> buffer_base_t & = default;
  auto operator=(buffer_base_t &&_buffer_base) -> buffer_base_t & = default;
  /// @}

  /**
   * 获取缓冲区宽度
   * @return 缓冲区宽度
   */
  [[nodiscard]] auto get_width() const -> uint32_t { return width; }

  /**
   * 获取缓冲区高度
   * @return 缓冲区高度
   */
  [[nodiscard]] auto get_height() const -> uint32_t { return height; }

  /**
   * 清空，如果 T_t 为 int，设置为黑色，否则视为深度缓冲，设为最小值
   */
  void clear() {
    if constexpr (std::is_same_v<T_t, color_t>) {
      std::fill_n(buffer.get(), width * height, color_t::BLACK);
    } else if ((std::is_same_v<T_t, float>) || (std::is_same_v<T_t, double>)) {
      std::fill_n(buffer.get(), width * height,
                  std::numeric_limits<T_t>::lowest());
    }
  }

  /**
   * () 重载，获取第 _row 行 _col 列的数据
   * @param _row 行
   * @param _col 列
   * @return 数据
   */
  auto operator()(uint32_t _row, uint32_t _col) -> T_t & {
    return buffer[_row * width + _col];
  }

  /**
   * () 重载，获取第 _row 行 _col 列的数据
   * @param _row 行
   * @param _col 列
   * @return 只读的数据
   */
  auto operator()(uint32_t _row, uint32_t _col) const -> const T_t & {
    return buffer[_row * width + _col];
  }

  /**
   * 转换为数组
   * @return 数组
   */
  auto data() -> T_t * { return buffer.data(); }

  /**
   * 转换为数组
   * @return 数组
   */
  auto data() const -> const T_t * { return buffer.data(); }

  /**
   * 获取缓冲区大小(字节数)
   * @return 字节数
   */
  [[nodiscard]] auto length() const -> size_t { return buffer.size() * BPP; }

private:
  /// 窗口宽度
  size_t width = 0;
  /// 窗口高度
  size_t height = 0;

  /// 缓冲区锁
  /// @todo 这个锁似乎可以优化掉
  //  std::mutex buffer_mutex;
  /// 缓冲数组
  std::vector<T_t> buffer;
};

#endif /* BUFFER_BASE_HPP */
