
/**
 * @file matrix4.hpp
 * @brief 四维矩阵
 * @author Zone.N (Zone.Niuzh@hotmail.com)
 * @version 1.0
 * @date 2022-09-07
 * @copyright MIT LICENSE
 * https://github.com/Simple-XX/SimpleRenderer
 * @par change log:
 * <table>
 * <tr><th>Date<th>Author<th>Description
 * <tr><td>2022-09-07<td>Zone.N<td>迁移到 doxygen
 * </table>
 */

#ifndef SIMPLERENDER_MATRIX4_HPP
#define SIMPLERENDER_MATRIX4_HPP

#include "array"
#include "cfloat"
#include "cstring"
#include "iomanip"
#include "iostream"
#include "vector"

#include "Eigen/Dense"

#include "log.hpp"
#include "vector.hpp"

/**
 * @brief 矩阵元素类型约束
 * @tparam T_t 元素类型
 */
template <class T_t>
concept matrix_element_concept_t = std::is_same<T_t, float>::value;

/**
 * @brief 四阶矩阵
 * @tparam T_t 矩阵元素类型
 * @note 没有做数值合法检查
 */
template <matrix_element_concept_t T_t> class matrix4_t {
public:
  /// @brief 阶数
  static constexpr uint8_t ORDER = 4;
  /// @brief 元素个数
  static constexpr uint8_t ELEMS = 16;
  /// @brief 矩阵元素
  std::array<T_t, ELEMS> elems;

  Eigen::Matrix<T_t, ORDER, ORDER> eigen_mat;

  /**
   * @brief 递归求 n 阶行列式的值
   * @param  _order           阶数
   * @return T_t              值
   */
  T_t determ(uint8_t _order) const;

  /**
   * @brief 代数余子式矩阵
   * @param  _row             行
   * @param  _col             列
   * @param  _order           当前阶
   * @return matrix4_t<T_t>   结果
   */
  matrix4_t<T_t> cofactor(uint8_t _row, uint8_t _col, uint8_t _order) const;

  /**
   * @brief 伴随矩阵
   * @return matrix4_t<T_t>   结果
   */
  matrix4_t<T_t> adjoint(void) const;

public:
  /**
   * @brief 构造函数，默认为单位矩阵
   */
  matrix4_t(void);

  /**
   * @brief 构造函数
   * @param  _mat             另一个矩阵
   */
  matrix4_t(const matrix4_t<T_t> &_mat);

  matrix4_t(const Eigen::Matrix<T_t, ORDER, ORDER> &_mat);

  /**
   * @brief 构造函数
   * @param  _arr             指针
   */
  explicit matrix4_t(const T_t *_arr);

  /**
   * @brief 构造函数
   * @param  _arr             数组
   */
  explicit matrix4_t(const T_t _arr[ORDER][ORDER]);

  /**
   * @brief 构造函数，构造齐次坐标，多余位置补 0
   * @param  _v               四维向量
   */
  explicit matrix4_t(const vector4_t<T_t> &_v);

  /**
   * @brief 赋值
   * @param  _mat             另一个 matrix4_t
   * @return matrix4_t<T_t>&    结果
   */
  matrix4_t<T_t> &operator=(const matrix4_t<T_t> &_mat);

  /**
   * @brief 矩阵相等
   * @param  _mat             另一个 matrix4_t
   * @return true             相等
   * @return false            不等
   */
  bool operator==(const matrix4_t<T_t> &_mat) const;

  /**
   * @brief 矩阵不等
   * @param  _mat             另一个 matrix4_t
   * @return true             不等
   * @return false            相等
   */
  bool operator!=(const matrix4_t<T_t> &_mat) const;

  /**
   * @brief 矩阵间加法
   * @param  _mat             另一个 matrix4_t
   * @return matrix4_t<T_t>   结果
   */
  matrix4_t<T_t> operator+(const matrix4_t<T_t> &_mat) const;

  /**
   * @brief 矩阵自加
   * @param  _mat             另一个 matrix4_t
   * @return matrix4_t<T_t>&    结果
   */
  matrix4_t<T_t> &operator+=(const matrix4_t<T_t> &_mat);

  /**
   * @brief 矩阵间减法
   * @param  _mat             另一个 matrix4_t
   * @return matrix4_t<T_t>   结果
   */
  matrix4_t<T_t> operator-(const matrix4_t<T_t> &_mat) const;

  /**
   * @brief 矩阵自减
   * @param  _mat             另一个 matrix4_t
   * @return matrix4_t<T_t>&    结果
   */
  matrix4_t<T_t> &operator-=(const matrix4_t<T_t> &_mat);

  /**
   * @brief 矩阵数乘
   * @tparam U_t 数类型
   * @param  _v               数
   * @param  _mat             矩阵
   * @return matrix4_t<T_t>   结果
   */
  template <class U_t>
  friend matrix4_t<T_t> operator*(const U_t &_v, const matrix4_t<T_t> &_mat) {
    //    T_t tmp[ELEMS] = {0};
    //
    //    tmp[0] = _mat.elems[0] * _v;
    //    tmp[1] = _mat.elems[1] * _v;
    //    tmp[2] = _mat.elems[2] * _v;
    //    tmp[3] = _mat.elems[3] * _v;
    //
    //    tmp[4] = _mat.elems[4] * _v;
    //    tmp[5] = _mat.elems[5] * _v;
    //    tmp[6] = _mat.elems[6] * _v;
    //    tmp[7] = _mat.elems[7] * _v;
    //
    //    tmp[8] = _mat.elems[8] * _v;
    //    tmp[9] = _mat.elems[9] * _v;
    //    tmp[10] = _mat.elems[10] * _v;
    //    tmp[11] = _mat.elems[11] * _v;
    //
    //    tmp[12] = _mat.elems[12] * _v;
    //    tmp[13] = _mat.elems[13] * _v;
    //    tmp[14] = _mat.elems[14] * _v;
    //    tmp[15] = _mat.elems[15] * _v;
    //
    //    return matrix4_t<T_t>(tmp);

    return matrix4_t<T_t>(_mat.eigen_mat * _v);
  }

  /**
   * @brief 矩阵数乘
   * @tparam U_t 数类型
   * @param  _mat             矩阵
   * @param  _v               数
   * @return matrix4_t<T_t>   结果
   */
  template <class U_t>
  friend matrix4_t<T_t> operator*(const matrix4_t<T_t> &_mat, const U_t &_v) {
    //    T_t tmp[ELEMS] = {0};
    //
    //    tmp[0] = _v * _mat.elems[0];
    //    tmp[1] = _v * _mat.elems[1];
    //    tmp[2] = _v * _mat.elems[2];
    //    tmp[3] = _v * _mat.elems[3];
    //
    //    tmp[4] = _v * _mat.elems[4];
    //    tmp[5] = _v * _mat.elems[5];
    //    tmp[6] = _v * _mat.elems[6];
    //    tmp[7] = _v * _mat.elems[7];
    //
    //    tmp[8] = _v * _mat.elems[8];
    //    tmp[9] = _v * _mat.elems[9];
    //    tmp[10] = _v * _mat.elems[10];
    //    tmp[11] = _v * _mat.elems[11];
    //
    //    tmp[12] = _v * _mat.elems[12];
    //    tmp[13] = _v * _mat.elems[13];
    //    tmp[14] = _v * _mat.elems[14];
    //    tmp[15] = _v * _mat.elems[15];
    //
    //    return matrix4_t<T_t>(tmp);

    return matrix4_t<T_t>(_v * _mat.eigen_mat);
  }

  /**
   * @brief 行向量乘矩阵
   * @tparam U_t 向量类型
   * @param  _v               向量
   * @param  _mat             矩阵
   * @return vector4_t<U_t>   结果
   */
  template <class U_t>
  friend vector4_t<U_t> operator*(const vector4_t<U_t> &_v,
                                  const matrix4_t<T_t> &_mat) {
    auto new_x = _v.x * _mat.elems[0] + _v.y * _mat.elems[4] +
                 _v.z * _mat.elems[8] + _v.w * _mat.elems[12];
    auto new_y = _v.x * _mat.elems[1] + _v.y * _mat.elems[5] +
                 _v.z * _mat.elems[9] + _v.w * _mat.elems[13];
    auto new_z = _v.x * _mat.elems[2] + _v.y * _mat.elems[6] +
                 _v.z * _mat.elems[10] + _v.w * _mat.elems[14];
    auto new_w = _v.x * _mat.elems[3] + _v.y * _mat.elems[7] +
                 _v.z * _mat.elems[11] + _v.w * _mat.elems[15];
    return vector4_t<U_t>(new_x, new_y, new_z, new_w);
  }

  /**
   * @brief 矩阵乘列向量
   * @tparam U_t 向量类型
   * @param  _mat             矩阵
   * @param  _v               向量
   * @return vector4_t<U_t>   结果
   */
  template <class U_t>
  friend vector4_t<U_t> operator*(const matrix4_t<T_t> &_mat,
                                  const vector4_t<U_t> &_v) {
    auto new_x = _v.x * _mat.elems[0] + _v.y * _mat.elems[1] +
                 _v.z * _mat.elems[2] + _v.w * _mat.elems[3];
    auto new_y = _v.x * _mat.elems[4] + _v.y * _mat.elems[5] +
                 _v.z * _mat.elems[6] + _v.w * _mat.elems[7];
    auto new_z = _v.x * _mat.elems[8] + _v.y * _mat.elems[9] +
                 _v.z * _mat.elems[10] + _v.w * _mat.elems[11];
    auto new_w = _v.x * _mat.elems[12] + _v.y * _mat.elems[13] +
                 _v.z * _mat.elems[14] + _v.w * _mat.elems[15];
    return vector4_t<U_t>(new_x, new_y, new_z, new_w);
  }

  /**
   * @brief 矩阵间乘法
   * @param  _mat             另一个 matrix4_t
   * @return matrix4_t<T_t>   结果
   */
  matrix4_t<T_t> operator*(const matrix4_t<T_t> &_mat) const;

  /**
   * @brief 矩阵数乘的自乘
   * @param  _v               数
   * @return matrix4_t<T_t>&  结果
   */
  matrix4_t<T_t> &operator*=(T_t _v);

  /**
   * @brief 行向量乘矩阵
   * @tparam U_t 向量类型
   * @param  _v               向量
   * @param  _mat             矩阵
   * @return vector4_t<U_t>&  结果
   */
  template <class U_t>
  friend vector4_t<U_t> &operator*=(vector4_t<U_t> &_v,
                                    const matrix4_t<T_t> &_mat) {
    auto new_x = _v.x * _mat.elems[0] + _v.y * _mat.elems[4] +
                 _v.z * _mat.elems[8] + _v.w * _mat.elems[12];
    auto new_y = _v.x * _mat.elems[1] + _v.y * _mat.elems[5] +
                 _v.z * _mat.elems[9] + _v.w * _mat.elems[13];
    auto new_z = _v.x * _mat.elems[2] + _v.y * _mat.elems[6] +
                 _v.z * _mat.elems[10] + _v.w * _mat.elems[14];
    auto new_w = _v.x * _mat.elems[3] + _v.y * _mat.elems[7] +
                 _v.z * _mat.elems[11] + _v.w * _mat.elems[15];

    _v.x = new_x;
    _v.y = new_y;
    _v.z = new_z;
    _v.w = new_w;
    return _v;
  }

  /**
   * @brief 矩阵乘列向量
   * @tparam U_t 向量类型
   * @param  _mat             矩阵
   * @param  _v               向量
   * @return vector4_t<U_t>&  结果
   */
  template <class U_t>
  friend vector4_t<U_t> &operator*=(const matrix4_t<T_t> &_mat,
                                    vector4_t<U_t> &_v) {
    auto new_x = _v.x * _mat.elems[0] + _v.y * _mat.elems[1] +
                 _v.z * _mat.elems[2] + _v.w * _mat.elems[3];
    auto new_y = _v.x * _mat.elems[4] + _v.y * _mat.elems[5] +
                 _v.z * _mat.elems[6] + _v.w * _mat.elems[7];
    auto new_z = _v.x * _mat.elems[8] + _v.y * _mat.elems[9] +
                 _v.z * _mat.elems[10] + _v.w * _mat.elems[11];
    auto new_w = _v.x * _mat.elems[12] + _v.y * _mat.elems[13] +
                 _v.z * _mat.elems[14] + _v.w * _mat.elems[15];

    _v.x = new_x;
    _v.y = new_y;
    _v.z = new_z;
    _v.w = new_w;
    return _v;
  }

  /**
   * @brief 矩阵间自乘
   * @param  _mat             另一个 matrix4_t
   * @return matrix4_t<T_t>&  结果
   */
  matrix4_t<T_t> &operator*=(const matrix4_t<T_t> &_mat);

  /**
   * @brief 下标重载
   * @param  _idx             元素下标
   * @return T_t&             元素
   * @note    注意不要越界访问
   */
  T_t &operator[](uint8_t _idx);

  /**
   * @brief 下标重载
   * @param  _idx             元素下标
   * @return const T_t&       元素
   */
  const T_t &operator[](uint8_t _idx) const;

  /**
   * @brief 矩阵转置
   * @return matrix4_t<T_t>   转置矩阵
   */
  matrix4_t<T_t> transpose(void) const;

  /**
   * @brief 逆矩阵
   * @return matrix4_t<T_t>   逆矩阵
   * @see https://www.geeksforgeeks.org/adjoint-inverse-matrix/
   */
  matrix4_t<T_t> inverse(void) const;

  /**
   * @brief 缩放矩阵
   * @param  _scale           缩放倍数
   * @return  matrix4_t<T_t>  构造好的旋转矩阵
   * @note 缩放的是顶点
   */
  matrix4_t<T_t> scale(T_t _scale) const;

  /**
   * @brief 缩放矩阵
   * @param  _x               x 方向缩放倍数
   * @param  _y               y 方向缩放倍数
   * @param  _z               z 方向缩放倍数
   * @return  matrix4_t<T_t>  构造好的缩放矩阵
   * @note 缩放的是顶点
   */
  matrix4_t<T_t> scale(T_t _x, T_t _y, T_t _z) const;

  //  /**
  //   * @brief 绕 x 轴旋转，返回当前矩阵与旋转矩阵相乘的结果 rotate_x_mat*
  //   *this
  //   * @param  _angle           要旋转的角度
  //   * @return matrix4_t<T_t>   构造好的旋转矩阵
  //   * @note 左手系，x 向右，y 向下，z 向屏幕外
  //   * @see http://www.songho.ca/opengl/gl_anglestoaxes.html
  //   *  {
  //   *      {1, 0, 0, 0},
  //   *      {0, c, -s, 0},
  //   *      {0, s, c, 0},
  //   *      {0, 0, 0, 1}
  //   *  }
  //   */
  //  matrix4_t<T_t> rotate_x(float _angle);
  //
  //  /**
  //   * @brief 绕 y 轴旋转，返回当前矩阵与旋转矩阵相乘的结果 rotate_y_mat*
  //   *this
  //   * @param  _angle           要旋转的角度
  //   * @return matrix4_t<T_t>   构造好的旋转矩阵
  //   * @note 左手系，x 向右，y 向下，z 向屏幕外
  //   * @see http://www.songho.ca/opengl/gl_anglestoaxes.html
  //   *  {
  //   *      {c, 0, s, 0},
  //   *      {0, 1, 0, 0},
  //   *      {-s, 0, c, 0},
  //   *      {0, 0, 0, 1}
  //   *  }
  //   */
  //  matrix4_t<T_t> rotate_y(float _angle);
  //
  //  /**
  //   * @brief 绕 z 轴旋转，返回当前矩阵与旋转矩阵相乘的结果 rotate_z_mat*
  //   *this
  //   * @param  _angle           要旋转的角度
  //   * @return matrix4_t<T_t>   构造好的旋转矩阵
  //   * @note 左手系，x 向右，y 向下，z 向屏幕外
  //   * @see http://www.songho.ca/opengl/gl_anglestoaxes.html
  //   *  {
  //   *      {c, -s, 0, 0},
  //   *      {s, c, 0, 0},
  //   *      {0, 0, 1, 0},
  //   *      {0, 0, 0, 1}
  //   *  }
  //   */
  //  matrix4_t<T_t> rotate_z(float _angle);

  /**
   * @brief 旋转矩阵，Rodriguez 方法
   * @param  _axis            旋转轴，起点为原点，单位向量
   * @param  _angle           要旋转的角度
   * @return matrix4_t<T_t>   构造好的旋转矩阵
   * @note 左手系，x 向右，y 向下，z 向屏幕外
   * @note 旋转的是顶点，逆时针为正方向
   * @note 旋转轴如无特殊需要应使用单位向量
   * @see https://zhuanlan.zhihu.com/p/401806150
   * @todo 四元数实现
   * @see https://krasjet.github.io/quaternion/quaternion.pdf
   * R(_axis, _angle) = cos(_angle) * I
   *                  + (1 - cos(_angle)) * r * rt
   *                  + sin(_angle) * N
   * 其中，_axis 为旋转轴，_angle 为要旋转的弧度，I 为单位矩阵，
   * r 为 [_axis.x, _axis.y, _axis.z]，rt 为 r 的转置
   * N 为 {       0, -_axis.z,  _axis.y},
   *     { _axis.z,        0, -_axis.x},
   *     {-_axis.y,  _axis.x,        0}
   */
  matrix4_t<T_t> rotate(const vector4_t<T_t> &_axis, float _angle) const;

  /**
   * @brief 从 _from 旋转到 _to，不需要单位向量
   * @param  _from            开始位置向量
   * @param  _to              目标位置向量
   * @return const matrix4_t<T_t>  旋转矩阵
   */
  matrix4_t<T_t> rotate_from_to(const vector4f_t &_from,
                                const vector4f_t &_to) const;

  /**
   * @brief 平移矩阵，返回当前矩阵与平移矩阵相乘的结果 translate_mat* *this
   * @param  _x               x 方向变换
   * @param  _y               y 方向变换
   * @param  _z               z 方向变换
   * @return matrix4_t<T_t>   构造好的平移矩阵
   * @note 先旋转再平移
   * @note 平移的是顶点
   * @see http://www.songho.ca/opengl/gl_transform.html#modelview
   *   {
   *      {1, 0, 0, _x},
   *      {0, 1, 0, _y},
   *      {0, 0, 1, _z},
   *      {0, 0, 0, 1}
   *   }
   */
  matrix4_t<T_t> translate(T_t _x, T_t _y, T_t _z) const;
  /**
   * @brief 角度转换为弧度
   * @param  _deg             角度
   * @return float            弧度
   */
  static float RAD(float _deg);

  /**
   * @brief 弧度转换为角度
   * @param  _rad             弧度
   * @return float            角度
   */
  static float DEG(float _rad);

  friend std::ostream &operator<<(std::ostream &_os,
                                  const matrix4_t<T_t> &_mat) {
    _os.setf(std::ios::right);
    _os.precision(16);

    _os << "{\n";

    _os << std::setw(4) << "{" << std::setw(25) << _mat.elems[0] << ", "
        << _mat.elems[1] << ", " << _mat.elems[2] << ", " << _mat.elems[3]
        << "},\n";

    _os << std::setw(4) << "{" << std::setw(25) << _mat.elems[4] << ", "
        << _mat.elems[5] << ", " << _mat.elems[6] << ", " << _mat.elems[7]
        << "},\n";

    _os << std::setw(4) << "{" << std::setw(25) << _mat.elems[8] << ", "
        << _mat.elems[9] << ", " << _mat.elems[10] << ", " << _mat.elems[11]
        << "},\n";

    _os << std::setw(4) << "{" << std::setw(25) << _mat.elems[12] << ", "
        << _mat.elems[13] << ", " << _mat.elems[14] << ", " << _mat.elems[15]
        << "}\n";

    _os << "}";

    _os.unsetf(std::ios::right);
    _os.precision(6);

    return _os;
  }

  /// @todo 逗号初始化
  /// @see https://gitee.com/aczz/cv-dbg/blob/master/comma_init/v4.cpp
  /// @see https://zhuanlan.zhihu.com/p/377573738
};

