
/**
 * @file main.cpp
 * @brief 入口
 * @author Zone.N (Zone.Niuzh@hotmail.com)
 * @version 1.0
 * @date 2022-06-07
 * @copyright MIT LICENSE
 * https://github.com/Simple-XX/SimpleRenderer
 * @par change log:
 * <table>
 * <tr><th>Date<th>Author<th>Description
 * <tr><td>2022-06-07<td>Zone.N<td>迁移到 doxygen
 * </table>
 */

#ifndef __VECTOR_HPP__
#define __VECTOR_HPP__

#include "assert.h"
#include "iostream"
#include "math.h"
#include "stdarg.h"
#include "vector"

template <class T, size_t N = 3>
class Vector {
private:
public:
    union {
        T vect[N];
        struct {
            T x;
            T y;
            T z;
        } coord;
    };

    Vector(void);
    Vector(const T *const _vect);
    Vector(const Vector<T, N> &_vector);
    Vector(const std::vector<T> &_vect);
    Vector(const T _x, const T _y);
    Vector(const T _x, const T _y, const T _z);
    ~Vector(void);
    // 向量不为零向量
    operator bool(void) const;
    // 向量比较
    bool operator==(const Vector<T, N> &_v) const;
    bool operator!=(const Vector<T, N> &_v) const;
    bool operator>(const Vector<T, N> &_v) const;
    bool operator>=(const Vector<T, N> &_v) const;
    bool operator<(const Vector<T, N> &_v) const;
    bool operator<=(const Vector<T, N> &_v) const;
    // 向量取反
    Vector<T, N> operator-() const;
    // 范数
    float norm(void) const;
    // 单位向量
    Vector<float, N> unit(void) const;
    // 赋值
    Vector<T, N> &operator=(const Vector<T, N> &_v);
    // 向量和
    Vector<T, N> operator+(const Vector<T, N> &_v) const;
    // 向量自加
    Vector<T, N> &operator+=(const Vector<T, N> &_v);
    // 向量差
    Vector<T, N> operator-(const Vector<T, N> &_v) const;
    // 向量自减
    Vector<T, N> &operator-=(const Vector<T, N> &_v);
    // 向量数乘
    Vector<T, N> operator*(const T _t) const;
    // 向量自乘
    Vector<T, N> &operator*=(const T _t);
    // 向量点积
    T operator*(const Vector<T, N> &_v) const;
    // 向量叉积
    Vector<T, N> operator^(const Vector<T, N> &_v) const;
    // 向量投影
    // 下标访问
    T &operator[](size_t _idx);
};

template <class T, size_t N>
Vector<T, N>::Vector(void) {
    coord.x = 0;
    coord.y = 0;
    coord.z = 0;
    return;
}

template <class T, size_t N>
Vector<T, N>::Vector(const T *const _vect) {
    coord.x = _vect[0];
    coord.y = _vect[1];
    if (N == 3) {
        coord.z = _vect[2];
    }
    return;
}

template <class T, size_t N>
Vector<T, N>::Vector(const Vector<T, N> &_vector) : coord(_vector.coord) {
    return;
}

template <class T, size_t N>
Vector<T, N>::Vector(const std::vector<T> &_vect) {
    coord.x = _vect.at(0);
    coord.y = _vect.at(1);
    if (N == 3) {
        coord.z = _vect.at(2);
    }
    return;
}

template <class T, size_t N>
Vector<T, N>::Vector(const T _x, const T _y) {
    assert(N == 2);
    coord.x = _x;
    coord.y = _y;
    return;
}

template <class T, size_t N>
Vector<T, N>::Vector(const T _x, const T _y, const T _z) {
    assert(N == 3);
    coord.x = _x;
    coord.y = _y;
    coord.z = _z;
    return;
}

template <class T, size_t N>
Vector<T, N>::~Vector(void) {
    return;
}

template <class T, size_t N>
Vector<T, N>::operator bool(void) const {
    bool res = false;
    if (coord.x != 0 || coord.y != 0 || coord.z != 0) {
        res = true;
    }
    return res;
}

template <class T, size_t N>
bool Vector<T, N>::operator==(const Vector<T, N> &_v) const {
    bool res = true;
    if (coord.x != _v.coord.x || coord.y != _v.coord.y ||
        coord.z != _v.coord.z) {
        res = false;
    }
    return res;
}

