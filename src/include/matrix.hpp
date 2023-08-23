
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
    return _v.vector * _mat.eigen_mat;
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
    return _mat.eigen_mat * _v.vector;
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
   * @brief 旋转矩阵，Rodriguez 方法
   * @param  _axis            旋转轴，起点为原点，单位向量
   * @param  _rad             要旋转的弧度
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
  matrix4_t<T_t> rotate(const vector4_t<T_t> &_axis, float _rad) const;

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
matrix4_t<T_t>::matrix4_t(const vector4_t<T_t> &_v) {
  eigen_mat.diagonal() = _v.vector;
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
matrix4_t<T_t> matrix4_t<T_t>::rotate(const vector4_t<T_t> &_axis,
                                      float _rad) const {
  Eigen::AngleAxis<float> vec(
      _rad,
      Eigen::Vector3f(_axis.vector.x(), _axis.vector.y(), _axis.vector.z()));
  auto mat = vec.matrix();
  Eigen::Matrix<float, 4, 4> res;
  res.setIdentity();
  res.block<3, 3>(0, 0) = mat;

  return matrix4_t(res * this->eigen_mat);
}

typedef matrix4_t<float> matrix4f_t;

// #include "Eigen/Dense"
// typedef Eigen::Matrix<float, 4, 4> matrix4f_t;

#endif /* SIMPLERENDER_MATRIX4_HPP */
