
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

// 前置声明
template <class _T>
class vector2_t;

template <class _T>
class vector3_t;

/**
 * @brief 2 维向量
 * @tparam _T                类型
 */
template <class _T>
class vector2_t {
public:
    _T x;
    _T y;

    /**
     * @brief 构造函数
     */
    vector2_t(void);

    /**
     * @brief 构造函数
     * @param  _x              x 值
     * @param  _y              y 值
     */
    vector2_t(const _T &_x, const _T &_y);

    /**
     * @brief 构造函数
     * @param  _v              另一个 vector2_t<_T>
     */
    vector2_t(const vector2_t<_T> &_v);

    /**
     * @brief = 重载
     * @param  _v               另一个 vector2_t<_T>
     * @return vector2_t<_T>&     结果
     */
    vector2_t<_T> &operator=(const vector2_t<_T> &_v);

    /**
     * @brief == 重载
     * @param  _v              另一个 vector2_t<_T>
     * @return true            相等
     * @return false           不相等
     */
    bool operator==(const vector2_t<_T> &_v) const;

    /**
     * @brief != 重载
     * @param  _v              另一个 vector2_t<_T>
     * @return true            不相等
     * @return false           相等
     */
    bool operator!=(const vector2_t<_T> &_v) const;

    /**
     * @brief + 重载
     * @param  _v              另一个 vector2_t<_T>
     * @return const vector2_t<_T>  结果
     */
    const vector2_t<_T> operator+(const vector2_t<_T> &_v) const;

    /**
     * @brief += 重载
     * @param  _v              另一个 vector2_t<_T>
     * @return vector2_t<_T>   结果
     */
    vector2_t<_T> &operator+=(const vector2_t<_T> &_v);

    /**
     * @brief - 重载，向量反向
     * @return const vector2_t<_T>  结果
     */
    const vector2_t<_T> operator-(void) const;

    /**
     * @brief - 重载
     * @param  _v              另一个 vector2_t<_T>
     * @return const vector2_t<_T>  结果
     */
    const vector2_t<_T> operator-(const vector2_t<_T> &_v) const;

    /**
     * @brief -= 重载
     * @param  _v              另一个 vector2_t<_T>
     * @return vector2_t<_T>   结果
     */
    vector2_t<_T> &operator-=(const vector2_t<_T> &_v);

    /**
     * @brief * 重载，向量数乘
     * @tparam _U              相乘的数的类型
     * @param  _f              相乘的数
     * @return const vector2_t<_T>  结果
     */
    template <class _U>
    const vector2_t<_T> operator*(const _U &_f) const;

    /**
     * @brief * 重载，向量点积
     * @param  _v              要乘的向量
     * @return const _T        结果
     */
    const _T operator*(const vector2_t<_T> &_v) const;

    /**
     * @brief *= 重载，向量数乘
     * @tparam _U              相乘的数的类型
     * @param  _f              相乘的数
     * @return vector2_t<_T>     结果
     */
    template <class _U>
    vector2_t<_T> &operator*=(const _U &_f);

    /**
     * @brief / 重载，向量数除
     * @tparam _U              相除的数的类型
     * @param  _f              相除的数
     * @return const vector2_t<_T>  结果
     */
    template <class _U>
    const vector2_t<_T> operator/(const _U &_f) const;

    /**
     * @brief /= 重载，向量数除
     * @tparam _U              相除的数的类型
     * @param  _f              相除的数
     * @return vector2_t<_T>     结果
     */
    template <class _U>
    vector2_t<_T> &operator/=(const _U &_f);

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
     * @brief 转换为 int32_t 类型
     */
    operator vector2_t<int32_t>(void) const;

    /**
     * @brief 转换为 float 类型
     */
    operator vector2_t<float>(void) const;

    /**
     * @brief 是否有非数值
     * @return true             有
     * @return false            无
     */
    bool HasNaNs(void) const;

    /**
     * @brief 距离^2
     * @return const  _T        距离的平方
     */
    const _T length_squared(void) const;

    /**
     * @brief 距离
     * @return const  _T        距离
     */
    const _T length(void) const;

    /**
     * @brief 对所有分量取绝对值
     * @return const vector2_t<_T>  结果
     */
    const vector2_t<_T> abs(void) const;