template <class T, size_t N>
bool Vector<T, N>::operator!=(const Vector<T, N> &_v) const {
    bool res = false;
    if (coord.x != _v.coord.x || coord.y != _v.coord.y ||
        coord.z != _v.coord.z) {
        res = true;
    }
    return res;
}

template <class T, size_t N>
bool Vector<T, N>::operator>(const Vector<T, N> &_v) const {
    bool res = true;
    if (coord.x <= _v.coord.x || coord.y <= _v.coord.y ||
        coord.z <= _v.coord.z) {
        res = false;
    }
    return res;
}

template <class T, size_t N>
bool Vector<T, N>::operator>=(const Vector<T, N> &_v) const {
    bool res = true;
    if (coord.x < _v.coord.x || coord.y < _v.coord.y || coord.z < _v.coord.z) {
        res = false;
    }
    return res;
}

template <class T, size_t N>
bool Vector<T, N>::operator<(const Vector<T, N> &_v) const {
    bool res = true;
    if (coord.x >= _v.coord.x || coord.y >= _v.coord.y ||
        coord.z >= _v.coord.z) {
        res = false;
    }
    return res;
}

template <class T, size_t N>
bool Vector<T, N>::operator<=(const Vector<T, N> &_v) const {
    bool res = true;
    if (coord.x > _v.coord.x || coord.y > _v.coord.y || coord.z > _v.coord.z) {
        res = false;
    }
    return res;
}

template <class T, size_t N>
Vector<T, N> Vector<T, N>::operator-() const {
    T tmp[N];
    tmp[0] = -coord.x;
    tmp[1] = -coord.y;
    if (N == 3) {
        tmp[2] = -coord.z;
    }
    return Vector<T, N>(tmp);
}

template <class T, size_t N>
float Vector<T, N>::norm(void) const {
    float res = 0;
    res += coord.x * coord.x;
    res += coord.y * coord.y;
    res += coord.z * coord.z;
    res = sqrt(res);
    return res;
}

template <class T, size_t N>
Vector<float, N> Vector<T, N>::unit(void) const {
    float tmp[N];
    float norm = this->norm();
    tmp[0]     = coord.x / norm;
    tmp[1]     = coord.y / norm;
    if (N == 3) {
        tmp[2] = coord.z / norm;
    }
    return Vector<float, N>(tmp);
}

template <class T, size_t N>
Vector<T, N> &Vector<T, N>::operator=(const Vector<T, N> &_v) {
    coord = _v.coord;
    return *this;
}

template <class T, size_t N>
Vector<T, N> Vector<T, N>::operator+(const Vector<T, N> &_v) const {
    T tmp[N];
    tmp[0] = coord.x + _v.coord.x;
    tmp[1] = coord.y + _v.coord.y;
    if (N == 3) {
        tmp[2] = coord.z + _v.coord.z;
    }
    return Vector<T, N>(tmp);
}

template <class T, size_t N>
Vector<T, N> &Vector<T, N>::operator+=(const Vector<T, N> &_v) {
    coord.x += _v.coord.x;
    coord.y += _v.coord.y;
    if (N == 3) {
        coord.z += _v.coord.z;
    }
    return *this;
}

template <class T, size_t N>
Vector<T, N> Vector<T, N>::operator-(const Vector<T, N> &_v) const {
    T tmp[N];
    tmp[0] = coord.x - _v.coord.x;
    tmp[1] = coord.y - _v.coord.y;
    if (N == 3) {
        tmp[2] = coord.z - _v.coord.z;
    }
    return Vector<T, N>(tmp);
}

template <class T, size_t N>
Vector<T, N> &Vector<T, N>::operator-=(const Vector<T, N> &_v) {
    coord.x -= _v.coord.x;
    coord.y -= _v.coord.y;
    if (N == 3) {
        coord.z -= _v.coord.z;
    }
    return *this;
}

template <class T, size_t N>
Vector<T, N> Vector<T, N>::operator*(const T _t) const {
    T tmp[N];
    tmp[0] = coord.x * _t;
    tmp[1] = coord.y * _t;
    if (N == 3) {
        tmp[2] = coord.z * _t;
    }
    return Vector<T, N>(tmp);
}

template <class T, size_t N>
Vector<T, N> &Vector<T, N>::operator*=(const T _t) {
    coord.x *= _t;
    coord.y *= _t;
    if (N == 3) {
        coord.z *= _t;
    }
    return *this;
}