template <matrix_element_concept_t T_t>
T_t matrix4_t<T_t>::determ(uint8_t _order) const {
  // 递归返回条件
  if (_order == 1) {
    return elems[0];
  }

  T_t res = 0;
  // 当前正负
  int sign = 1;
  // 计算 mat[0][i] 的代数余子式
  for (auto i = 0; i < _order; i++) {
    res += sign * elems[i] * cofactor(0, i, _order).determ(_order - 1);
    // 符号取反
    sign = -sign;
  }

  return res;
}

template <matrix_element_concept_t T_t>
matrix4_t<T_t> matrix4_t<T_t>::cofactor(uint8_t _row, uint8_t _col,
                                        uint8_t _order) const {
  T_t tmp[ELEMS] = {0};
  auto row_idx = 0;
  auto col_idx = 0;
  for (auto i = 0; i < _order; i++) {
    for (auto j = 0; j < _order; j++) {
      if (i != _row && j != _col) {
        tmp[row_idx * ORDER + col_idx++] = elems[i * ORDER + j];
        // 换行
        if (col_idx == _order - 1) {
          col_idx = 0;
          row_idx++;
        }
      }
    }
  }

  return matrix4_t<T_t>(tmp);
}

template <matrix_element_concept_t T_t>
matrix4_t<T_t> matrix4_t<T_t>::adjoint(void) const {
  T_t tmp[ELEMS] = {0};

  // 计算代数余子式矩阵并转置
  // 行列索引之和为奇数时为负
  tmp[0] = cofactor(0, 0, 4).determ(3);
  tmp[1] = -cofactor(1, 0, 4).determ(3);
  tmp[2] = cofactor(2, 0, 4).determ(3);
  tmp[3] = -cofactor(3, 0, 4).determ(3);

  tmp[4] = -cofactor(0, 1, 4).determ(3);
  tmp[5] = cofactor(1, 1, 4).determ(3);
  tmp[6] = -cofactor(2, 1, 4).determ(3);
  tmp[7] = cofactor(3, 1, 4).determ(3);

  tmp[8] = cofactor(0, 2, 4).determ(3);
  tmp[9] = -cofactor(1, 2, 4).determ(3);
  tmp[10] = cofactor(2, 2, 4).determ(3);
  tmp[11] = -cofactor(3, 2, 4).determ(3);

  tmp[12] = -cofactor(0, 3, 4).determ(3);
  tmp[13] = cofactor(1, 3, 4).determ(3);
  tmp[14] = -cofactor(2, 3, 4).determ(3);
  tmp[15] = cofactor(3, 3, 4).determ(3);

  return matrix4_t<T_t>(tmp);
}