    /**
     * @brief 归一化
     * @return const vector2_t<_T>  结果
     */
    const vector2_t<_T> normalize(void) const;

    /**
     * @brief 使用两个向量的较小分量构建新的向量
     * @param  _v               另一个向量
     * @return const vector2_t<_T>  新的向量
     */
    const vector2_t<_T> min(const vector2_t<_T> &_v) const;

    /**
     * @brief 使用两个向量的较大分量构建新的向量
     * @param  _v               另一个向量
     * @return const vector2_t<_T>  新的向量
     */
    const vector2_t<_T> max(const vector2_t<_T> &_v) const;

    friend std::ostream &operator<<(std::ostream        &_os,
                                    const vector2_t<_T> &_v) {
        _os << "[ " << _v.x << ", " << _v.y << " ]";
        return _os;
    }
};

template <class _T>
vector2_t<_T>::vector2_t(void) {
    x = 0;
    y = 0;
    return;
}

template <class _T>
vector2_t<_T>::vector2_t(const _T &_x, const _T &_y) : x(_x), y(_y) {
    try {
        if (HasNaNs()) {
            throw std::invalid_argument(log("HasNaNs()"));
        }
    } catch (const std::invalid_argument &e) {
        std::cerr << e.what() << std::endl;
        return;
    }
    return;
}

template <class _T>
vector2_t<_T>::vector2_t(const vector2_t<_T> &_v) {
    try {
        if (_v.HasNaNs()) {
            throw std::invalid_argument(log("_v.HasNaNs()"));
        }
    } catch (const std::invalid_argument &e) {
        std::cerr << e.what() << std::endl;
        *this = vector2_t();
        return;
    }
    x = _v.x;
    y = _v.y;
    return;
}

template <class _T>
vector2_t<_T> &vector2_t<_T>::operator=(const vector2_t<_T> &_v) {
    try {
        if (_v.HasNaNs()) {
            throw std::invalid_argument(log("_v.HasNaNs()"));
        }
    } catch (const std::invalid_argument &e) {
        std::cerr << e.what() << std::endl;
        return *this;
    }
    x = _v.x;
    y = _v.y;
    return *this;
}

template <class _T>
bool vector2_t<_T>::operator==(const vector2_t<_T> &_v) const {
    try {
        if (_v.HasNaNs()) {
            throw std::invalid_argument(log("_v.HasNaNs()"));
        }
    } catch (const std::invalid_argument &e) {
        std::cerr << e.what() << std::endl;
        return false;
    }
    return x == _v.x && y == _v.y;
}

template <class _T>
bool vector2_t<_T>::operator!=(const vector2_t<_T> &_v) const {
    try {
        if (_v.HasNaNs()) {
            throw std::invalid_argument(log("_v.HasNaNs()"));
        }
    } catch (const std::invalid_argument &e) {
        std::cerr << e.what() << std::endl;
        return false;
    }
    return x != _v.x || y != _v.y;
}

template <class _T>
const vector2_t<_T> vector2_t<_T>::operator+(const vector2_t<_T> &_v) const {
    try {
        if (_v.HasNaNs()) {
            throw std::invalid_argument(log("_v.HasNaNs()"));
        }
    } catch (const std::invalid_argument &e) {
        std::cerr << e.what() << std::endl;
        return *this;
    }
    return vector2_t(x + _v.x, y + _v.y);
}

template <class _T>
vector2_t<_T> &vector2_t<_T>::operator+=(const vector2_t<_T> &_v) {
    try {
        if (_v.HasNaNs()) {
            throw std::invalid_argument(log("_v.HasNaNs()"));
        }
    } catch (const std::invalid_argument &e) {
        std::cerr << e.what() << std::endl;
        return *this;
    }
    x += _v.x;
    y += _v.y;
    return *this;
}

template <class _T>
const vector2_t<_T> vector2_t<_T>::operator-(void) const {
    try {
        if (HasNaNs()) {
            throw std::invalid_argument(log("HasNaNs()"));
        }
    } catch (const std::invalid_argument &e) {
        std::cerr << e.what() << std::endl;
        return *this;
    }
    return vector2_t<_T>(-x, -y);
}