template <class T, size_t N>
T Vector<T, N>::operator*(const Vector<T, N> &_v) const {
    T res = 0;
    res += coord.x * _v.coord.x;
    res += coord.y * _v.coord.y;
    if (N == 3) {
        res += coord.z * _v.coord.z;
    }
    return res;
}

template <class T, size_t N>
Vector<T, N> Vector<T, N>::operator^(const Vector<T, N> &_v) const {
    assert(N == 3);
    T tmp[N];
    tmp[0] = coord.y * _v.coord.z - coord.z * _v.coord.y;
    tmp[1] = coord.z * _v.coord.x - coord.x * _v.coord.z;
    tmp[2] = coord.x * _v.coord.y - coord.y * _v.coord.x;
    return Vector<T, N>(tmp);
}

template <class T, size_t N>
T &Vector<T, N>::operator[](size_t _idx) {
    return vect[_idx];
}

// 输出
template <class T, size_t N>
std::ostream &operator<<(std::ostream &_os, const Vector<T, N> &_v) {
    _os << "(" << _v.coord.x << ", " << _v.coord.y;
    if (N == 3) {
        _os << ", " << _v.coord.z;
    }
    _os << ")";
    return _os;
}

typedef Vector<size_t, 2> Vectors2;
typedef Vector<float, 2>  Vectorf2;
typedef Vector<float, 3>  Vectorf3;

#include "cmath"
#include "glog/logging.h"

template <class _T>
class Vector2;
template <class _T>
class Vector3;
template <class _T>
class Point3;
template <class _T>
class Point2;
template <class _T>
class Normal3;

/**
 * @brief 是否为非数值
 * @param  _T                类型
 * @param  _x                值
 * @return true              是
 * @return false             否
 */
template <class _T>
inline bool isNaN(const _T _x) {
    return std::isnan(_x);
}

/**
 * @brief 是否为非数值
 * @param  _x                值
 * @return true              是
 * @return false             否
 */
template <>
inline bool isNaN(const int _x) {
    return false;
}

/**
 * @brief 2 维向量
 * @param  _T                类型
 */
template <class _T>
class Vector2 {
public:
    _T x;
    _T y;

    /**
     * @brief 构造函数
     */
    Vector2(void);

    /**
     * @brief 构造函数
     * @param  _x              x 值
     * @param  _y              y 值
     */
    Vector2(_T _x, _T _y);

    /**
     * @brief 构造函数
     * @param  _p              二维点
     */
    Vector2(const Point2<_T> &_p);

    /**
     * @brief 构造函数
     * @param  _p              三维点
     */
    Vector2(const Point3<_T> &_p);

    /**
     * @brief 构造函数
     * @param  _v              另一个 Vector2<_T>
     */
    Vector2(const Vector2<_T> &_v);

    /**
     * @brief = 重载
     * @param  _v               另一个 Vector2<_T>
     * @return Vector2<_T>&     结果
     */
    Vector2<_T> &operator=(const Vector2<_T> &_v);

    /**
     * @brief == 重载
     * @param  _v              另一个 Vector2<_T>
     * @return true            相等
     * @return false           不相等
     */
    bool operator==(const Vector2<_T> &_v) const;

    /**
     * @brief != 重载
     * @param  _v              另一个 Vector2<_T>
     * @return true            不相等
     * @return false           相等
     */
    bool operator!=(const Vector2<_T> &_v) const;

    /**
     * @brief + 重载
     * @param  _v              另一个 Vector2<_T>
     * @return Vector2<_T>     结果
     */
    Vector2<_T> operator+(const Vector2<_T> &_v) const;

    /**
     * @brief += 重载
     * @param  _v              另一个 Vector2<_T>
     * @return Vector2<_T>     结果
     */
    Vector2<_T> &operator+=(const Vector2<_T> &_v);

    /**
     * @brief - 重载，向量反向
     * @return Vector2<_T>     结果
     */
    Vector2<_T> operator-() const;

    /**
     * @brief - 重载
     * @param  _v              另一个 Vector2<_T>
     * @return Vector2<_T>     结果
     */
    Vector2<_T> operator-(const Vector2<_T> &_v) const;

    /**
     * @brief -= 重载
     * @param  _v              另一个 Vector2<_T>
     * @return Vector2<_T>     结果
     */
    Vector2<_T> &operator-=(const Vector2<_T> &_v);