template <matrix_element_concept_t T_t> matrix4_t<T_t>::matrix4_t(void) {
  elems.fill(0);
  elems[0] = 1;
  elems[5] = 1;
  elems[10] = 1;
  elems[15] = 1;

  eigen_mat.setIdentity();

  return;
}

template <matrix_element_concept_t T_t>
matrix4_t<T_t>::matrix4_t(const matrix4_t<T_t> &_mat) {
  elems = _mat.elems;
  eigen_mat = _mat.eigen_mat;
  return;
}

template <matrix_element_concept_t T_t>
matrix4_t<T_t>::matrix4_t(const Eigen::Matrix<T_t, ORDER, ORDER> &_mat) {
  eigen_mat = _mat;
  return;
}

template <matrix_element_concept_t T_t>
matrix4_t<T_t>::matrix4_t(const T_t *_arr) {
  if (_arr == nullptr) {
    throw std::invalid_argument(log("_arr == nullptr"));
  }

  elems[0] = _arr[0];
  elems[1] = _arr[1];
  elems[2] = _arr[2];
  elems[3] = _arr[3];

  elems[4] = _arr[4];
  elems[5] = _arr[5];
  elems[6] = _arr[6];
  elems[7] = _arr[7];

  elems[8] = _arr[8];
  elems[9] = _arr[9];
  elems[10] = _arr[10];
  elems[11] = _arr[11];

  elems[12] = _arr[12];
  elems[13] = _arr[13];
  elems[14] = _arr[14];
  elems[15] = _arr[15];

//  eigen_mat.setIdentity();
  eigen_mat(0, 0) = _arr[0];
  eigen_mat(0, 1) = _arr[1];
  eigen_mat(0, 2) = _arr[2];
  eigen_mat(0, 3) = _arr[3];

  eigen_mat(1, 0) = _arr[4];
  eigen_mat(1, 1) = _arr[5];
  eigen_mat(1, 2) = _arr[6];
  eigen_mat(1, 3) = _arr[7];

  eigen_mat(2, 0) = _arr[8];
  eigen_mat(2, 1) = _arr[9];
  eigen_mat(2, 2) = _arr[10];
  eigen_mat(2, 3) = _arr[11];

  eigen_mat(3, 0) = _arr[12];
  eigen_mat(3, 1) = _arr[13];
  eigen_mat(3, 2) = _arr[14];
  eigen_mat(3, 3) = _arr[15];

  return;
}

