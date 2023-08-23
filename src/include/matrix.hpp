
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
  Eigen::Matrix<T_t, ORDER, ORDER> eigen_mat;

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
    return vector4_t<U_t>(_v.vector.transpose() * _mat.eigen_mat);
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
    return vector4_t<U_t>(_mat.eigen_mat * _v.vector);
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
    _os << _mat.eigen_mat;

    return _os;
  }
};

template <matrix_element_concept_t T_t> matrix4_t<T_t>::matrix4_t(void) {
  eigen_mat.setIdentity();
  return;
}

template <matrix_element_concept_t T_t>
matrix4_t<T_t>::matrix4_t(const matrix4_t<T_t> &_mat) {
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

  eigen_mat.setIdentity();
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
}

template <matrix_element_concept_t T_t>
matrix4_t<T_t>::matrix4_t(const vector4_t<T_t> &_v) {
  eigen_mat.setIdentity();
  eigen_mat(0, 0) = _v.x;
  eigen_mat(1, 1) = _v.y;
  eigen_mat(2, 2) = _v.z;
  eigen_mat(3, 3) = _v.w;
}

template <matrix_element_concept_t T_t>
matrix4_t<T_t> &matrix4_t<T_t>::operator=(const matrix4_t<T_t> &_mat) {
  if (this == &_mat) {
    throw std::runtime_error(log("this == &_mat"));
  }
  eigen_mat = _mat.eigen_mat;
  return *this;
}

template <matrix_element_concept_t T_t>
bool matrix4_t<T_t>::operator==(const matrix4_t<T_t> &_mat) const {
  return eigen_mat.isApprox(_mat.eigen_mat, 1e-5);
}

template <matrix_element_concept_t T_t>
bool matrix4_t<T_t>::operator!=(const matrix4_t<T_t> &_mat) const {
  return !eigen_mat.isApprox(_mat.eigen_mat, 1e-5);
}

template <matrix_element_concept_t T_t>
matrix4_t<T_t> matrix4_t<T_t>::operator+(const matrix4_t<T_t> &_mat) const {
  return matrix4_t<T_t>(eigen_mat + _mat.eigen_mat);
}

template <matrix_element_concept_t T_t>
matrix4_t<T_t> &matrix4_t<T_t>::operator+=(const matrix4_t<T_t> &_mat) {
  eigen_mat += _mat.eigen_mat;

  return *this;
}

template <matrix_element_concept_t T_t>
matrix4_t<T_t> matrix4_t<T_t>::operator-(const matrix4_t<T_t> &_mat) const {
  return matrix4_t<T_t>(eigen_mat - _mat.eigen_mat);
}

template <matrix_element_concept_t T_t>
matrix4_t<T_t> &matrix4_t<T_t>::operator-=(const matrix4_t<T_t> &_mat) {
  eigen_mat -= _mat.eigen_mat;

  return *this;
}

template <matrix_element_concept_t T_t>
matrix4_t<T_t> matrix4_t<T_t>::operator*(const matrix4_t<T_t> &_mat) const {
  return matrix4_t<T_t>(eigen_mat * _mat.eigen_mat);
}

template <matrix_element_concept_t T_t>
matrix4_t<T_t> &matrix4_t<T_t>::operator*=(T_t _v) {
  eigen_mat *= _v;
  return *this;
}

template <matrix_element_concept_t T_t>
matrix4_t<T_t> &matrix4_t<T_t>::operator*=(const matrix4_t<T_t> &_mat) {
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
  return matrix4_t<T_t>(eigen_mat.transpose());
}

template <matrix_element_concept_t T_t>
matrix4_t<T_t> matrix4_t<T_t>::inverse(void) const {
  return matrix4_t<T_t>(eigen_mat.inverse());
}

template <matrix_element_concept_t T_t>
matrix4_t<T_t> matrix4_t<T_t>::scale(T_t _scale) const {
  Eigen::Matrix<T_t, ORDER, ORDER> res;
  res.setIdentity();
  res(0, 0) = eigen_mat(0, 0) * _scale;
  res(0, 1) = eigen_mat(0, 1) * _scale;
  res(0, 2) = eigen_mat(0, 2) * _scale;
  res(0, 3) = eigen_mat(0, 3) * _scale;

  res(1, 0) = eigen_mat(1, 0) * _scale;
  res(1, 1) = eigen_mat(1, 1) * _scale;
  res(1, 2) = eigen_mat(1, 2) * _scale;
  res(1, 3) = eigen_mat(1, 3) * _scale;

  res(2, 0) = eigen_mat(2, 0) * _scale;
  res(2, 1) = eigen_mat(2, 1) * _scale;
  res(2, 2) = eigen_mat(2, 2) * _scale;
  res(2, 3) = eigen_mat(2, 3) * _scale;

  res(3, 0) = eigen_mat(3, 0);
  res(3, 1) = eigen_mat(3, 1);
  res(3, 2) = eigen_mat(3, 2);
  res(3, 3) = eigen_mat(3, 3);

  return matrix4_t<T_t>(res);
}