    /**
     * @brief * 重载，向量数乘
     * @param  _U              相乘的数的类型
     * @param  _f              相乘的数
     * @return Vector2<_T>     结果
     */
    template <class _U>
    Vector2<_T> operator*(_U _f) const;

    /**
     * @brief * 重载，向量点积
     * @param  _v              要乘的向量
     * @return _T              结果
     */
    _T operator*(Vector2<_T> _v) const;

    /**
     * @brief *= 重载，向量数乘
     * @param  _U              相乘的数的类型
     * @param  _f              相乘的数
     * @return Vector2<_T>     结果
     */
    template <class _U>
    Vector2<_T> &operator*=(_U _f);

    /**
     * @brief / 重载，向量数除
     * @param  _U              相除的数的类型
     * @param  _f              相除的数
     * @return Vector2<_T>     结果
     */
    template <class _U>
    Vector2<_T> operator/(_U _f) const;

    /**
     * @brief /= 重载，向量数除
     * @param  _U              相除的数的类型
     * @param  _f              相除的数
     * @return Vector2<_T>     结果
     */
    template <class _U>
    Vector2<_T> &operator/=(_U _f);

    /**
     * @brief [] 重载
     * @return _T              结果
     */
    _T operator[](int _idx) const;

    /**
     * @brief [] 重载
     * @return _T&             结果
     */
    _T &operator[](int _idx);

    /**
     * @brief 是否有非数值
     * @return true             有
     * @return false            无
     */
    bool HasNaNs(void) const;

    /**
     * @brief 距离^2
     * @return _T              距离的平方
     */
    _T LengthSquared(void) const;

    /**
     * @brief 距离
     * @return _T              距离
     */
    _T Length(void) const;

    /**
     * @brief 对所有分量取绝对值
     * @param  _v              向量
     * @return Vector2<_T>     结果
     */
    Vector2<_T> Abs(const Vector2<_T> &_v);

    /**
     * @brief 点积
     * @param  _v1             向量1
     * @param  _v2             向量2
     * @return _T              结果
     */
    _T Dot(const Vector2<_T> &_v1, const Vector2<_T> &_v2);

    /**
     * @brief 点积的绝对值
     * @param  _v1             向量1
     * @param  _v2             向量2
     * @return _T              结果
     */
    _T AbsDot(const Vector2<_T> &_v1, const Vector2<_T> &_v2);

    /**
     * @brief 归一化
     * @param  _v              向量
     * @return Vector2<_T>     结果
     */
    Vector2<_T> Normalize(const Vector2<_T> &_v);

    friend std::ostream &operator<<(std::ostream &_os, const Vector2<_T> &_v);
};

template <class _T>
Vector2<_T>::Vector2(void) {
    x = 0;
    y = 0;
    return;
}

template <class _T>
Vector2<_T>::Vector2(_T _x, _T _y) : x(_x), y(_y) {
    DCHECK(!HasNaNs());
    return;
}

template <class _T>
Vector2<_T>::Vector2(const Point2<_T> &_p) : x(_p.x), y(_p.y) {
    DCHECK(!HasNaNs());
    return;
}

template <class _T>
Vector2<_T>::Vector2(const Point3<_T> &_p) : x(_p.x), y(_p.y) {
    DCHECK(!HasNaNs());
    return;
}

template <class _T>
Vector2<_T>::Vector2(const Vector2<_T> &_v) {
    DCHECK(!_v.HasNaNs());
    x = _v.x;
    y = _v.y;
    return;
}

template <class _T>
Vector2<_T> &Vector2<_T>::operator=(const Vector2<_T> &_v) {
    DCHECK(!_v.HasNaNs());
    x = _v.x;
    y = _v.y;
    return *this;
}

template <class _T>
bool Vector2<_T>::operator==(const Vector2<_T> &_v) const {
    return x == _v.x && y == _v.y;
}

template <class _T>
bool Vector2<_T>::operator!=(const Vector2<_T> &_v) const {
    return x != _v.x || y != _v.y;
}

template <class _T>
Vector2<_T> Vector2<_T>::operator+(const Vector2<_T> &_v) const {
    DCHECK(!_v.HasNaNs());
    return Vector2(x + _v.x, y + _v.y);
}

