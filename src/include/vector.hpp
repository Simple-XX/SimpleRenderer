
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

#ifndef _VECTOR_HPP_
#define _VECTOR_HPP_

#include "cmath"
#include "cassert"
#include "iostream"
#include "log.hpp"

/**
 * @brief 4 维向量
 * @tparam _T                类型
 * @note w 不一般参与运算
 */
template <class _T>
class vector4_t {
public:
    _T x;
    _T y;
    _T z;
    _T w;

    /**
     * @brief 构造函数
     */
    vector4_t(void);

    /**
     * @brief 构造函数
     * @param  _x              x 值
     * @param  _y              y 值
     * @param  _z              z 值
     * @param  _w              w 值
     */
    explicit vector4_t(const _T &_x, const _T &_y, const _T &_z = 0,
                       const _T &_w = 1);

    /**
     * @brief 构造函数
     * @param  _v               另一个 vector4_t<_T>
     */
    explicit vector4_t(const vector4_t<_T> &_v);

    /**
     * @brief = 重载
     * @param  _v               另一个 vector4_t<_T>
     * @return vector4_t<_T>&   结果
     */
    vector4_t<_T> &operator=(const vector4_t<_T> &_v);

    /**
     * @brief == 重载
     * @param  _v              另一个 vector4_t<_T>
     * @return true            相等
     * @return false           不相等
     */
    bool operator==(const vector4_t<_T> &_v) const;

    /**
     * @brief != 重载
     * @param  _v              另一个 vector4_t<_T>
     * @return true            不相等
     * @return false           相等
     */
    bool operator!=(const vector4_t<_T> &_v) const;

    /**
     * @brief + 重载，向量加
     * @param  _v               另一个 vector4_t<_T>
     * @return const vector4_t<_T>  结果
     */
    const vector4_t<_T> operator+(const vector4_t<_T> &_v) const;

    /**
     * @brief += 重载，向量加
     * @param  _v               另一个 vector4_t<_T>
     * @return vector4_t<_T>&     结果
     */
    vector4_t<_T> &operator+=(const vector4_t<_T> &_v);

    /**
     * @brief - 重载，向量反向
     * @return const vector4_t<_T>  结果
     */
    const vector4_t<_T> operator-(void) const;

    /**
     * @brief - 重载，向量减
     * @param  _v               另一个 vector4_t<_T>
     * @return const vector4_t<_T>  结果
     */
    const vector4_t<_T> operator-(const vector4_t<_T> &_v) const;

    /**
     * @brief -= 重载，向量减
     * @param  _v               另一个 vector4_t<_T>
     * @return vector4_t<_T>      结果
     */
    vector4_t<_T> &operator-=(const vector4_t<_T> &_v);

    /**
     * @brief * 重载，向量数乘
     * @tparam _U              数的类型
     * @param  _s              数
     * @return const vector4_t<_T>  结果
     */
    template <class _U>
    const vector4_t<_T> operator*(const _U _s) const;

    /**
     * @brief * 重载，向量点积
     * @param  _v               要乘的向量
     * @return const _T         结果
     */
    const _T operator*(const vector4_t<_T> &_v) const;

    /// @brief *矩阵见 matrix.hpp

    /**
     * @brief *= 重载，向量数乘
     * @tparam _U              数的类型
     * @param  _s              数
     * @return vector4_t<_T>&    结果
     */
    template <class _U>
    vector4_t<_T> &operator*=(const _U _s);

    /// @brief 行向量*=矩阵见 matrix.hpp

    /**
     * @brief ^ 重载，向量叉积
     * @param  _v              要乘的向量
     * @return const vector4_t<_T>  结果
     */
    const vector4_t<_T> operator^(const vector4_t<_T> &_v) const;

    /**
     * @brief / 重载，向量数除
     * @tparam _U              数的类型
     * @param  _f              数
     * @return vector4_t<_T>     结果
     */
    template <class _U>
    const vector4_t<_T> operator/(const _U _f) const;

    /**
     * @brief /= 重载，向量数除
     * @tparam _U              数的类型
     * @param  _f              数
     * @return vector4_t<_T>&    结果
     */
    template <class _U>
    vector4_t<_T> &operator/=(const _U _f);

    /**
     * @brief [] 重载
     * @param  _idx            下标
     * @return const _T        结果
     */
    const _T operator[](const uint32_t _idx) const;

    /**
     * @brief [] 重载
     * @param  _idx            下标
     * @return _T              结果
     */
    _T operator[](const uint32_t _idx);

    /**
     * @brief 距离^2
     * @return const _T         结果
     */
    const _T length_squared(void) const;