template <class _T>
const vector2_t<_T> vector2_t<_T>::operator-(const vector2_t<_T> &_v) const {
    try {
        if (_v.HasNaNs()) {
            throw std::invalid_argument(log("_v.HasNaNs()"));
        }
    } catch (const std::invalid_argument &e) {
        std::cerr << e.what() << std::endl;
        return *this;
    }
    return vector2_t(x - _v.x, y - _v.y);
}

template <class _T>
vector2_t<_T> &vector2_t<_T>::operator-=(const vector2_t<_T> &_v) {
    try {
        if (_v.HasNaNs()) {
            throw std::invalid_argument(log("_v.HasNaNs()"));
        }
    } catch (const std::invalid_argument &e) {
        std::cerr << e.what() << std::endl;
        return *this;
    }
    x -= _v.x;
    y -= _v.y;
    return *this;
}

template <class _T>
template <class _U>
const vector2_t<_T> vector2_t<_T>::operator*(const _U &_f) const {
    try {
        if (std::isnan(_f)) {
            throw std::invalid_argument(log("std::isnan(_f)"));
        }
    } catch (const std::invalid_argument &e) {
        std::cerr << e.what() << std::endl;
        return *this;
    }
    return vector2_t<_T>(_f * x, _f * y);
}

template <class _T>
const _T vector2_t<_T>::operator*(const vector2_t<_T> &_v) const {
    try {
        if (_v.HasNaNs()) {
            throw std::invalid_argument(log("_v.HasNaNs()"));
        }
    } catch (const std::invalid_argument &e) {
        std::cerr << e.what() << std::endl;
        return 0;
    }
    return x * _v.x + y * _v.y;
}

template <class _T>
template <class _U>
vector2_t<_T> &vector2_t<_T>::operator*=(const _U &_f) {
    try {
        if (std::isnan(_f)) {
            throw std::invalid_argument(log("std::isnan(_f)"));
        }
    } catch (const std::invalid_argument &e) {
        std::cerr << e.what() << std::endl;
        return *this;
    }
    x *= _f;
    y *= _f;
    return *this;
}

template <class _T>
template <class _U>
const vector2_t<_T> vector2_t<_T>::operator/(const _U &_f) const {
    try {
        if (std::isnan(_f)) {
            throw std::invalid_argument(log("std::isnan(_f)"));
        }
        if (_f == 0) {
            throw std::invalid_argument(log("_f == 0"));
        }
    } catch (const std::invalid_argument &e) {
        std::cerr << e.what() << std::endl;
        return *this;
    }
    _T inv = (_T)1 / _f;
    return vector2_t<_T>(x * inv, y * inv);
}

template <class _T>
template <class _U>
vector2_t<_T> &vector2_t<_T>::operator/=(const _U &_f) {
    try {
        if (std::isnan(_f)) {
            throw std::invalid_argument(log("std::isnan(_f)"));
        }
        if (_f == 0) {
            throw std::invalid_argument(log("_f == 0"));
        }
    } catch (const std::invalid_argument &e) {
        std::cerr << e.what() << std::endl;
        return *this;
    }
    _T inv = (_T)1 / _f;
    x *= inv;
    y *= inv;
    return *this;
}

template <class _T>
const _T vector2_t<_T>::operator[](const uint32_t _idx) const {
    try {
        if (_idx > 1) {
            throw std::invalid_argument(log("_idx > 1"));
        }
    } catch (const std::invalid_argument &e) {
        std::cerr << e.what() << std::endl;
        return 0;
    }
    if (_idx == 0) {
        return x;
    }
    return y;
}

template <class _T>
_T vector2_t<_T>::operator[](const uint32_t _idx) {
    try {
        if (_idx > 1) {
            throw std::invalid_argument(log("_idx > 1"));
        }
    } catch (const std::invalid_argument &e) {
        std::cerr << e.what() << std::endl;
        return 0;
    }
    if (_idx == 0) {
        return x;
    }
    return y;
}

template <class _T>
vector2_t<_T>::operator vector2_t<int32_t>(void) const {
    try {
        if (HasNaNs()) {
            throw std::invalid_argument(log("HasNaNs()"));
        }
    } catch (const std::invalid_argument &e) {
        std::cerr << e.what() << std::endl;
        return vector2_t<int32_t>();
    }
    return vector2_t<int32_t>((int32_t)x, (int32_t)y);
}