template <class _T>
Vector2<_T> &Vector2<_T>::operator+=(const Vector2<_T> &_v) {
    DCHECK(!_v.HasNaNs());
    x += _v.x;
    y += _v.y;
    return *this;
}

template <class _T>
Vector2<_T> Vector2<_T>::operator-() const {
    return Vector2<_T>(-x, -y);
}

template <class _T>
Vector2<_T> Vector2<_T>::operator-(const Vector2<_T> &_v) const {
    DCHECK(!_v.HasNaNs());
    return Vector2(x - _v.x, y - _v.y);
}

template <class _T>
Vector2<_T> &Vector2<_T>::operator-=(const Vector2<_T> &_v) {
    DCHECK(!_v.HasNaNs());
    x -= _v.x;
    y -= _v.y;
    return *this;
}

template <class _T>
template <class _U>
Vector2<_T> Vector2<_T>::operator*(_U _f) const {
    return Vector2<_T>(_f * x, _f * y);
}

template <class _T>
_T Vector2<_T>::operator*(Vector2<_T> _v) const {
    DCHECK(!_v.HasNaNs());
    return x * _v.x + y * _v.y;
}

template <class _T>
template <class _U>
Vector2<_T> &Vector2<_T>::operator*=(_U _f) {
    DCHECK(!isNaN(_f));
    x *= _f;
    y *= _f;
    return *this;
}

template <class _T>
template <class _U>
Vector2<_T> Vector2<_T>::operator/(_U _f) const {
    CHECK_NE(_f, 0);
    _T inv = (_T)1 / _f;
    return Vector2<_T>(x * inv, y * inv);
}

template <class _T>
template <class _U>
Vector2<_T> &Vector2<_T>::operator/=(_U _f) {
    CHECK_NE(_f, 0);
    _T inv = (_T)1 / _f;
    x *= inv;
    y *= inv;
    return *this;
}

template <class _T>
_T Vector2<_T>::operator[](int _idx) const {
    DCHECK(_idx >= 0 && _idx <= 1);
    if (_idx == 0)
        return x;
    return y;
}

template <class _T>
_T &Vector2<_T>::operator[](int _idx) {
    DCHECK(_idx >= 0 && _idx <= 1);
    if (_idx == 0)
        return x;
    return y;
}

template <class _T>
bool Vector2<_T>::HasNaNs(void) const {
    return isNaN(x) || isNaN(y);
}

template <class _T>
_T Vector2<_T>::LengthSquared(void) const {
    return x * x + y * y;
}

template <class _T>
_T Vector2<_T>::Length(void) const {
    return std::sqrt(LengthSquared());
}

template <class _T>
Vector2<_T> Vector2<_T>::Abs(const Vector2<_T> &_v) {
    return Vector2<_T>(std::abs(_v.x), std::abs(_v.y));
}

template <class _T>
_T Vector2<_T>::Dot(const Vector2<_T> &_v1, const Vector2<_T> &_v2) {
    DCHECK(!_v1.HasNaNs() && !_v2.HasNaNs());
    return _v1.x * _v2.x + _v1.y * _v2.y;
}

template <class _T>
_T Vector2<_T>::AbsDot(const Vector2<_T> &_v1, const Vector2<_T> &_v2) {
    DCHECK(!_v1.HasNaNs() && !_v2.HasNaNs());
    return std::abs(Dot(_v1, _v2));
}

template <class _T>
Vector2<_T> Vector2<_T>::Normalize(const Vector2<_T> &_v) {
    return _v / _v.Length();
}

template <class _T>
std::ostream &operator<<(std::ostream &_os, const Vector2<_T> &_v) {
    _os << "[ " << _v.x << ", " << _v.y << " ]";
    return _os;
}

/**
 * @brief 3 维向量
 * @param  _T                类型
 */
template <class _T>
class Vector3 {
public:
    _T x;
    _T y;
    _T z;

    /**
     * @brief 构造函数
     */
    Vector3(void) {
        x = 0;
        y = 0;
        z = 0;
        return;
    }

    /**
     * @brief 构造函数
     * @param  _x              x 值
     * @param  _y              y 值
     * @param  _z              z 值
     */
    Vector3(_T _x, _T _y, _T _z) : x(_x), y(_y), z(_z) {
        DCHECK(!HasNaNs());
        return;
    }