    /**
     * @brief 距离
     * @return const _T         结果
     */
    const _T length(void) const;

    /**
     * @brief 是否有非数值
     * @return true             有
     * @return false            无
     */
    bool HasNaNs(void) const;

    /**
     * @brief 对所有分量取绝对值
     * @param  _v              向量
     * @return const vector4_t<_T>  结果
     */
    const vector4_t<_T> abs(const vector4_t<_T> &_v);

    /**
     * @brief 归一化
     * @param  _v              向量
     * @return const vector4_t<_T>  结果
     */
    const vector4_t<_T> normalize(void) const;

    /**
     * @brief 构造最小向量
     * @param  _v             向量
     * @return const vector4_t<_T>  结果
     */
    const vector4_t<_T> min(const vector4_t<_T> &_v) const;

    /**
     * @brief 构造最大向量
     * @param  _v             向量
     * @return const vector4_t<_T>  结果
     */
    const vector4_t<_T> max(const vector4_t<_T> &_v) const;

    /**
     * @brief 转换为 float 类型
     */
    operator vector4_t<float>(void) const;

    friend std::ostream &operator<<(std::ostream        &_os,
                                    const vector4_t<_T> &_v) {
        _os << "[ " << _v.x << ", " << _v.y << ", " << _v.z << ", " << _v.w
            << " ]";
        return _os;
    }
};

template <class _T>
vector4_t<_T>::vector4_t(void) {
    x = 0;
    y = 0;
    z = 0;
    w = 1;
    return;
}

template <class _T>
vector4_t<_T>::vector4_t(const _T &_x, const _T &_y, const _T &_z, const _T &_w)
    : x(_x), y(_y), z(_z), w(_w) {
    if (HasNaNs()) {
        throw std::invalid_argument(log("HasNaNs()"));
    }
    return;
}

template <class _T>
vector4_t<_T>::vector4_t(const vector4_t<_T> &_v) {
    if (_v.HasNaNs()) {
        throw std::invalid_argument(log("_v.HasNaNs()"));
    }
    x = _v.x;
    y = _v.y;
    z = _v.z;
    w = _v.w;
    return;
}

template <class _T>
vector4_t<_T> &vector4_t<_T>::operator=(const vector4_t<_T> &_v) {
    if (_v.HasNaNs()) {
        throw std::invalid_argument(log("_v.HasNaNs()"));
    }
    x = _v.x;
    y = _v.y;
    z = _v.z;
    w = _v.w;
    return *this;
}

template <class _T>
bool vector4_t<_T>::operator==(const vector4_t<_T> &_v) const {
    if (_v.HasNaNs()) {
        throw std::invalid_argument(log("_v.HasNaNs()"));
    }
    return x == _v.x && y == _v.y && z == _v.z && w == _v.w;
}

template <class _T>
bool vector4_t<_T>::operator!=(const vector4_t<_T> &_v) const {
    if (_v.HasNaNs()) {
        throw std::invalid_argument(log("_v.HasNaNs()"));
    }
    return x != _v.x || y != _v.y || z != _v.z || w != _v.w;
}

template <class _T>
const vector4_t<_T> vector4_t<_T>::operator+(const vector4_t<_T> &_v) const {
    if (_v.HasNaNs()) {
        throw std::invalid_argument(log("_v.HasNaNs()"));
    }
    return vector4_t(x + _v.x, y + _v.y, z + _v.z);
}

template <class _T>
vector4_t<_T> &vector4_t<_T>::operator+=(const vector4_t<_T> &_v) {
    if (_v.HasNaNs()) {
        throw std::invalid_argument(log("_v.HasNaNs()"));
    }
    x += _v.x;
    y += _v.y;
    z += _v.z;
    return *this;
}

template <class _T>
const vector4_t<_T> vector4_t<_T>::operator-(void) const {
    if (HasNaNs()) {
        throw std::invalid_argument(log("HasNaNs()"));
    }
    return vector4_t<_T>(-x, -y, -z);
}

template <class _T>
const vector4_t<_T> vector4_t<_T>::operator-(const vector4_t<_T> &_v) const {
    if (_v.HasNaNs()) {
        throw std::invalid_argument(log("_v.HasNaNs()"));
    }
    return vector4_t(x - _v.x, y - _v.y, z - _v.z);
}

template <class _T>
vector4_t<_T> &vector4_t<_T>::operator-=(const vector4_t<_T> &_v) {
    if (_v.HasNaNs()) {
        throw std::invalid_argument(log("_v.HasNaNs()"));
    }
    x -= _v.x;
    y -= _v.y;
    z -= _v.z;
    return *this;
}

