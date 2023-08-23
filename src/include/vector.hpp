
/**
 * @file vector.hpp
 * @brief 向量模版
 * @author Zone.N (Zone.Niuzh@hotmail.com)
 * @version 1.0
 * @date 2022-06-07
 * @copyright MIT LICENSE
 * https://github.com/Simple-XX/SimpleRenderer
 * Based on https://github.com/mmp/pbrt-v3
 * @par change log:
 * <table>
 * <tr><th>Date<th>Author<th>Description
 * <tr><td>2022-06-07<td>Zone.N<td>迁移到 doxygen
 * </table>
 */

#ifndef SIMPLERENDER_VECTOR_HPP
#define SIMPLERENDER_VECTOR_HPP

#include "cassert"
#include "cmath"
#include "iostream"

#include "Eigen/Dense"

#include "log.hpp"

/**
 * @brief 向量元素类型约束
 * @tparam T_t 元素类型
 */
template <class T_t>
concept vector_element_concept_t = std::is_same<T_t, float>::value;

/**
 * @brief 4 维向量，默认为列向量
 * @tparam T_t                 类型
 * @note w 为 0 时表示向量，w 为 1 时表示点
 * @note 没有做数值合法检查
 */
template <vector_element_concept_t T_t> class vector4_t {
public:
  /// @brief 表示一个向量
  static constexpr const T_t W_VECTOR = 0;
  /// @brief 表示一个顶点
  static constexpr const T_t W_POINT = 1;

  Eigen::Vector<T_t, 4> vector;

  /**
   * @brief 构造函数
   */
  vector4_t(void);

  /**
   * @brief 构造函数
   * @param  _v               另一个 vector4_t<T_t>
   */
  vector4_t(const vector4_t<T_t> &_v);
  vector4_t(const Eigen::Vector<T_t, 4> &_v);

  /**
   * @brief 构造函数
   * @param  _x               x 值
   * @param  _y               y 值
   * @param  _z               z 值
   * @param  _w               w 值
   */
  explicit vector4_t(T_t _x, T_t _y, T_t _z = 0, T_t _w = 0);

  /**
   * @brief = 重载
   * @param  _v               另一个 vector4_t<T_t>
   * @return vector4_t<T_t>&  结果
   */
  vector4_t<T_t> &operator=(const vector4_t<T_t> &_v);

  /**
   * @brief == 重载
   * @param  _v               另一个 vector4_t<T_t>
   * @return true             相等
   * @return false            不相等
   */
  bool operator==(const vector4_t<T_t> &_v) const;

  /**
   * @brief != 重载
   * @param  _v               另一个 vector4_t<T_t>
   * @return true             不相等
   * @return false            相等
   */
  bool operator!=(const vector4_t<T_t> &_v) const;

  /**
   * @brief + 重载，向量加
   * @param  _v               另一个 vector4_t<T_t>
   * @return vector4_t<T_t>   结果
   */
  vector4_t<T_t> operator+(const vector4_t<T_t> &_v) const;

  /**
   * @brief += 重载，向量加
   * @param  _v               另一个 vector4_t<T_t>
   * @return vector4_t<T_t>&  结果
   */
  vector4_t<T_t> &operator+=(const vector4_t<T_t> &_v);

  /**
   * @brief - 重载，向量反向
   * @return vector4_t<T_t>  结果
   */
  vector4_t<T_t> operator-(void) const;

  /**
   * @brief - 重载，向量减
   * @param  _v               另一个 vector4_t<T_t>
   * @return vector4_t<T_t>   结果
   */
  vector4_t<T_t> operator-(const vector4_t<T_t> &_v) const;

  /**
   * @brief -= 重载，向量减
   * @param  _v               另一个 vector4_t<T_t>
   * @return vector4_t<T_t>   结果
   */
  vector4_t<T_t> &operator-=(const vector4_t<T_t> &_v);

  /**
   * @brief * 重载，向量数乘
   * @tparam U_t              数的类型
   * @param  _s               数
   * @return vector4_t<T_t>   结果
   */
  template <vector_element_concept_t U_t>
  vector4_t<T_t> operator*(U_t _s) const;

  /**
   * @brief * 重载，向量点积
   * @param  _v               要乘的向量
   * @return T_t              结果
   */
  T_t operator*(const vector4_t<T_t> &_v) const;

  /// @brief *矩阵见 matrix.hpp
  // operator*(const vector4_t<U_t>& _v, const matrix4_t<T_t>& _mat);
  // operator*(const matrix4_t<T_t>& _mat, const vector4_t<U_t>& _v);

  /**
   * @brief *= 重载，向量数乘
   * @tparam U_t              数的类型
   * @param  _s               数
   * @return vector4_t<T_t>&  结果
   */
  template <vector_element_concept_t U_t> vector4_t<T_t> &operator*=(U_t _s);

  /// @brief 行向量*=矩阵见 matrix.hpp
  // operator*=(vector4_t<U_t>& _v, const matrix4_t<T_t>& _mat);
  // operator*=(const matrix4_t<T_t>& _mat, vector4_t<U_t>& _v);

  /**
   * @brief ^ 重载，向量叉积
   * @param  _v               要乘的向量
   * @return vector4_t<T_t>   结果
   * @note w 为 W_VECTOR
   */
  vector4_t<T_t> operator^(const vector4_t<T_t> &_v) const;

  /**
   * @brief / 重载，向量数除
   * @tparam U_t              数的类型
   * @param  _f               数
   * @return vector4_t<T_t>   结果
   */
  template <vector_element_concept_t U_t>
  vector4_t<T_t> operator/(U_t _f) const;

  /**
   * @brief /= 重载，向量数除
   * @tparam U_t              数的类型
   * @param  _f               数
   * @return vector4_t<T_t>&  结果
   */
  template <vector_element_concept_t U_t> vector4_t<T_t> &operator/=(U_t _f);

  /**
   * @brief [] 重载
   * @param  _idx             下标
   * @return const T_t&       结果
   */
  const T_t &operator[](uint32_t _idx) const;

  /**
   * @brief [] 重载
   * @param  _idx             下标
   * @return T_t&             结果
   */
  T_t &operator[](uint32_t _idx);

  /**
   * @brief 长度的平方
   * @return T_t              结果
   * @note w 不参与运算
   */
  T_t length_squared(void) const;

  /**
   * @brief 长度
   * @return T_t              结果
   * @note w 不参与运算
   */
  T_t length(void) const;

  /**
   * @brief 对所有分量取绝对值
   * @param  _v               向量
   * @return vector4_t<T_t>   结果
   * @note w 不参与运算
   */
  vector4_t<T_t> abs(const vector4_t<T_t> &_v);

  /**
   * @brief 归一化
   * @return vector4_t<T_t>   结果
   * @note w 不参与运算
   */
  vector4_t<T_t> normalize(void) const;

  /**
   * @brief 构造最小向量
   * @param  _v               向量
   * @return vector4_t<T_t>   结果
   */
  vector4_t<T_t> min(const vector4_t<T_t> &_v) const;

  /**
   * @brief 构造最大向量
   * @param  _v               向量
   * @return vector4_t<T_t>   结果
   */
  vector4_t<T_t> max(const vector4_t<T_t> &_v) const;

  /**
   * @brief 转换为 float 类型
   * @todo
   */
  // operator vector4_t<float>(void) const;

  friend std::ostream &operator<<(std::ostream &_os, const vector4_t<T_t> &_v) {
    _os << _v.vector;
    return _os;
  }
};

