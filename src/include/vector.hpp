
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
template <vector_element_concept_t T_t>
class vector4_t {
public:
    /// @brief 表示一个向量
    static constexpr const T_t W_VECTOR = 0;
    /// @brief 表示一个顶点
    static constexpr const T_t W_POINT  = 1;

    T_t                        x;
    T_t                        y;
    T_t                        z;
    T_t                        w;

    /**
     * @brief 构造函数
     */
    vector4_t(void);

    /**
     * @brief 构造函数
     * @param  _v               另一个 vector4_t<T_t>
     */
    vector4_t(const vector4_t<T_t>& _v);

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
    vector4_t<T_t>& operator=(const vector4_t<T_t>& _v);

    /**
     * @brief == 重载
     * @param  _v               另一个 vector4_t<T_t>
     * @return true             相等
     * @return false            不相等
     */
    bool            operator==(const vector4_t<T_t>& _v) const;

    /**
     * @brief != 重载
     * @param  _v               另一个 vector4_t<T_t>
     * @return true             不相等
     * @return false            相等
     */
    bool            operator!=(const vector4_t<T_t>& _v) const;

    /**
     * @brief + 重载，向量加
     * @param  _v               另一个 vector4_t<T_t>
     * @return vector4_t<T_t>   结果
     */
    vector4_t<T_t>  operator+(const vector4_t<T_t>& _v) const;

    /**
     * @brief += 重载，向量加
     * @param  _v               另一个 vector4_t<T_t>
     * @return vector4_t<T_t>&  结果
     */
    vector4_t<T_t>& operator+=(const vector4_t<T_t>& _v);

    /**
     * @brief - 重载，向量反向
     * @return vector4_t<T_t>  结果
     */
    vector4_t<T_t>  operator-(void) const;

    /**
     * @brief - 重载，向量减
     * @param  _v               另一个 vector4_t<T_t>
     * @return vector4_t<T_t>   结果
     */
    vector4_t<T_t>  operator-(const vector4_t<T_t>& _v) const;

    /**
     * @brief -= 重载，向量减
     * @param  _v               另一个 vector4_t<T_t>
     * @return vector4_t<T_t>   结果
     */
    vector4_t<T_t>& operator-=(const vector4_t<T_t>& _v);

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
    T_t            operator*(const vector4_t<T_t>& _v) const;

    /// @brief *矩阵见 matrix.hpp
    // operator*(const vector4_t<U_t>& _v, const matrix4_t<T_t>& _mat);
    // operator*(const matrix4_t<T_t>& _mat, const vector4_t<U_t>& _v);

    /**
     * @brief *= 重载，向量数乘
     * @tparam U_t              数的类型
     * @param  _s               数
     * @return vector4_t<T_t>&  结果
     */
    template <vector_element_concept_t U_t>
    vector4_t<T_t>& operator*=(U_t _s);

    /// @brief 行向量*=矩阵见 matrix.hpp
    // operator*=(vector4_t<U_t>& _v, const matrix4_t<T_t>& _mat);
    // operator*=(const matrix4_t<T_t>& _mat, vector4_t<U_t>& _v);

    /**
     * @brief ^ 重载，向量叉积
     * @param  _v               要乘的向量
     * @return vector4_t<T_t>   结果
     * @note w 为 W_VECTOR
     */
    vector4_t<T_t>  operator^(const vector4_t<T_t>& _v) const;

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
    template <vector_element_concept_t U_t>
    vector4_t<T_t>& operator/=(U_t _f);

    /**
     * @brief [] 重载
     * @param  _idx             下标
     * @return const T_t&       结果
     */
    const T_t&      operator[](uint32_t _idx) const;

    /**
     * @brief [] 重载
     * @param  _idx             下标
     * @return T_t&             结果
     */
    T_t&            operator[](uint32_t _idx);

    /**
     * @brief 长度的平方
     * @return size_t           结果
     * @note w 不参与运算
     */
    size_t          length_squared(void) const;

    /**
     * @brief 长度
     * @return size_t           结果
     * @note w 不参与运算
     */
    size_t          length(void) const;

    /**
     * @brief 对所有分量取绝对值
     * @param  _v               向量
     * @return vector4_t<T_t>   结果
     * @note w 不参与运算
     */
    vector4_t<T_t>  abs(const vector4_t<T_t>& _v);