template <class _T>
vector2_t<_T>::operator vector2_t<float>(void) const {
    try {
        if (HasNaNs()) {
            throw std::invalid_argument(log("HasNaNs()"));
        }
    } catch (const std::invalid_argument &e) {
        std::cerr << e.what() << std::endl;
        return vector2_t<float>();
    }
    return vector2_t<float>((float)x, (float)y);
}

template <class _T>
bool vector2_t<_T>::HasNaNs(void) const {
    return std::isnan(x) || std::isnan(y);
}

template <class _T>
const _T vector2_t<_T>::length_squared(void) const {
    try {
        if (HasNaNs()) {
            throw std::invalid_argument(log("HasNaNs()"));
        }
    } catch (const std::invalid_argument &e) {
        std::cerr << e.what() << std::endl;
        return 0;
    }
    return x * x + y * y;
}

template <class _T>
const _T vector2_t<_T>::length(void) const {
    try {
        if (HasNaNs()) {
            throw std::invalid_argument(log("HasNaNs()"));
        }
        if (x == 0 && y == 0) {
            throw std::runtime_error(log("x == 0 && y == 0"));
        }
    } catch (const std::invalid_argument &e) {
        std::cerr << e.what() << std::endl;
        return 0;
    } catch (const std::runtime_error &e) {
        std::cerr << e.what() << std::endl;
        return 0;
    }
    return std::sqrt(length_squared());
}

template <class _T>
const vector2_t<_T> vector2_t<_T>::abs(void) const {
    try {
        if (HasNaNs()) {
            throw std::invalid_argument(log("HasNaNs()"));
        }
    } catch (const std::invalid_argument &e) {
        std::cerr << e.what() << std::endl;
        return *this;
    }
    return vector2_t<_T>(std::abs(x), std::abs(y));
}

template <class _T>
const vector2_t<_T> vector2_t<_T>::normalize(void) const {
    try {
        if (HasNaNs()) {
            throw std::invalid_argument(log("HasNaNs()"));
        }
        if (length() == 0) {
            throw std::runtime_error(log("length() == 0"));
        }
    } catch (const std::invalid_argument &e) {
        std::cerr << e.what() << std::endl;
        return *this;
    } catch (const std::runtime_error &e) {
        std::cerr << e.what() << std::endl;
        return *this;
    }
    return *this / length();
}

template <class _T>
const vector2_t<_T> vector2_t<_T>::min(const vector2_t<_T> &_v) const {
    try {
        if (HasNaNs()) {
            throw std::invalid_argument(log("HasNaNs()"));
        }
    } catch (const std::invalid_argument &e) {
        std::cerr << e.what() << std::endl;
        return *this;
    }
    auto min_x = std::min(x, _v.x);
    auto min_y = std::min(y, _v.y);
    return vector2_t<_T>(min_x, min_y);
}

template <class _T>
const vector2_t<_T> vector2_t<_T>::max(const vector2_t<_T> &_v) const {
    try {
        if (HasNaNs()) {
            throw std::invalid_argument(log("HasNaNs()"));
        }
    } catch (const std::invalid_argument &e) {
        std::cerr << e.what() << std::endl;
        return *this;
    }
    auto max_x = std::max(x, _v.x);
    auto max_y = std::max(y, _v.y);
    return vector2_t<_T>(max_x, max_y);
}

/**
 * @brief 3 维向量
 * @tparam _T                类型
 */
template <class _T>
class vector3_t {
public:
    _T x;
    _T y;
    _T z;

    /**
     * @brief 构造函数
     */
    vector3_t(void);

    /**
     * @brief 构造函数
     * @param  _x              x 值
     * @param  _y              y 值
     * @param  _z              z 值
     */
    vector3_t(const _T &_x, const _T &_y, const _T &_z);

    /**
     * @brief 构造函数
     * @param  _v               另一个 vector3_t<_T>
     */
    vector3_t(const vector3_t<_T> &_v);

    /**
     * @brief = 重载
     * @param  _v               另一个 vector3_t<_T>
     * @return vector3_t<_T>&   结果
     */
    vector3_t<_T> &operator=(const vector3_t<_T> &_v);

    /**
     * @brief == 重载
     * @param  _v              另一个 vector3_t<_T>
     * @return true            相等
     * @return false           不相等
     */
    bool operator==(const vector3_t<_T> &_v) const;