    /**
     * @brief 构造函数
     * @param  _p              三维点
     */
    Vector3(const Point3<_T> &_p) : x(_p.x), y(_p.y), z(_p.z) {
        DCHECK(!HasNaNs());
        return;
    }

    /**
     * @brief 构造函数
     * @param  _v               另一个 Vector3<_T>
     */
    Vector3(const Vector3<_T> &_v) {
        DCHECK(!_v.HasNaNs());
        x = _v.x;
        y = _v.y;
        z = _v.z;
        return;
    }

    /**
     * @brief 构造函数
     * @param  _n               三维法向量
     */
    Vector3(const Normal3<_T> &_n) : x(_n.x), y(_n.y), z(_n.z) {
        DCHECK(!_n.HasNaNs());
        return;
    }

    /**
     * @brief = 重载
     * @param  _v               另一个 Vector3<_T>
     * @return Vector3<_T>&     结果
     */
    Vector3<_T> &operator=(const Vector3<_T> &_v) {
        DCHECK(!_v.HasNaNs());
        x = _v.x;
        y = _v.y;
        z = _v.z;
        return *this;
    }

    /**
     * @brief == 重载
     * @param  _v              另一个 Vector3<_T>
     * @return true            相等
     * @return false           不相等
     */
    bool operator==(const Vector3<_T> &_v) const {
        return x == _v.x && y == _v.y && z == _v.z;
    }

    /**
     * @brief != 重载
     * @param  _v              另一个 Vector3<_T>
     * @return true            不相等
     * @return false           相等
     */
    bool operator!=(const Vector3<_T> &_v) const {
        return x != _v.x || y != _v.y || z != _v.z;
    }

    /**
     * @brief + 重载，向量加
     * @param  _v               另一个 Vector3<_T>
     * @return Vector3<_T>      结果
     */
    Vector3<_T> operator+(const Vector3<_T> &_v) const {
        DCHECK(!_v.HasNaNs());
        return Vector3(x + _v.x, y + _v.y, z + _v.z);
    }

    /**
     * @brief += 重载，向量加
     * @param  _v               另一个 Vector3<_T>
     * @return Vector3<_T>&     结果
     */
    Vector3<_T> &operator+=(const Vector3<_T> &_v) {
        DCHECK(!_v.HasNaNs());
        x += _v.x;
        y += _v.y;
        z += _v.z;
        return *this;
    }

    /**
     * @brief - 重载，向量反向
     * @return Vector3<_T>     结果
     */
    Vector3<_T> operator-() const {
        return Vector3<_T>(-x, -y, -z);
    }

    /**
     * @brief - 重载，向量减
     * @param  _v               另一个 Vector3<_T>
     * @return Vector3<_T>      结果
     */
    Vector3<_T> operator-(const Vector3<_T> &_v) const {
        DCHECK(!_v.HasNaNs());
        return Vector3(x - _v.x, y - _v.y, z - _v.z);
    }

    /**
     * @brief -= 重载，向量减
     * @param  _v              另一个 Vector3<_T>
     * @return Vector3<_T>      结果
     */
    Vector3<_T> &operator-=(const Vector3<_T> &_v) {
        DCHECK(!_v.HasNaNs());
        x -= _v.x;
        y -= _v.y;
        z -= _v.z;
        return *this;
    }

    /**
     * @brief * 重载，向量数乘
     * @param  _U              数的类型
     * @param  _s              数
     * @return Vector3<_T>     结果
     */
    template <class _U>
    Vector3<_T> operator*(_U _s) const {
        return Vector3<_T>(_s * x, _s * y, _s * z);
    }

    /**
     * @brief * 重载，向量点积
     * @param  _v              要乘的向量
     * @return _T              结果
     */
    _T operator*(Vector3<_T> _v) const {
        DCHECK(!_v.HasNaNs());
        return x * _v.x + y * _v.y + z * _v.z;
    }

    /**
     * @brief *= 重载，向量数乘
     * @param  _U              数的类型
     * @param  _s              数
     * @return Vector3<_T>&    结果
     */
    template <class _U>
    Vector3<_T> &operator*=(_U _s) {
        DCHECK(!isNaN(_s));
        x *= _s;
        y *= _s;
        z *= _s;
        return *this;
    }