template <matrix_element_concept_t T_t>
matrix4_t<T_t>::matrix4_t(const T_t _arr[ORDER][ORDER]) {
  if (_arr == nullptr) {
    throw std::invalid_argument(log("_arr == nullptr"));
  }

  elems[0] = _arr[0][0];
  elems[1] = _arr[0][1];
  elems[2] = _arr[0][2];
  elems[3] = _arr[0][3];

  elems[4] = _arr[1][0];
  elems[5] = _arr[1][1];
  elems[6] = _arr[1][2];
  elems[7] = _arr[1][3];

  elems[8] = _arr[2][0];
  elems[9] = _arr[2][1];
  elems[10] = _arr[2][2];
  elems[11] = _arr[2][3];

  elems[12] = _arr[3][0];
  elems[13] = _arr[3][1];
  elems[14] = _arr[3][2];
  elems[15] = _arr[3][3];

  eigen_mat(0, 0) = _arr[0][0];
  eigen_mat(0, 1) = _arr[0][1];
  eigen_mat(0, 2) = _arr[0][2];
  eigen_mat(0, 3) = _arr[0][3];

  eigen_mat(1, 0) = _arr[1][0];
  eigen_mat(1, 1) = _arr[1][1];
  eigen_mat(1, 2) = _arr[1][2];
  eigen_mat(1, 3) = _arr[1][3];

  eigen_mat(2, 0) = _arr[2][0];
  eigen_mat(2, 1) = _arr[2][1];
  eigen_mat(2, 2) = _arr[2][2];
  eigen_mat(2, 3) = _arr[2][3];

  eigen_mat(3, 0) = _arr[3][0];
  eigen_mat(3, 1) = _arr[3][1];
  eigen_mat(3, 2) = _arr[3][2];
  eigen_mat(3, 3) = _arr[3][3];

  return;
}