    /**
     * @brief != 重载
     * @param  _v              另一个 vector3_t<_T>
     * @return true            不相等
     * @return false           相等
     */
    bool operator!=(const vector3_t<_T> &_v) const;

    /**
     * @brief + 重载，向量加
     * @param  _v               另一个 vector3_t<_T>
     * @return const vector3_t<_T>  结果
     */
    const vector3_t<_T> operator+(const vector3_t<_T> &_v) const;

    /**
     * @brief += 重载，向量加
     * @param  _v               另一个 vector3_t<_T>
     * @return vector3_t<_T>&     结果
     */
    vector3_t<_T> &operator+=(const vector3_t<_T> &_v);

    /**
     * @brief - 重载，向量反向
     * @return const vector3_t<_T>  结果
     */
    const vector3_t<_T> operator-(void) const;

    /**
     * @brief - 重载，向量减
     * @param  _v               另一个 vector3_t<_T>
     * @return const vector3_t<_T>  结果
     */
    const vector3_t<_T> operator-(const vector3_t<_T> &_v) const;

    /**
     * @brief -= 重载，向量减
     * @param  _v               另一个 vector3_t<_T>
     * @return vector3_t<_T>      结果
     */
    vector3_t<_T> &operator-=(const vector3_t<_T> &_v);

    /**
     * @brief * 重载，向量数乘
     * @tparam _U              数的类型
     * @param  _s              数
     * @return const vector3_t<_T>  结果
     */
    template <class _U>
    const vector3_t<_T> operator*(const _U _s) const;

    /**
     * @brief * 重载，向量点积
     * @param  _v               要乘的向量
     * @return const _T         结果
     */
    const _T operator*(const vector3_t<_T> &_v) const;

    /**
     * @brief *= 重载，向量数乘
     * @tparam _U              数的类型
     * @param  _s              数
     * @return vector3_t<_T>&    结果
     */
    template <class _U>
    vector3_t<_T> &operator*=(const _U _s);

    /**
     * @brief ^ 重载，向量叉积
     * @param  _v              要乘的向量
     * @return const vector3_t<_T>  结果
     */
    const vector3_t<_T> operator^(const vector3_t<_T> &_v) const;

    /**
     * @brief / 重载，向量数除
     * @tparam _U              数的类型
     * @param  _f              数
     * @return vector3_t<_T>     结果
     */
    template <class _U>
    const vector3_t<_T> operator/(const _U _f) const;

    /**
     * @brief /= 重载，向量数除
     * @tparam _U              数的类型
     * @param  _f              数
     * @return vector3_t<_T>&    结果
     */
    template <class _U>
    vector3_t<_T> &operator/=(const _U _f);

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
     * @brief 转换为 int32_t 类型
     */
    operator vector3_t<int32_t>(void) const;

    /**
     * @brief 转换为 float 类型
     */
    operator vector3_t<float>(void) const;

    /**
     * @brief 转换为 vector2_t<int32_t> 类型
     */
    operator vector2_t<int32_t>(void) const;

    /**
     * @brief 转换为 vector2_t<float> 类型
     */
    operator vector2_t<float>(void) const;

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
     * @return const vector3_t<_T>  结果
     */
    const vector3_t<_T> abs(const vector3_t<_T> &_v);

    /**
     * @brief 归一化
     * @param  _v              向量
     * @return const vector3_t<_T>  结果
     */
    const vector3_t<_T> normalize(void) const;

    /**
     * @brief 构造最小向量
     * @param  _v             向量
     * @return const vector3_t<_T>  结果
     */
    const vector3_t<_T> min(const vector3_t<_T> &_v) const;

    /**
     * @brief 构造最大向量
     * @param  _v             向量
     * @return const vector3_t<_T>  结果
     */
    const vector3_t<_T> max(const vector3_t<_T> &_v) const;

    friend std::ostream &operator<<(std::ostream        &_os,
                                    const vector3_t<_T> &_v) {
        _os << "[ " << _v.x << ", " << _v.y << ", " << _v.z << " ]";
        return _os;
    }
};

template <class _T>
vector3_t<_T>::vector3_t(void) {
    x = 0;
    y = 0;
    z = 0;
    return;
}