    /**
     * @brief / 重载，向量数除
     * @param  _U              数的类型
     * @param  _f              数
     * @return Vector3<_T>     结果
     */
    template <class _U>
    Vector3<_T> operator/(_U _f) const {
        CHECK_NE(_f, 0);
        _T inv = (_T)1 / _f;
        return Vector3<_T>(x * inv, y * inv, z * inv);
    }

    /**
     * @brief /= 重载，向量数除
     * @param  _U              数的类型
     * @param  _f              数
     * @return Vector3<_T>&    结果
     */
    template <class _U>
    Vector3<_T> &operator/=(_U _f) {
        CHECK_NE(_f, 0);
        _T inv = (_T)1 / _f;
        x *= inv;
        y *= inv;
        z *= inv;
        return *this;
    }

    /**
     * @brief [] 重载
     * @param  _idx            下标
     * @return _T              结果
     */
    _T operator[](int _idx) const {
        DCHECK(_idx >= 0 && _idx <= 2);
        if (_idx == 0)
            return x;
        if (_idx == 1)
            return y;
        return z;
    }

    /**
     * @brief [] 重载
     * @param  _idx            下标
     * @return _T&             结果
     */
    _T &operator[](int _idx) {
        DCHECK(_idx >= 0 && _idx <= 2);
        if (_idx == 0)
            return x;
        if (_idx == 1)
            return y;
        return z;
    }

    /**
     * @brief 距离^2
     * @return _T              距离的平方
     */
    _T LengthSquared(void) const {
        return x * x + y * y + z * z;
    }

    /**
     * @brief 距离
     * @return _T              距离
     */
    _T Length(void) const {
        return std::sqrt(LengthSquared());
    }

    /**
     * @brief 是否有非数值
     * @return true             有
     * @return false            无
     */
    bool HasNaNs(void) const {
        return isNaN(x) || isNaN(y) || isNaN(z);
    }

    /**
     * @brief 对所有分量取绝对值
     * @param  _v              向量
     * @return Vector2<_T>     结果
     */
    Vector3<_T> Abs(const Vector3<_T> &_v) {
        return Vector3<_T>(std::abs(_v.x), std::abs(_v.y), std::abs(_v.z));
    }

    /**
     * @brief 点积
     * @param  _v1             向量1
     * @param  _v2             向量2
     * @return _T              结果
     */
    inline _T Dot(const Vector3<_T> &_v1, const Vector3<_T> &_v2) {
        DCHECK(!_v1.HasNaNs() && !_v2.HasNaNs());
        return _v1.x * _v2.x + _v1.y * _v2.y + _v1.z * _v2.z;
    }

    /**
     * @brief 点积的绝对值
     * @param  _v1             向量1
     * @param  _v2             向量2
     * @return _T              结果
     */
    inline _T AbsDot(const Vector3<_T> &_v1, const Vector3<_T> &_v2) {
        DCHECK(!_v1.HasNaNs() && !_v2.HasNaNs());
        return std::abs(Dot(_v1, _v2));
    }

    /**
     * @brief 叉积
     * @param  _v1             向量1
     * @param  _v2             向量2
     * @return Vector2<_T>     结果
     */
    inline Vector3<_T> Cross(const Vector3<_T> &_v1, const Vector3<_T> &_v2) {
        DCHECK(!_v1.HasNaNs() && !_v2.HasNaNs());
        _T v1x = _v1.x, v1y = _v1.y, v1z = _v1.z;
        _T v2x = _v2.x, v2y = _v2.y, v2z = _v2.z;
        return Vector3<_T>((v1y * v2z) - (v1z * v2y), (v1z * v2x) - (v1x * v2z),
                           (v1x * v2y) - (v1y * v2x));
    }

    /**
     * @brief 叉积
     * @param  _v1             向量
     * @param  _v2             法向量
     * @return Vector2<_T>     结果
     */
    inline Vector3<_T> Cross(const Vector3<_T> &_v1, const Normal3<_T> &_v2) {
        DCHECK(!_v1.HasNaNs() && !_v2.HasNaNs());
        _T v1x = _v1.x, v1y = _v1.y, v1z = _v1.z;
        _T v2x = _v2.x, v2y = _v2.y, v2z = _v2.z;
        return Vector3<_T>((v1y * v2z) - (v1z * v2y), (v1z * v2x) - (v1x * v2z),
                           (v1x * v2y) - (v1y * v2x));
    }