template <class _T>
template <class _U>
const vector4_t<_T> vector4_t<_T>::operator*(const _U _s) const {
    if (std::isnan(_s)) {
        throw std::invalid_argument(log("std::isnan(_s)"));
    }
    return vector4_t<_T>(x * _s, y * _s, z * _s, w * _s);
}

template <class _T>
const _T vector4_t<_T>::operator*(const vector4_t<_T> &_v) const {
    if (_v.HasNaNs()) {
        throw std::invalid_argument(log("_v.HasNaNs()"));
    }
    return x * _v.x + y * _v.y + z * _v.z;
}

template <class _T>
template <class _U>
vector4_t<_T> &vector4_t<_T>::operator*=(const _U _s) {
    if (std::isnan(_s)) {
        throw std::invalid_argument(log("std::isnan(_s)"));
    }
    x *= _s;
    y *= _s;
    z *= _s;
    w *= _s;
    return *this;
}

template <class _T>
const vector4_t<_T> vector4_t<_T>::operator^(const vector4_t<_T> &_v) const {
    if (_v.HasNaNs()) {
        throw std::invalid_argument(log("_v.HasNaNs()"));
    }
    return vector4_t<_T>((y * _v.z) - (z * _v.y), (z * _v.x) - (x * _v.z),
                         (x * _v.y) - (y * _v.x));
}

template <class _T>
template <class _U>
const vector4_t<_T> vector4_t<_T>::operator/(const _U _f) const {
    if (_f == 0) {
        throw std::invalid_argument(log("_f == 0"));
    }
    _T inv = (_T)1 / _f;
    return vector4_t<_T>(x * inv, y * inv, z * inv, w * inv);
}

template <class _T>
template <class _U>
vector4_t<_T> &vector4_t<_T>::operator/=(const _U _f) {
    if (_f == 0) {
        throw std::invalid_argument(log("_f == 0"));
    }
    _T inv = (_T)1 / _f;
    x *= inv;
    y *= inv;
    z *= inv;
    w *= inv;
    return *this;
}

template <class _T>
const _T vector4_t<_T>::operator[](const uint32_t _idx) const {
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

template <class _T>
_T vector4_t<_T>::operator[](const uint32_t _idx) {
    if (_idx > 3) {
        throw std::invalid_argument(log("_idx > 3"));
    }
    else if (_idx == 0) {
        return x;
    }
    else if (_idx == 1) {
        return y;
    }
    else if (_idx == 1) {
        return z;
    }
    else {
        return w;
    }
}

template <class _T>
bool vector4_t<_T>::HasNaNs(void) const {
    return std::isnan(x) || std::isnan(y) || std::isnan(z) || std::isnan(w);
}

template <class _T>
const _T vector4_t<_T>::length_squared(void) const {
    if (HasNaNs()) {
        throw std::invalid_argument(log("HasNaNs()"));
    }
    return x * x + y * y + z * z;
}

template <class _T>
const _T vector4_t<_T>::length(void) const {
    if (HasNaNs()) {
        throw std::invalid_argument(log("HasNaNs()"));
    }
    return std::sqrt(length_squared());
}

template <class _T>
const vector4_t<_T> vector4_t<_T>::abs(const vector4_t<_T> &_v) {
    if (HasNaNs()) {
        throw std::invalid_argument(log("HasNaNs()"));
    }
    return vector4_t<_T>(std::abs(_v.x), std::abs(_v.y), std::abs(_v.z));
}

template <class _T>
const vector4_t<_T> vector4_t<_T>::normalize(void) const {
    if (HasNaNs()) {
        throw std::invalid_argument(log("HasNaNs()"));
    }
    if (length() == 0) {
        return vector4_t<_T>();
    }
    return vector4_t<_T>(x / length(), y / length(), z / length(), 1);
}

template <class _T>
const vector4_t<_T> vector4_t<_T>::min(const vector4_t<_T> &_v) const {
    if (HasNaNs()) {
        throw std::invalid_argument(log("HasNaNs()"));
    }
    return vector4_t<_T>(std::min(x, _v.x), std::min(y, _v.y),
                         std::min(z, _v.z));
}

template <class _T>
const vector4_t<_T> vector4_t<_T>::max(const vector4_t<_T> &_v) const {
    if (HasNaNs()) {
        throw std::invalid_argument(log("HasNaNs()"));
    }
    return vector4_t<_T>(std::max(x, _v.x), std::max(y, _v.y),
                         std::max(z, _v.z));
}

typedef vector4_t<float> vector4f_t;

#endif /* _VECTOR_HPP_ */