template <class _T>
vector3_t<_T>::vector3_t(const _T &_x, const _T &_y, const _T &_z)
    : x(_x), y(_y), z(_z) {
    try {
        if (HasNaNs()) {
            throw std::invalid_argument(log("HasNaNs()"));
        }
    } catch (const std::invalid_argument &e) {
        std::cerr << e.what() << std::endl;
        *this = vector3_t<_T>();
        return;
    }
    return;
}

template <class _T>
vector3_t<_T>::vector3_t(const vector3_t<_T> &_v) {
    try {
        if (_v.HasNaNs()) {
            throw std::invalid_argument(log("_v.HasNaNs()"));
        }
    } catch (const std::invalid_argument &e) {
        std::cerr << e.what() << std::endl;
        *this = vector3_t<_T>();
        return;
    }
    x = _v.x;
    y = _v.y;
    z = _v.z;
    return;
}

template <class _T>
vector3_t<_T> &vector3_t<_T>::operator=(const vector3_t<_T> &_v) {
    try {
        if (_v.HasNaNs()) {
            throw std::invalid_argument(log("_v.HasNaNs()"));
        }
    } catch (const std::invalid_argument &e) {
        std::cerr << e.what() << std::endl;
        return *this;
    }
    x = _v.x;
    y = _v.y;
    z = _v.z;
    return *this;
}

template <class _T>
bool vector3_t<_T>::operator==(const vector3_t<_T> &_v) const {
    try {
        if (_v.HasNaNs()) {
            throw std::invalid_argument(log("_v.HasNaNs()"));
        }
    } catch (const std::invalid_argument &e) {
        std::cerr << e.what() << std::endl;
        return false;
    }
    return x == _v.x && y == _v.y && z == _v.z;
}

template <class _T>
bool vector3_t<_T>::operator!=(const vector3_t<_T> &_v) const {
    try {
        if (_v.HasNaNs()) {
            throw std::invalid_argument(log("_v.HasNaNs()"));
        }
    } catch (const std::invalid_argument &e) {
        std::cerr << e.what() << std::endl;
        return false;
    }
    return x != _v.x || y != _v.y || z != _v.z;
}

template <class _T>
const vector3_t<_T> vector3_t<_T>::operator+(const vector3_t<_T> &_v) const {
    try {
        if (_v.HasNaNs()) {
            throw std::invalid_argument(log("_v.HasNaNs()"));
        }
    } catch (const std::invalid_argument &e) {
        std::cerr << e.what() << std::endl;
        return *this;
    }
    return vector3_t(x + _v.x, y + _v.y, z + _v.z);
}

template <class _T>
vector3_t<_T> &vector3_t<_T>::operator+=(const vector3_t<_T> &_v) {
    try {
        if (_v.HasNaNs()) {
            throw std::invalid_argument(log("_v.HasNaNs()"));
        }
    } catch (const std::invalid_argument &e) {
        std::cerr << e.what() << std::endl;
        return *this;
    }
    x += _v.x;
    y += _v.y;
    z += _v.z;
    return *this;
}

template <class _T>
const vector3_t<_T> vector3_t<_T>::operator-(void) const {
    try {
        if (HasNaNs()) {
            throw std::invalid_argument(log("HasNaNs()"));
        }
    } catch (const std::invalid_argument &e) {
        std::cerr << e.what() << std::endl;
        return *this;
    }
    return vector3_t<_T>(-x, -y, -z);
}

template <class _T>
const vector3_t<_T> vector3_t<_T>::operator-(const vector3_t<_T> &_v) const {
    try {
        if (_v.HasNaNs()) {
            throw std::invalid_argument(log("_v.HasNaNs()"));
        }
    } catch (const std::invalid_argument &e) {
        std::cerr << e.what() << std::endl;
        return *this;
    }
    return vector3_t(x - _v.x, y - _v.y, z - _v.z);
}

template <class _T>
vector3_t<_T> &vector3_t<_T>::operator-=(const vector3_t<_T> &_v) {
    try {
        if (_v.HasNaNs()) {
            throw std::invalid_argument(log("_v.HasNaNs()"));
        }
    } catch (const std::invalid_argument &e) {
        std::cerr << e.what() << std::endl;
        return *this;
    }
    x -= _v.x;
    y -= _v.y;
    z -= _v.z;
    return *this;
}