template <vector_element_concept_t T_t> vector4_t<T_t>::vector4_t(void) {
  vector.setZero();

  return;
}

template <vector_element_concept_t T_t>
vector4_t<T_t>::vector4_t(const vector4_t<T_t> &_v) {
  vector = _v.vector;
}

template <vector_element_concept_t T_t>
vector4_t<T_t>::vector4_t(const Eigen::Vector<T_t, 4> &_v) {
  vector = _v;
}

template <vector_element_concept_t T_t>
vector4_t<T_t>::vector4_t(T_t _x, T_t _y, T_t _z, T_t _w) {
  vector.x() = _x;
  vector.y() = _y;
  vector.z() = _z;
  vector.w() = _w;
}

template <vector_element_concept_t T_t>
vector4_t<T_t> &vector4_t<T_t>::operator=(const vector4_t<T_t> &_v) {
  vector = _v.vector;
  return *this;
}

template <vector_element_concept_t T_t>
bool vector4_t<T_t>::operator==(const vector4_t<T_t> &_v) const {
  return vector.isApprox(_v.vector, 1e-5);
}

template <vector_element_concept_t T_t>
bool vector4_t<T_t>::operator!=(const vector4_t<T_t> &_v) const {
  return !vector.isApprox(_v.vector, 1e-5);
}