template <matrix_element_concept_t T_t>
matrix4_t<T_t>::matrix4_t(const vector4_t<T_t> &_v) {
  elems.fill(0);
  elems[0] = _v.x;
  elems[5] = _v.y;
  elems[10] = _v.z;
  elems[15] = _v.w;

  eigen_mat.setIdentity();
  //  eigen_mat[0] = _v.x;
  //  eigen_mat[5] = _v.y;
  //  eigen_mat[10] = _v.z;
  //  eigen_mat[15] = _v.w;

  return;
}

template <matrix_element_concept_t T_t>
matrix4_t<T_t> &matrix4_t<T_t>::operator=(const matrix4_t<T_t> &_mat) {
  if (this == &_mat) {
    throw std::runtime_error(log("this == &_mat"));
  }
  elems = _mat.elems;
  eigen_mat = _mat.eigen_mat;
  return *this;
}

template <matrix_element_concept_t T_t>
bool matrix4_t<T_t>::operator==(const matrix4_t<T_t> &_mat) const {
  return eigen_mat == _mat.eigen_mat;
}

template <matrix_element_concept_t T_t>
bool matrix4_t<T_t>::operator!=(const matrix4_t<T_t> &_mat) const {
  return eigen_mat != _mat.eigen_mat;
}

template <matrix_element_concept_t T_t>
matrix4_t<T_t> matrix4_t<T_t>::operator+(const matrix4_t<T_t> &_mat) const {
  //  T_t tmp[ELEMS] = {0};
  //
  //  tmp[0] = elems[0] + _mat.elems[0];
  //  tmp[1] = elems[1] + _mat.elems[1];
  //  tmp[2] = elems[2] + _mat.elems[2];
  //  tmp[3] = elems[3] + _mat.elems[3];
  //
  //  tmp[4] = elems[4] + _mat.elems[4];
  //  tmp[5] = elems[5] + _mat.elems[5];
  //  tmp[6] = elems[6] + _mat.elems[6];
  //  tmp[7] = elems[7] + _mat.elems[7];
  //
  //  tmp[8] = elems[8] + _mat.elems[8];
  //  tmp[9] = elems[9] + _mat.elems[9];
  //  tmp[10] = elems[10] + _mat.elems[10];
  //  tmp[11] = elems[11] + _mat.elems[11];
  //
  //  tmp[12] = elems[12] + _mat.elems[12];
  //  tmp[13] = elems[13] + _mat.elems[13];
  //  tmp[14] = elems[14] + _mat.elems[14];
  //  tmp[15] = elems[15] + _mat.elems[15];
  //
  //  return matrix4_t<T_t>(tmp);

  return matrix4_t<T_t>(eigen_mat + _mat.eigen_mat);
}

template <matrix_element_concept_t T_t>
matrix4_t<T_t> &matrix4_t<T_t>::operator+=(const matrix4_t<T_t> &_mat) {
  elems[0] += _mat.elems[0];
  elems[1] += _mat.elems[1];
  elems[2] += _mat.elems[2];
  elems[3] += _mat.elems[3];

  elems[4] += _mat.elems[4];
  elems[5] += _mat.elems[5];
  elems[6] += _mat.elems[6];
  elems[7] += _mat.elems[7];

  elems[8] += _mat.elems[8];
  elems[9] += _mat.elems[9];
  elems[10] += _mat.elems[10];
  elems[11] += _mat.elems[11];

  elems[12] += _mat.elems[12];
  elems[13] += _mat.elems[13];
  elems[14] += _mat.elems[14];
  elems[15] += _mat.elems[15];

  eigen_mat += _mat.eigen_mat;

  return *this;
}

template <matrix_element_concept_t T_t>
matrix4_t<T_t> matrix4_t<T_t>::operator-(const matrix4_t<T_t> &_mat) const {
  //  T_t tmp[ELEMS] = {0};
  //
  //  tmp[0] = elems[0] - _mat.elems[0];
  //  tmp[1] = elems[1] - _mat.elems[1];
  //  tmp[2] = elems[2] - _mat.elems[2];
  //  tmp[3] = elems[3] - _mat.elems[3];
  //
  //  tmp[4] = elems[4] - _mat.elems[4];
  //  tmp[5] = elems[5] - _mat.elems[5];
  //  tmp[6] = elems[6] - _mat.elems[6];
  //  tmp[7] = elems[7] - _mat.elems[7];
  //
  //  tmp[8] = elems[8] - _mat.elems[8];
  //  tmp[9] = elems[9] - _mat.elems[9];
  //  tmp[10] = elems[10] - _mat.elems[10];
  //  tmp[11] = elems[11] - _mat.elems[11];
  //
  //  tmp[12] = elems[12] - _mat.elems[12];
  //  tmp[13] = elems[13] - _mat.elems[13];
  //  tmp[14] = elems[14] - _mat.elems[14];
  //  tmp[15] = elems[15] - _mat.elems[15];
  //
  //  return matrix4_t<T_t>(tmp);

  return matrix4_t<T_t>(eigen_mat - _mat.eigen_mat);
}