template <class _T>
template <class _U>
const vector3_t<_T> vector3_t<_T>::operator*(const _U _s) const {
    try {
        if (std::isnan(_s)) {
            throw std::invalid_argument(log("std::isnan(_s)"));
        }
    } catch (const std::invalid_argument &e) {
        std::cerr << e.what() << std::endl;
        return *this;
    }
    return vector3_t<_T>(_s * x, _s * y, _s * z);
}

template <class _T>
const _T vector3_t<_T>::operator*(const vector3_t<_T> &_v) const {
    try {
        if (_v.HasNaNs()) {
            throw std::invalid_argument(log("_v.HasNaNs()"));
        }
    } catch (const std::invalid_argument &e) {
        std::cerr << e.what() << std::endl;
        return 0;
    }
    return x * _v.x + y * _v.y + z * _v.z;
}

template <class _T>
template <class _U>
vector3_t<_T> &vector3_t<_T>::operator*=(const _U _s) {
    try {
        if (std::isnan(_s)) {
            throw std::invalid_argument(log("std::isnan(_s)"));
        }
    } catch (const std::invalid_argument &e) {
        std::cerr << e.what() << std::endl;
        return *this;
    }
    x *= _s;
    y *= _s;
    z *= _s;
    return *this;
}

template <class _T>
const vector3_t<_T> vector3_t<_T>::operator^(const vector3_t<_T> &_v) const {
    try {
        if (_v.HasNaNs()) {
            throw std::invalid_argument(log("_v.HasNaNs()"));
        }
    } catch (const std::invalid_argument &e) {
        std::cerr << e.what() << std::endl;
        return *this;
    }
    return vector3_t<_T>((y * _v.z) - (z * _v.y), (z * _v.x) - (x * _v.z),
                         (x * _v.y) - (y * _v.x));
}

template <class _T>
template <class _U>
const vector3_t<_T> vector3_t<_T>::operator/(const _U _f) const {
    try {
        if (_f == 0) {
            throw std::invalid_argument(log("_f == 0"));
        }
    } catch (const std::invalid_argument &e) {
        std::cerr << e.what() << std::endl;
        return *this;
    }
    _T inv = (_T)1 / _f;
    return vector3_t<_T>(x * inv, y * inv, z * inv);
}

template <class _T>
template <class _U>
vector3_t<_T> &vector3_t<_T>::operator/=(const _U _f) {
    try {
        if (_f == 0) {
            throw std::invalid_argument(log("_f == 0"));
        }
    } catch (const std::invalid_argument &e) {
        std::cerr << e.what() << std::endl;
        return *this;
    }
    _T inv = (_T)1 / _f;
    x *= inv;
    y *= inv;
    z *= inv;
    return *this;
}

template <class _T>
const _T vector3_t<_T>::operator[](const uint32_t _idx) const {
    try {
        if (_idx > 2) {
            throw std::invalid_argument(log("_idx > 2"));
        }
    } catch (const std::invalid_argument &e) {
        std::cerr << e.what() << std::endl;
        return 0;
    }
    if (_idx == 0) {
        return x;
    }
    if (_idx == 1) {
        return y;
    }
    return z;
}

template <class _T>
_T vector3_t<_T>::operator[](const uint32_t _idx) {
    try {
        if (_idx > 2) {
            throw std::invalid_argument(log("_idx > 2"));
        }
    } catch (const std::invalid_argument &e) {
        std::cerr << e.what() << std::endl;
        return 0;
    }
    if (_idx == 0) {
        return x;
    }
    if (_idx == 1) {
        return y;
    }
    return z;
}

template <class _T>
vector3_t<_T>::operator vector3_t<int32_t>(void) const {
    try {
        if (HasNaNs()) {
            throw std::invalid_argument(log("HasNaNs()"));
        }
    } catch (const std::invalid_argument &e) {
        std::cerr << e.what() << std::endl;
        return vector3_t<int32_t>();
    }
    return vector3_t<int32_t>((int32_t)x, (int32_t)y, (int32_t)z);
}

template <class _T>
vector3_t<_T>::operator vector3_t<float>(void) const {
    try {
        if (HasNaNs()) {
            throw std::invalid_argument(log("HasNaNs()"));
        }
    } catch (const std::invalid_argument &e) {
        std::cerr << e.what() << std::endl;
        return vector3_t<float>();
    }
    return vector3_t<float>((float)x, (float)y, (float)z);
}