template <vector_element_concept_t T_t>
vector4_t<T_t> vector4_t<T_t>::operator+(const vector4_t<T_t> &_v) const {
  return vector4_t(vector + _v.vector);
}

template <vector_element_concept_t T_t>
vector4_t<T_t> &vector4_t<T_t>::operator+=(const vector4_t<T_t> &_v) {
  vector += _v.vector;
  return *this;
}

template <vector_element_concept_t T_t>
vector4_t<T_t> vector4_t<T_t>::operator-(void) const {
  return vector4_t<T_t>(-vector);
}

template <vector_element_concept_t T_t>
vector4_t<T_t> vector4_t<T_t>::operator-(const vector4_t<T_t> &_v) const {
  return vector4_t(vector - _v.vector);
}

template <vector_element_concept_t T_t>
vector4_t<T_t> &vector4_t<T_t>::operator-=(const vector4_t<T_t> &_v) {
  vector -= _v.vector;
  return *this;
}

template <vector_element_concept_t T_t>
template <vector_element_concept_t U_t>
vector4_t<T_t> vector4_t<T_t>::operator*(U_t _s) const {
  return vector4_t<T_t>(vector * _s);
}

template <vector_element_concept_t T_t>
T_t vector4_t<T_t>::operator*(const vector4_t<T_t> &_v) const {
  return vector.dot(_v.vector);
}

template <vector_element_concept_t T_t>
template <vector_element_concept_t U_t>
vector4_t<T_t> &vector4_t<T_t>::operator*=(U_t _s) {
  vector *= _s;
  return *this;
}

template <vector_element_concept_t T_t>
vector4_t<T_t> vector4_t<T_t>::operator^(const vector4_t<T_t> &_v) const {
  return vector4_t<T_t>(
      (vector.y() * _v.vector.z()) - (vector.z() * _v.vector.y()),
      (vector.z() * _v.vector.x()) - (vector.x() * _v.vector.z()),
      (vector.x() * _v.vector.y()) - (vector.y() * _v.vector.x()), W_VECTOR);

  //  return vector4_t<T_t>(vector.head<3>().cross(_v.vector.head<3>()));
}

template <vector_element_concept_t T_t>
template <vector_element_concept_t U_t>
vector4_t<T_t> vector4_t<T_t>::operator/(U_t _f) const {
  if (_f == 0) {
    throw std::invalid_argument(log("_f == 0"));
  }

  return vector4_t<T_t>(vector / _f);
}

template <vector_element_concept_t T_t>
template <vector_element_concept_t U_t>
vector4_t<T_t> &vector4_t<T_t>::operator/=(U_t _f) {
  if (_f == 0) {
    throw std::invalid_argument(log("_f == 0"));
  }
  vector /= _f;
  return *this;
}

template <vector_element_concept_t T_t>
const T_t &vector4_t<T_t>::operator[](uint32_t _idx) const {
  return vector[_idx];
}

template <vector_element_concept_t T_t>
T_t &vector4_t<T_t>::operator[](uint32_t _idx) {
  return vector[_idx];
}

template <vector_element_concept_t T_t> T_t vector4_t<T_t>::length(void) const {
  return vector.norm();
}

template <vector_element_concept_t T_t>
vector4_t<T_t> vector4_t<T_t>::abs(const vector4_t<T_t> &_v) {
  return vector4_t<T_t>(std::abs(_v.x), std::abs(_v.y), std::abs(_v.z));
}

template <vector_element_concept_t T_t>
vector4_t<T_t> vector4_t<T_t>::normalize(void) const {
  return vector.normalized();
}

template <vector_element_concept_t T_t>
vector4_t<T_t> vector4_t<T_t>::min(const vector4_t<T_t> &_v) const {
  return vector4_t<T_t>(std::min(vector.x(), _v.vector.x()),
                        std::min(vector.y(), _v.vector.y()),
                        std::min(vector.z(), _v.vector.z()));
}

template <vector_element_concept_t T_t>
vector4_t<T_t> vector4_t<T_t>::max(const vector4_t<T_t> &_v) const {
  return vector4_t<T_t>(std::max(vector.x(), _v.vector.x()),
                        std::max(vector.y(), _v.vector.y()),
                        std::max(vector.z(), _v.vector.z()));
}

typedef vector4_t<float> vector4f_t;

#endif /* SIMPLERENDER_VECTOR_HPP */