template <matrix_element_concept_t T_t>
matrix4_t<T_t> &matrix4_t<T_t>::operator-=(const matrix4_t<T_t> &_mat) {
  elems[0] -= _mat.elems[0];
  elems[1] -= _mat.elems[1];
  elems[2] -= _mat.elems[2];
  elems[3] -= _mat.elems[3];

  elems[4] -= _mat.elems[4];
  elems[5] -= _mat.elems[5];
  elems[6] -= _mat.elems[6];
  elems[7] -= _mat.elems[7];

  elems[8] -= _mat.elems[8];
  elems[9] -= _mat.elems[9];
  elems[10] -= _mat.elems[10];
  elems[11] -= _mat.elems[11];

  elems[12] -= _mat.elems[12];
  elems[13] -= _mat.elems[13];
  elems[14] -= _mat.elems[14];
  elems[15] -= _mat.elems[15];

  eigen_mat -= _mat.eigen_mat;

  return *this;
}

template <matrix_element_concept_t T_t>
matrix4_t<T_t> matrix4_t<T_t>::operator*(const matrix4_t<T_t> &_mat) const {
  //  T_t tmp[ELEMS] = {0};
  //
  //  tmp[0] = elems[0] * _mat.elems[0] + elems[1] * _mat.elems[4] +
  //           elems[2] * _mat.elems[8] + elems[3] * _mat.elems[12];
  //  tmp[1] = elems[0] * _mat.elems[1] + elems[1] * _mat.elems[5] +
  //           elems[2] * _mat.elems[9] + elems[3] * _mat.elems[13];
  //  tmp[2] = elems[0] * _mat.elems[2] + elems[1] * _mat.elems[6] +
  //           elems[2] * _mat.elems[10] + elems[3] * _mat.elems[14];
  //  tmp[3] = elems[0] * _mat.elems[3] + elems[1] * _mat.elems[7] +
  //           elems[2] * _mat.elems[11] + elems[3] * _mat.elems[15];
  //
  //  tmp[4] = elems[4] * _mat.elems[0] + elems[5] * _mat.elems[4] +
  //           elems[6] * _mat.elems[8] + elems[7] * _mat.elems[12];
  //  tmp[5] = elems[4] * _mat.elems[1] + elems[5] * _mat.elems[5] +
  //           elems[6] * _mat.elems[9] + elems[7] * _mat.elems[13];
  //  tmp[6] = elems[4] * _mat.elems[2] + elems[5] * _mat.elems[6] +
  //           elems[6] * _mat.elems[10] + elems[7] * _mat.elems[14];
  //  tmp[7] = elems[4] * _mat.elems[3] + elems[5] * _mat.elems[7] +
  //           elems[6] * _mat.elems[11] + elems[7] * _mat.elems[15];
  //
  //  tmp[8] = elems[8] * _mat.elems[0] + elems[9] * _mat.elems[4] +
  //           elems[10] * _mat.elems[8] + elems[11] * _mat.elems[12];
  //  tmp[9] = elems[8] * _mat.elems[1] + elems[9] * _mat.elems[5] +
  //           elems[10] * _mat.elems[9] + elems[11] * _mat.elems[13];
  //  tmp[10] = elems[8] * _mat.elems[2] + elems[9] * _mat.elems[6] +
  //            elems[10] * _mat.elems[10] + elems[11] * _mat.elems[14];
  //  tmp[11] = elems[8] * _mat.elems[3] + elems[9] * _mat.elems[7] +
  //            elems[10] * _mat.elems[11] + elems[11] * _mat.elems[15];
  //
  //  tmp[12] = elems[12] * _mat.elems[0] + elems[13] * _mat.elems[4] +
  //            elems[14] * _mat.elems[8] + elems[15] * _mat.elems[12];
  //  tmp[13] = elems[12] * _mat.elems[1] + elems[13] * _mat.elems[5] +
  //            elems[14] * _mat.elems[9] + elems[15] * _mat.elems[13];
  //  tmp[14] = elems[12] * _mat.elems[2] + elems[13] * _mat.elems[6] +
  //            elems[14] * _mat.elems[10] + elems[15] * _mat.elems[14];
  //  tmp[15] = elems[12] * _mat.elems[3] + elems[13] * _mat.elems[7] +
  //            elems[14] * _mat.elems[11] + elems[15] * _mat.elems[15];
  //
  //  return matrix4_t<T_t>(tmp);

  return matrix4_t<T_t>(eigen_mat * _mat.eigen_mat);
}

template <matrix_element_concept_t T_t>
matrix4_t<T_t> &matrix4_t<T_t>::operator*=(T_t _v) {
  for (auto &i : elems) {
    i *= _v;
  }

  return *this;
}