    /**
     * @brief 归一化
     * @return vector4_t<T_t>   结果
     * @note w 不参与运算
     */
    vector4_t<T_t>  normalize(void) const;

    /**
     * @brief 构造最小向量
     * @param  _v               向量
     * @return vector4_t<T_t>   结果
     */
    vector4_t<T_t>  min(const vector4_t<T_t>& _v) const;

    /**
     * @brief 构造最大向量
     * @param  _v               向量
     * @return vector4_t<T_t>   结果
     */
    vector4_t<T_t>  max(const vector4_t<T_t>& _v) const;

    /**
     * @brief 转换为 float 类型
     * @todo
     */
    // operator vector4_t<float>(void) const;

    friend std::ostream&
    operator<<(std::ostream& _os, const vector4_t<T_t>& _v) {
        _os.setf(std::ios::right);
        _os.precision(16);

        _os << "[ " << _v.x << ", " << _v.y << ", " << _v.z << ", " << _v.w
            << " ]";

        _os.unsetf(std::ios::right);
        _os.precision(6);
        return _os;
    }
};

template <vector_element_concept_t T_t>
vector4_t<T_t>::vector4_t(void) {
    x = 0;
    y = 0;
    z = 0;
    w = 0;
    return;
}

template <vector_element_concept_t T_t>
vector4_t<T_t>::vector4_t(const vector4_t<T_t>& _v) {
    x = _v.x;
    y = _v.y;
    z = _v.z;
    w = _v.w;
    return;
}

template <vector_element_concept_t T_t>
vector4_t<T_t>::vector4_t(T_t _x, T_t _y, T_t _z, T_t _w)
    : x(_x), y(_y), z(_z), w(_w) {
    return;
}

template <vector_element_concept_t T_t>
vector4_t<T_t>& vector4_t<T_t>::operator=(const vector4_t<T_t>& _v) {
    x = _v.x;
    y = _v.y;
    z = _v.z;
    w = _v.w;
    return *this;
}

template <vector_element_concept_t T_t>
bool vector4_t<T_t>::operator==(const vector4_t<T_t>& _v) const {
    if (std::abs(x - _v.x) > std::numeric_limits<T_t>::epsilon()) {
        return false;
    }

    if (std::abs(y - _v.y) > std::numeric_limits<T_t>::epsilon()) {
        return false;
    }

    if (std::abs(z - _v.z) > std::numeric_limits<T_t>::epsilon()) {
        return false;
    }

    if (std::abs(w - _v.w) > std::numeric_limits<T_t>::epsilon()) {
        return false;
    }

    return true;
}

template <vector_element_concept_t T_t>
bool vector4_t<T_t>::operator!=(const vector4_t<T_t>& _v) const {
    if (std::abs(x - _v.x) > std::numeric_limits<T_t>::epsilon()) {
        return true;
    }

    if (std::abs(y - _v.y) > std::numeric_limits<T_t>::epsilon()) {
        return true;
    }

    if (std::abs(z - _v.z) > std::numeric_limits<T_t>::epsilon()) {
        return true;
    }

    if (std::abs(w - _v.w) > std::numeric_limits<T_t>::epsilon()) {
        return true;
    }

    return false;
}

template <vector_element_concept_t T_t>
vector4_t<T_t> vector4_t<T_t>::operator+(const vector4_t<T_t>& _v) const {
    return vector4_t(x + _v.x, y + _v.y, z + _v.z, w + _v.w);
}

template <vector_element_concept_t T_t>
vector4_t<T_t>& vector4_t<T_t>::operator+=(const vector4_t<T_t>& _v) {
    x += _v.x;
    y += _v.y;
    z += _v.z;
    w += _v.w;
    return *this;
}

template <vector_element_concept_t T_t>
vector4_t<T_t> vector4_t<T_t>::operator-(void) const {
    return vector4_t<T_t>(-x, -y, -z, -w);
}

template <vector_element_concept_t T_t>
vector4_t<T_t> vector4_t<T_t>::operator-(const vector4_t<T_t>& _v) const {
    return vector4_t(x - _v.x, y - _v.y, z - _v.z, w - _v.w);
}

template <vector_element_concept_t T_t>
vector4_t<T_t>& vector4_t<T_t>::operator-=(const vector4_t<T_t>& _v) {
    x -= _v.x;
    y -= _v.y;
    z -= _v.z;
    w -= _v.w;
    return *this;
}