template <matrix_element_concept_t T_t>
matrix4_t<T_t> matrix4_t<T_t>::scale(T_t _x, T_t _y, T_t _z) const {
  Eigen::Matrix<T_t, ORDER, ORDER> res;
  res.setIdentity();
  res(0, 0) = eigen_mat(0, 0) * _x;
  res(0, 1) = eigen_mat(0, 1) * _x;
  res(0, 2) = eigen_mat(0, 2) * _x;
  res(0, 3) = eigen_mat(0, 3) * _x;

  res(1, 0) = eigen_mat(1, 0) * _y;
  res(1, 1) = eigen_mat(1, 1) * _y;
  res(1, 2) = eigen_mat(1, 2) * _y;
  res(1, 3) = eigen_mat(1, 3) * _y;

  res(2, 0) = eigen_mat(2, 0) * _z;
  res(2, 1) = eigen_mat(2, 1) * _z;
  res(2, 2) = eigen_mat(2, 2) * _z;
  res(2, 3) = eigen_mat(2, 3) * _z;

  res(3, 0) = eigen_mat(3, 0);
  res(3, 1) = eigen_mat(3, 1);
  res(3, 2) = eigen_mat(3, 2);
  res(3, 3) = eigen_mat(3, 3);

  return matrix4_t<T_t>(res);
}

template <matrix_element_concept_t T_t>
matrix4_t<T_t> matrix4_t<T_t>::rotate(const vector4_t<T_t> &_axis,
                                      float _angle) const {
  auto rad = RAD(_angle);
  Eigen::AngleAxis<float> vec(rad, Eigen::Vector3f(_axis.vector.x(), _axis.vector.y(), _axis.vector.z()));
  auto mat = vec.matrix();
  Eigen::Matrix<float, 4, 4> res;
  res.setIdentity();
  res.block<3, 3>(0, 0) = mat;

  return matrix4_t(res * this->eigen_mat);
}

template <matrix_element_concept_t T_t>
matrix4_t<T_t> matrix4_t<T_t>::translate(T_t _x, T_t _y, T_t _z) const {
  Eigen::Matrix<T_t, ORDER, ORDER> res;
  res.setIdentity();
  res(0, 0) = eigen_mat(0, 0) + _x * eigen_mat(3, 0);
  res(0, 1) = eigen_mat(0, 1) + _x * eigen_mat(3, 1);
  res(0, 2) = eigen_mat(0, 2) + _x * eigen_mat(3, 2);
  res(0, 3) = eigen_mat(0, 3) + _x * eigen_mat(3, 3);

  res(1, 0) = eigen_mat(1, 0) + _y * eigen_mat(3, 0);
  res(1, 1) = eigen_mat(1, 1) + _y * eigen_mat(3, 1);
  res(1, 2) = eigen_mat(1, 2) + _y * eigen_mat(3, 2);
  res(1, 3) = eigen_mat(1, 3) + _y * eigen_mat(3, 3);

  res(2, 0) = eigen_mat(2, 0) + _z * eigen_mat(3, 0);
  res(2, 1) = eigen_mat(2, 1) + _z * eigen_mat(3, 1);
  res(2, 2) = eigen_mat(2, 2) + _z * eigen_mat(3, 2);
  res(2, 3) = eigen_mat(2, 3) + _z * eigen_mat(3, 3);

  res(3, 0) = eigen_mat(3, 0);
  res(3, 1) = eigen_mat(3, 1);
  res(3, 2) = eigen_mat(3, 2);
  res(3, 3) = eigen_mat(3, 3);

  return matrix4_t<T_t>(res);
}

template <matrix_element_concept_t T_t> float matrix4_t<T_t>::RAD(float _deg) {
  return static_cast<float>((M_PI / 180) * (_deg));
}

template <matrix_element_concept_t T_t> float matrix4_t<T_t>::DEG(float _rad) {
  return static_cast<float>((180 / M_PI) * (_rad));
}

typedef matrix4_t<float> matrix4f_t;

#endif /* SIMPLERENDER_MATRIX4_HPP */