template <matrix_element_concept_t T_t>
matrix4_t<T_t> &matrix4_t<T_t>::operator*=(const matrix4_t<T_t> &_mat) {
  T_t tmp[ELEMS] = {0};
  tmp[0] = elems[0] * _mat.elems[0] + elems[1] * _mat.elems[4] +
           elems[2] * _mat.elems[8] + elems[3] * _mat.elems[12];
  tmp[1] = elems[0] * _mat.elems[1] + elems[1] * _mat.elems[5] +
           elems[2] * _mat.elems[9] + elems[3] * _mat.elems[13];
  tmp[2] = elems[0] * _mat.elems[2] + elems[1] * _mat.elems[6] +
           elems[2] * _mat.elems[10] + elems[3] * _mat.elems[14];
  tmp[3] = elems[0] * _mat.elems[3] + elems[1] * _mat.elems[7] +
           elems[2] * _mat.elems[11] + elems[3] * _mat.elems[15];

  tmp[4] = elems[4] * _mat.elems[0] + elems[5] * _mat.elems[4] +
           elems[6] * _mat.elems[8] + elems[7] * _mat.elems[12];
  tmp[5] = elems[4] * _mat.elems[1] + elems[5] * _mat.elems[5] +
           elems[6] * _mat.elems[9] + elems[7] * _mat.elems[13];
  tmp[6] = elems[4] * _mat.elems[2] + elems[5] * _mat.elems[6] +
           elems[6] * _mat.elems[10] + elems[7] * _mat.elems[14];
  tmp[7] = elems[4] * _mat.elems[3] + elems[5] * _mat.elems[7] +
           elems[6] * _mat.elems[11] + elems[7] * _mat.elems[15];

  tmp[8] = elems[8] * _mat.elems[0] + elems[9] * _mat.elems[4] +
           elems[10] * _mat.elems[8] + elems[11] * _mat.elems[12];
  tmp[9] = elems[8] * _mat.elems[1] + elems[9] * _mat.elems[5] +
           elems[10] * _mat.elems[9] + elems[11] * _mat.elems[13];
  tmp[10] = elems[8] * _mat.elems[2] + elems[9] * _mat.elems[6] +
            elems[10] * _mat.elems[10] + elems[11] * _mat.elems[14];
  tmp[11] = elems[8] * _mat.elems[3] + elems[9] * _mat.elems[7] +
            elems[10] * _mat.elems[11] + elems[11] * _mat.elems[15];

  tmp[12] = elems[12] * _mat.elems[0] + elems[13] * _mat.elems[4] +
            elems[14] * _mat.elems[8] + elems[15] * _mat.elems[12];
  tmp[13] = elems[12] * _mat.elems[1] + elems[13] * _mat.elems[5] +
            elems[14] * _mat.elems[9] + elems[15] * _mat.elems[13];
  tmp[14] = elems[12] * _mat.elems[2] + elems[13] * _mat.elems[6] +
            elems[14] * _mat.elems[10] + elems[15] * _mat.elems[14];
  tmp[15] = elems[12] * _mat.elems[3] + elems[13] * _mat.elems[7] +
            elems[14] * _mat.elems[11] + elems[15] * _mat.elems[15];

  elems[0] = tmp[0];
  elems[1] = tmp[1];
  elems[2] = tmp[2];
  elems[3] = tmp[3];

  elems[4] = tmp[4];
  elems[5] = tmp[5];
  elems[6] = tmp[6];
  elems[7] = tmp[7];

  elems[8] = tmp[8];
  elems[9] = tmp[9];
  elems[10] = tmp[10];
  elems[11] = tmp[11];

  elems[12] = tmp[12];
  elems[13] = tmp[13];
  elems[14] = tmp[14];
  elems[15] = tmp[15];

  eigen_mat *= _mat.eigen_mat;

  return *this;
}

template <matrix_element_concept_t T_t>
T_t &matrix4_t<T_t>::operator[](uint8_t _idx) {
  if (_idx > ORDER) {
    throw std::invalid_argument(log("_idx > ORDER"));
  }
  return eigen_mat[_idx];
}

template <matrix_element_concept_t T_t>
const T_t &matrix4_t<T_t>::operator[](uint8_t _idx) const {
  if (_idx > ORDER) {
    throw std::invalid_argument(log("_idx > ORDER"));
  }
  return eigen_mat[_idx];
}

template <matrix_element_concept_t T_t>
matrix4_t<T_t> matrix4_t<T_t>::transpose(void) const {
  //  T_t tmp[ELEMS] = {0};
  //  tmp[0] = elems[0];
  //  tmp[1] = elems[4];
  //  tmp[2] = elems[8];
  //  tmp[3] = elems[12];
  //
  //  tmp[4] = elems[1];
  //  tmp[5] = elems[5];
  //  tmp[6] = elems[9];
  //  tmp[7] = elems[13];
  //
  //  tmp[8] = elems[2];
  //  tmp[9] = elems[6];
  //  tmp[10] = elems[10];
  //  tmp[11] = elems[14];
  //
  //  tmp[12] = elems[3];
  //  tmp[13] = elems[7];
  //  tmp[14] = elems[11];
  //  tmp[15] = elems[15];
  //
  //  return matrix4_t<T_t>(tmp);

  return matrix4_t<T_t>(eigen_mat.transpose());
}

template <matrix_element_concept_t T_t>
matrix4_t<T_t> matrix4_t<T_t>::inverse(void) const {
  //  // 计算行列式
  //  T_t det = determ(ORDER);
  //  if (std::abs(det) <= std::numeric_limits<T_t>::epsilon()) {
  //    throw std::runtime_error(
  //        log("std::abs(det) <= std::numeric_limits<T_t>::epsilon(), Singular
  //        "
  //            "matrix, can't find its inverse"));
  //  }
  //
  //  // 逆矩阵 = 伴随矩阵 / 行列式
  //  auto tmp = adjoint() * (1 / det);
  //
  //  return matrix4_t<T_t>(tmp);

  return matrix4_t<T_t>(eigen_mat.inverse());
}

template <matrix_element_concept_t T_t>
matrix4_t<T_t> matrix4_t<T_t>::scale(T_t _scale) const {
  T_t tmp[ELEMS] = {0};

  tmp[0] = _scale * elems[0];
  tmp[1] = _scale * elems[1];
  tmp[2] = _scale * elems[2];
  tmp[3] = _scale * elems[3];

  tmp[4] = _scale * elems[4];
  tmp[5] = _scale * elems[5];
  tmp[6] = _scale * elems[6];
  tmp[7] = _scale * elems[7];

  tmp[8] = _scale * elems[8];
  tmp[9] = _scale * elems[9];
  tmp[10] = _scale * elems[10];
  tmp[11] = _scale * elems[11];

  tmp[12] = elems[12];
  tmp[13] = elems[13];
  tmp[14] = elems[14];
  tmp[15] = elems[15];

  return matrix4_t<T_t>(tmp);
}

template <matrix_element_concept_t T_t>
matrix4_t<T_t> matrix4_t<T_t>::scale(T_t _x, T_t _y, T_t _z) const {
  T_t tmp[ELEMS] = {0};

  tmp[0] = _x * elems[0];
  tmp[1] = _x * elems[1];
  tmp[2] = _x * elems[2];
  tmp[3] = _x * elems[3];

  tmp[4] = _y * elems[4];
  tmp[5] = _y * elems[5];
  tmp[6] = _y * elems[6];
  tmp[7] = _y * elems[7];

  tmp[8] = _z * elems[8];
  tmp[9] = _z * elems[9];
  tmp[10] = _z * elems[10];
  tmp[11] = _z * elems[11];

  tmp[12] = elems[12];
  tmp[13] = elems[13];
  tmp[14] = elems[14];
  tmp[15] = elems[15];

  return matrix4_t<T_t>(tmp);
}