template <vector_element_concept_t T_t>
template <vector_element_concept_t U_t>
vector4_t<T_t> vector4_t<T_t>::operator*(U_t _s) const {
    return vector4_t<T_t>(x * _s, y * _s, z * _s, w * _s);
}

template <vector_element_concept_t T_t>
T_t vector4_t<T_t>::operator*(const vector4_t<T_t>& _v) const {
    return x * _v.x + y * _v.y + z * _v.z + w * _v.w;
}

template <vector_element_concept_t T_t>
template <vector_element_concept_t U_t>
vector4_t<T_t>& vector4_t<T_t>::operator*=(U_t _s) {
    x *= _s;
    y *= _s;
    z *= _s;
    w *= _s;
    return *this;
}

template <vector_element_concept_t T_t>
vector4_t<T_t> vector4_t<T_t>::operator^(const vector4_t<T_t>& _v) const {
    return vector4_t<T_t>((y * _v.z) - (z * _v.y), (z * _v.x) - (x * _v.z),
                          (x * _v.y) - (y * _v.x), W_VECTOR);
}

template <vector_element_concept_t T_t>
template <vector_element_concept_t U_t>
vector4_t<T_t> vector4_t<T_t>::operator/(U_t _f) const {
    if (_f == 0) {
        throw std::invalid_argument(log("_f == 0"));
    }
    T_t inv = (T_t)1 / _f;
    return vector4_t<T_t>(x * inv, y * inv, z * inv, w * inv);
}

template <vector_element_concept_t T_t>
template <vector_element_concept_t U_t>
vector4_t<T_t>& vector4_t<T_t>::operator/=(U_t _f) {
    if (_f == 0) {
        throw std::invalid_argument(log("_f == 0"));
    }
    T_t inv = (T_t)1 / _f;
    x       *= inv;
    y       *= inv;
    z       *= inv;
    w       *= inv;
    return *this;
}

template <vector_element_concept_t T_t>
const T_t& vector4_t<T_t>::operator[](uint32_t _idx) const {
    if (_idx > 3) {
        throw std::invalid_argument(log("_idx > 3"));
    }
    else if (_idx == 0) {
        return x;
    }
    else if (_idx == 1) {
        return y;
    }
    else if (_idx == 2) {
        return z;
    }
    else {
        return w;
    }
}

template <vector_element_concept_t T_t>
T_t& vector4_t<T_t>::operator[](uint32_t _idx) {
    if (_idx > 3) {
        throw std::invalid_argument(log("_idx > 3"));
    }
    else if (_idx == 0) {
        return x;
    }
    else if (_idx == 1) {
        return y;
    }
    else if (_idx == 2) {
        return z;
    }
    else {
        return w;
    }
}

template <vector_element_concept_t T_t>
size_t vector4_t<T_t>::length_squared(void) const {
    return x * x + y * y + z * z + w * w;
}

template <vector_element_concept_t T_t>
size_t vector4_t<T_t>::length(void) const {
    return std::sqrt(length_squared());
}

template <vector_element_concept_t T_t>
vector4_t<T_t> vector4_t<T_t>::abs(const vector4_t<T_t>& _v) {
    return vector4_t<T_t>(std::abs(_v.x), std::abs(_v.y), std::abs(_v.z));
}

template <vector_element_concept_t T_t>
vector4_t<T_t> vector4_t<T_t>::normalize(void) const {
    if (length() == 0) {
        return vector4_t<T_t>();
    }
    return vector4_t<T_t>(x / length(), y / length(), z / length(),
                          w / length());
}

template <vector_element_concept_t T_t>
vector4_t<T_t> vector4_t<T_t>::min(const vector4_t<T_t>& _v) const {
    return vector4_t<T_t>(std::min(x, _v.x), std::min(y, _v.y),
                          std::min(z, _v.z));
}

template <vector_element_concept_t T_t>
vector4_t<T_t> vector4_t<T_t>::max(const vector4_t<T_t>& _v) const {
    return vector4_t<T_t>(std::max(x, _v.x), std::max(y, _v.y),
                          std::max(z, _v.z));
}

typedef vector4_t<float> vector4f_t;

#endif /* SIMPLERENDER_VECTOR_HPP */