template <class _T>
vector3_t<_T>::operator vector2_t<int32_t>(void) const {
    try {
        if (HasNaNs()) {
            throw std::invalid_argument(log("HasNaNs()"));
        }
    } catch (const std::invalid_argument &e) {
        std::cerr << e.what() << std::endl;
        return vector2_t<int32_t>();
    }
    return vector2_t<int32_t>((int32_t)x, (int32_t)y);
}

template <class _T>
vector3_t<_T>::operator vector2_t<float>(void) const {
    try {
        if (HasNaNs()) {
            throw std::invalid_argument(log("HasNaNs()"));
        }
    } catch (const std::invalid_argument &e) {
        std::cerr << e.what() << std::endl;
        return vector2_t<float>();
    }
    return vector2_t<float>((float)x, (float)y);
}

template <class _T>
bool vector3_t<_T>::HasNaNs(void) const {
    return std::isnan(x) || std::isnan(y) || std::isnan(z);
}

template <class _T>
const _T vector3_t<_T>::length_squared(void) const {
    try {
        if (HasNaNs()) {
            throw std::invalid_argument(log("HasNaNs()"));
        }
    } catch (const std::invalid_argument &e) {
        std::cerr << e.what() << std::endl;
        return 0;
    }
    return x * x + y * y + z * z;
}

template <class _T>
const _T vector3_t<_T>::length(void) const {
    try {
        if (HasNaNs()) {
            throw std::invalid_argument(log("HasNaNs()"));
        }
        if (x == 0 && y == 0 && z == 0) {
            throw std::runtime_error(log("x == 0 && y == 0 && z == 0"));
        }
    } catch (const std::invalid_argument &e) {
        std::cerr << e.what() << std::endl;
        return 0;
    } catch (const std::runtime_error &e) {
        std::cerr << e.what() << std::endl;
        return 0;
    }
    return std::sqrt(length_squared());
}

template <class _T>
const vector3_t<_T> vector3_t<_T>::abs(const vector3_t<_T> &_v) {
    try {
        if (HasNaNs()) {
            throw std::invalid_argument(log("HasNaNs()"));
        }
    } catch (const std::invalid_argument &e) {
        std::cerr << e.what() << std::endl;
        return *this;
    }
    return vector3_t<_T>(std::abs(_v.x), std::abs(_v.y), std::abs(_v.z));
}

template <class _T>
const vector3_t<_T> vector3_t<_T>::normalize(void) const {
    try {
        if (HasNaNs()) {
            throw std::invalid_argument(log("HasNaNs()"));
        }
        if (length() == 0) {
            throw std::runtime_error(log("length() == 0"));
        }
    } catch (const std::invalid_argument &e) {
        std::cerr << e.what() << std::endl;
        return *this;
    } catch (const std::runtime_error &e) {
        std::cerr << e.what() << std::endl;
        return *this;
    }
    return *this / length();
}

template <class _T>
const vector3_t<_T> vector3_t<_T>::min(const vector3_t<_T> &_v) const {
    try {
        if (HasNaNs()) {
            throw std::invalid_argument(log("HasNaNs()"));
        }
    } catch (const std::invalid_argument &e) {
        std::cerr << e.what() << std::endl;
        return *this;
    }
    return vector3_t<_T>(std::min(x, _v.x), std::min(y, _v.y),
                         std::min(z, _v.z));
}

template <class _T>
const vector3_t<_T> vector3_t<_T>::max(const vector3_t<_T> &_v) const {
    try {
        if (HasNaNs()) {
            throw std::invalid_argument(log("HasNaNs()"));
        }
    } catch (const std::invalid_argument &e) {
        std::cerr << e.what() << std::endl;
        return *this;
    }
    return vector3_t<_T>(std::max(x, _v.x), std::max(y, _v.y),
                         std::max(z, _v.z));
}

typedef vector2_t<float>   vector2f_t;
typedef vector2_t<int32_t> vector2i_t;
typedef vector3_t<float>   vector3f_t;
typedef vector3_t<int32_t> vector3i_t;

#endif /* _VECTOR_HPP_ */