///// @todo 修改为直接返回
// template <matrix_element_concept_t T_t>
// matrix4_t<T_t> matrix4_t<T_t>::rotate_x(float _angle) {
//   // 角度转弧度
//   auto rad = RAD(_angle);
//
//   auto c = cos(rad);
//   auto s = sin(rad);
//   auto m = matrix4_t<T_t>();
//   m.elems[5] = c;
//   m.elems[6] = -s;
//   m.elems[9] = s;
//   m.elems[10] = c;
//
//   return m * *this;
// }
//
///// @todo 修改为直接返回
// template <matrix_element_concept_t T_t>
// matrix4_t<T_t> matrix4_t<T_t>::rotate_y(float _angle) {
//   // 角度转弧度
//   auto rad = RAD(_angle);
//
//   auto c = cos(rad);
//   auto s = sin(rad);
//   auto m = matrix4_t<T_t>();
//   m.elems[0] = c;
//   m.elems[2] = s;
//   m.elems[8] = -s;
//   m.elems[10] = c;
//
//   return m * *this;
// }
//
///// @todo 修改为直接返回
// template <matrix_element_concept_t T_t>
// matrix4_t<T_t> matrix4_t<T_t>::rotate_z(float _angle) {
//   // 角度转弧度
//   auto rad = RAD(_angle);
//
//   auto c = cos(rad);
//   auto s = sin(rad);
//   auto m = matrix4_t<T_t>();
//   m.elems[0] = c;
//   m.elems[1] = -s;
//   m.elems[4] = s;
//   m.elems[5] = c;
//
//   return m * *this;
// }

/// @todo 精度问题
template <matrix_element_concept_t T_t>
matrix4_t<T_t> matrix4_t<T_t>::rotate(const vector4_t<T_t> &_axis,
                                      float _angle) const {
  // 角度转弧度
  auto rad = RAD(_angle);

  // 计算公式中的参数
  auto c = std::cos(rad);
  auto s = std::sin(rad);
  auto t = 1 - c;

  auto tx = t * _axis.x;
  auto ty = t * _axis.y;
  auto tz = t * _axis.z;

  auto sx = s * _axis.x;
  auto sy = s * _axis.y;
  auto sz = s * _axis.z;

  // 计算结果
  // cos(_angle) * I
  T_t cI_arr[4][4] = {{c, 0, 0, 0}, {0, c, 0, 0}, {0, 0, c, 0}, {0, 0, 0, 0}};
  auto cI = matrix4_t<T_t>(cI_arr);

  // (1 - cos(_angle)) * r * rt
  T_t rrt_arr[4][4] = {{tx * _axis.x, tx * _axis.y, tx * _axis.z, 0},
                       {tx * _axis.y, ty * _axis.y, ty * _axis.z, 0},
                       {tx * _axis.z, ty * _axis.z, tz * _axis.z, 0},
                       {0, 0, 0, 0}};
  auto rrt = matrix4_t<T_t>(rrt_arr);

  // sin(_angle) * N
  float sN_arr[4][4] = {
      {0, -sz, sy, 0}, {sz, 0, -sx, 0}, {-sy, sx, 0, 0}, {0, 0, 0, 0}};
  auto sN = matrix4_t<T_t>(sN_arr);

  matrix4_t<T_t> res = cI + rrt + sN;
  res.elems[15] = 1;

  return res * *this;
}

/// @todo 精度问题
template <matrix_element_concept_t T_t>
matrix4_t<T_t> matrix4_t<T_t>::rotate_from_to(const vector4f_t &_from,
                                              const vector4f_t &_to) const {
  auto f = _from.normalize();
  auto t = _to.normalize();

  // axis multiplication by sin
  auto vs(t ^ f);

  // axis of rotation
  auto v(vs);
  v = v.normalize();

  // cosinus angle
  auto ca = f * t;

  auto vt(v * (1 - ca));

  T_t tmp[ELEMS] = {0};

  tmp[0] = vt.x * v.x + ca;
  tmp[5] = vt.y * v.y + ca;
  tmp[10] = vt.z * v.z + ca;

  vt.x *= v.y;
  vt.z *= v.x;
  vt.y *= v.z;

  tmp[1] = vt.x + vs.z;
  tmp[2] = vt.z - vs.y;
  tmp[3] = 0;

  tmp[4] = vt.x - vs.z;
  tmp[6] = vt.y + vs.x;
  tmp[7] = 0;

  tmp[8] = vt.z + vs.y;
  tmp[9] = vt.y - vs.x;
  tmp[11] = 0;

  tmp[12] = 0;
  tmp[13] = 0;
  tmp[14] = 0;
  tmp[15] = 1;

  return matrix4_t<T_t>(tmp);
}

template <matrix_element_concept_t T_t>
matrix4_t<T_t> matrix4_t<T_t>::translate(T_t _x, T_t _y, T_t _z) const {
  T_t tmp[ELEMS] = {0};

  tmp[0] = elems[0] + _x * elems[12];
  tmp[1] = elems[1] + _x * elems[13];
  tmp[2] = elems[2] + _x * elems[14];
  tmp[3] = elems[3] + _x * elems[15];

  tmp[4] = elems[4] + _y * elems[12];
  tmp[5] = elems[5] + _y * elems[13];
  tmp[6] = elems[6] + _y * elems[14];
  tmp[7] = elems[7] + _y * elems[15];

  tmp[8] = elems[8] + _z * elems[12];
  tmp[9] = elems[9] + _z * elems[13];
  tmp[10] = elems[10] + _z * elems[14];
  tmp[11] = elems[11] + _z * elems[15];

  tmp[12] = elems[12];
  tmp[13] = elems[13];
  tmp[14] = elems[14];
  tmp[15] = elems[15];

  return matrix4_t<T_t>(tmp);
}

template <matrix_element_concept_t T_t> float matrix4_t<T_t>::RAD(float _deg) {
  return static_cast<float>((M_PI / 180) * (_deg));
}

template <matrix_element_concept_t T_t> float matrix4_t<T_t>::DEG(float _rad) {
  return static_cast<float>((180 / M_PI) * (_rad));
}

typedef matrix4_t<float> matrix4f_t;

#endif /* SIMPLERENDER_MATRIX4_HPP */