    /**
     * @brief 叉积
     * @param  _v1             法向量
     * @param  _v2             向量
     * @return Vector2<_T>     结果
     */
    inline Vector3<_T> Cross(const Normal3<_T> &_v1, const Vector3<_T> &_v2) {
        DCHECK(!_v1.HasNaNs() && !_v2.HasNaNs());
        _T v1x = _v1.x, v1y = _v1.y, v1z = _v1.z;
        _T v2x = _v2.x, v2y = _v2.y, v2z = _v2.z;
        return Vector3<_T>((v1y * v2z) - (v1z * v2y), (v1z * v2x) - (v1x * v2z),
                           (v1x * v2y) - (v1y * v2x));
    }

    /**
     * @brief 归一化
     * @param  _v              向量
     * @return Vector2<_T>     结果
     */
    inline Vector3<_T> Normalize(const Vector3<_T> &_v) {
        return _v / _v.Length();
    }

    /**
     * @brief 最小分量
     * @param  _v              向量
     * @return _T              结果
     */
    _T MinComponent(const Vector3<_T> &_v) {
        return std::min(_v.x, std::min(_v.y, _v.z));
    }

    /**
     * @brief 最大分量
     * @param  _v              向量
     * @return _T              结果
     */
    _T MaxComponent(const Vector3<_T> &_v) {
        return std::max(_v.x, std::max(_v.y, _v.z));
    }

    /**
     * @brief 最大分量下标
     * @param  _v              向量
     * @return int             结果
     */
    int MaxDimension(const Vector3<_T> &_v) {
        return (_v.x > _v.y) ? ((_v.x > _v.z) ? 0 : 2)
                             : ((_v.y > _v.z) ? 1 : 2);
    }

    /**
     * @brief 构造最小向量
     * @param  _v1             向量1
     * @param  _v2             向量2
     * @return Vector2<_T>     结果
     */
    Vector3<_T> Min(const Vector3<_T> &_p1, const Vector3<_T> &_p2) {
        return Vector3<_T>(std::min(_p1.x, _p2.x), std::min(_p1.y, _p2.y),
                           std::min(_p1.z, _p2.z));
    }

    /**
     * @brief 构造最大向量
     * @param  _v1             向量1
     * @param  _v2             向量2
     * @return Vector2<_T>     结果
     */
    Vector3<_T> Max(const Vector3<_T> &_p1, const Vector3<_T> &_p2) {
        return Vector3<_T>(std::max(_p1.x, _p2.x), std::max(_p1.y, _p2.y),
                           std::max(_p1.z, _p2.z));
    }

    /**
     * @brief 重新排列向量
     * @param  _v              向量
     * @param  _x              新的 x 在 _v 中的下标
     * @param  _y              新的 y 在 _v 中的下标
     * @param  _z              新的 z 在 _v 中的下标
     * @return Vector2<_T>     结果
     */
    Vector3<_T> Permute(const Vector3<_T> &_v, int _x, int _y, int _z) {
        return Vector3<_T>(_v[_x], _v[_y], _v[_z]);
    }

    /**
     * @brief 基于 _v1 构建出另外两个正交向量 _v2，_v3
     * @param  _v1             向量1
     * @param  _v2             向量2，用于获取返回值
     * @param  _v3             向量3，用于获取返回值
     */
    inline void CoordinateSystem(const Vector3<_T> &_v1, Vector3<_T> *_v2,
                                 Vector3<_T> *_v3) {
        if (std::abs(_v1.x) > std::abs(_v1.y)) {
            *_v2 = Vector3<_T>(-_v1.z, 0, _v1.x) /
                   std::sqrt(_v1.x * _v1.x + _v1.z * _v1.z);
        }
        else {
            *_v2 = Vector3<_T>(0, _v1.z, -_v1.y) /
                   std::sqrt(_v1.y * _v1.y + _v1.z * _v1.z);
        }
        *_v3 = Cross(_v1, *_v2);
        return;
    }

    friend std::ostream &operator<<(std::ostream &os, const Vector3<_T> &_v) {
        os << "[ " << _v.x << ", " << _v.y << ", " << _v.z << " ]";
        return os;
    }
};

typedef Vector2<float>  Vector2f;
typedef Vector2<int>    Vector2i;
typedef Vector2<size_t> Vector2s;
typedef Vector3<float>  Vector3f;
typedef Vector3<int>    Vector3i;

#endif /* __VECTOR_HPP__ */
