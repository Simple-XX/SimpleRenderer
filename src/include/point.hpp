
/**
 * @file point.hpp
 * @brief 点模版
 * @author Zone.N (Zone.Niuzh@hotmail.com)
 * @version 1.0
 * @date 2022-06-07
 * @copyright MIT LICENSE
 * https://github.com/Simple-XX/SimpleRenderer
 * Based on https://github.com/mmp/pbrt-v3
 * @par change log:
 * <table>
 * <tr><th>Date<th>Author<th>Description
 * <tr><td>2022-06-07<td>Zone.N<td>创建文件
 * </table>
 */

#ifndef __POINT_HPP__
#define __POINT_HPP__

#include "iostream"
#include "cmath"

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
 * @brief 二维点模版
 * @tparam _T                类型
 */
template <class _T>
class Point2 {
public:
    _T x;
    _T y;

    /**
     * @brief 构造函数
     */
    Point2(void);

    /**
     * @brief 构造函数
     * @param  _x              x 值
     * @param  _y              y 值
     */
    Point2(_T _x, _T _y);

    /**
     * @brief 构造函数
     * @param  _p               二维点
     */
    Point2(const Point2<_T> &_p);

    /**
     * @brief 构造函数
     * @tparam _U               二维点类型
     * @param  _p               二维点
     */
    template <class _U>
    explicit Point2(const Point2<_U> &_p);

    /**
     * @brief 构造函数
     * @tparam _U               二维向量类型
     * @param  _p               二维向量
     */
    template <class _U>
    explicit Point2(const Vector2<_U> &_p);

    /**
     * @brief 构造函数
     * @param  _p               三维点
     */
    explicit Point2(const Point3<_T> &_p);

    /**
     * @brief () 重载，类型转换
     * @tparam _U               要转换的 Vector2 类型
     * @return Vector2<_U>      结果
     */
    template <class _U>
    explicit operator Vector2<_U>() const;

    /**
     * @brief = 重载
     * @param  _p               另一个 Point2<_T>
     * @return Point2<_T>&      结果
     */
    Point2<_T> &operator=(const Point2<_T> &_p);

    /**
     * @brief == 重载
     * @param  _p               另一个 Point2<_T>
     * @return true             相等
     * @return false            不相等
     */
    bool operator==(const Point2<_T> &_p) const;

    /**
     * @brief != 重载
     * @param  _p               另一个 Point2<_T>
     * @return true             不相等
     * @return false            相等
     */
    bool operator!=(const Point2<_T> &_p) const;

    /**
     * @brief + 重载
     * @param  _p               另一个 Point2<_T>
     * @return Point2<_T>       结果
     */
    Point2<_T> operator+(const Point2<_T> &_p) const;

    /**
     * @brief += 重载
     * @param  _p               另一个 Point2<_T>
     * @return Point2<_T>&      结果
     */
    Point2<_T> &operator+=(const Point2<_T> &_p);

    /**
     * @brief + 重载
     * @param  _v               另一个 Vector2<_T>
     * @return Point2<_T>       结果
     */
    Point2<_T> operator+(const Vector2<_T> &_v) const;

    /**
     * @brief += 重载
     * @param  _v               另一个 Vector2<_T>
     * @return Point2<_T>&      结果
     */
    Point2<_T> &operator+=(const Vector2<_T> &_v);

    /**
     * @brief - 重载，向量反向
     * @return Point2<_T>      结果
     */
    Point2<_T> operator-() const;

    /**
     * @brief - 重载，向量减
     * @param  _p               要减的 Point2<_T>
     * @return Vector2<_T>      结果
     */
    Vector2<_T> operator-(const Point2<_T> &_p) const;

    /**
     * @brief - 重载，向量减
     * @param  _v               另一个 Vector3<_T>
     * @return Point2<_T>       结果
     */
    Point2<_T> operator-(const Vector2<_T> &_v) const;

    /**
     * @brief -= 重载，向量减
     * @param  _v               Vector2<_T>
     * @return Point2<_T>       结果
     */
    Point2<_T> &operator-=(const Vector2<_T> &_v);

    /**
     * @brief * 重载，向量数乘
     * @tparam _U              数的类型
     * @param  _f              数
     * @return Point2<_T>      结果
     */
    template <class _U>
    Point2<_T> operator*(_U _f) const;

    /**
     * @brief *= 重载，向量数乘
     * @tparam _U              数的类型
     * @param  _f              数
     * @return Point2<_T>&     结果
     */
    template <class _U>
    Point2<_T> &operator*=(_U _f);

    /**
     * @brief / 重载，向量数除
     * @tparam _U              相除的数的类型
     * @param  _f              相除的数
     * @return Point2<_T>      结果
     */
    template <class _U>
    Point2<_T> operator/(_U _f) const;

    /**
     * @brief /= 重载，向量数除
     * @tparam _U              数的类型
     * @param  _f              数
     * @return Point2<_T>&     结果
     */
    template <class _U>
    Point2<_T> &operator/=(_U _f);

    /**
     * @brief [] 重载
     * @param  _idx            下标
     * @return _T              结果
     */
    _T operator[](int _idx) const;

    /**
     * @brief [] 重载
     * @param  _idx            下标
     * @return _T&             结果
     */
    _T &operator[](int _idx);

    /**
     * @brief 是否有非数值
     * @return true             有
     * @return false            无
     */
    bool HasNaNs(void) const;

    friend std::ostream &operator<<(std::ostream &_os, const Point2<_T> &_v);
};

template <class _T>
Point2<_T>::Point2(void) {
    x = 0;
    y = 0;
    return;
}

template <class _T>
Point2<_T>::Point2(_T _x, _T _y) : x(_x), y(_y) {
    assert(!HasNaNs());
    return;
}

template <class _T>
Point2<_T>::Point2(const Point2<_T> &_p) {
    assert(!_p.HasNaNs());
    x = _p.x;
    y = _p.y;
    return;
}

template <class _T>
template <class _U>
Point2<_T>::Point2(const Point2<_U> &_p) {
    x = (_T)_p.x;
    y = (_T)_p.y;
    assert(!HasNaNs());
    return;
}

template <class _T>
template <class _U>
Point2<_T>::Point2(const Vector2<_U> &_p) {
    x = (_T)_p.x;
    y = (_T)_p.y;
    assert(!HasNaNs());
    return;
}

template <class _T>
Point2<_T>::Point2(const Point3<_T> &_p) : x(_p.x), y(_p.y) {
    assert(!HasNaNs());
    return;
}

template <class _T>
template <class _U>
Point2<_T>::operator Vector2<_U>() const {
    return Vector2<_U>(x, y);
}

template <class _T>
Point2<_T> &Point2<_T>::operator=(const Point2<_T> &_p) {
    assert(!_p.HasNaNs());
    x = _p.x;
    y = _p.y;
    return *this;
}

template <class _T>
bool Point2<_T>::operator==(const Point2<_T> &_p) const {
    return x == _p.x && y == _p.y;
}

template <class _T>
bool Point2<_T>::operator!=(const Point2<_T> &_p) const {
    return x != _p.x || y != _p.y;
}

template <class _T>
Point2<_T> Point2<_T>::operator+(const Point2<_T> &_p) const {
    assert(!_p.HasNaNs());
    return Point2<_T>(x + _p.x, y + _p.y);
}

template <class _T>
Point2<_T> &Point2<_T>::operator+=(const Point2<_T> &_p) {
    assert(!_p.HasNaNs());
    x += _p.x;
    y += _p.y;
    return *this;
}

template <class _T>
Point2<_T> Point2<_T>::operator+(const Vector2<_T> &_v) const {
    assert(!_v.HasNaNs());
    return Point2<_T>(x + _v.x, y + _v.y);
}

template <class _T>
Point2<_T> &Point2<_T>::operator+=(const Vector2<_T> &_v) {
    assert(!_v.HasNaNs());
    x += _v.x;
    y += _v.y;
    return *this;
}

template <class _T>
Point2<_T> Point2<_T>::operator-() const {
    return Point2<_T>(-x, -y);
}

template <class _T>
Vector2<_T> Point2<_T>::operator-(const Point2<_T> &_p) const {
    assert(!_p.HasNaNs());
    return Vector2<_T>(x - _p.x, y - _p.y);
}

template <class _T>
Point2<_T> Point2<_T>::operator-(const Vector2<_T> &_v) const {
    assert(!_v.HasNaNs());
    return Point2<_T>(x - _v.x, y - _v.y);
}

template <class _T>
Point2<_T> &Point2<_T>::operator-=(const Vector2<_T> &_v) {
    assert(!_v.HasNaNs());
    x -= _v.x;
    y -= _v.y;
    return *this;
}

template <class _T>
template <class _U>
Point2<_T> Point2<_T>::operator*(_U _f) const {
    return Point2<_T>(_f * x, _f * y);
}

template <class _T>
template <class _U>
Point2<_T> &Point2<_T>::operator*=(_U _f) {
    x *= _f;
    y *= _f;
    return *this;
}

template <class _T>
template <class _U>
Point2<_T> Point2<_T>::operator/(_U _f) const {
    assert(_f != 0);
    _T inv = (_T)1 / _f;
    return Point2<_T>(inv * x, inv * y);
}

template <class _T>
template <class _U>
Point2<_T> &Point2<_T>::operator/=(_U _f) {
    assert(_f != 0);
    _T inv = (_T)1 / _f;
    x *= inv;
    y *= inv;
    return *this;
}

template <class _T>
_T Point2<_T>::operator[](int _idx) const {
    assert(_idx >= 0 && _idx <= 1);
    if (_idx == 0) {
        return x;
    }
    return y;
}

template <class _T>
_T &Point2<_T>::operator[](int _idx) {
    assert(_idx >= 0 && _idx <= 1);
    if (_idx == 0) {
        return x;
    }
    return y;
}

template <class _T>
bool Point2<_T>::HasNaNs(void) const {
    return isNaN(x) || isNaN(y);
}

template <class _T>
std::ostream &operator<<(std::ostream &_os, const Point2<_T> &_v) {
    _os << "[ " << _v.x << ", " << _v.y << " ]";
    return _os;
}

/**
 * @brief 三维点模版
 * @tparam _T                类型
 */
template <class _T>
class Point3 {
public:
    _T x;
    _T y;
    _T z;

    /**
     * @brief 构造函数
     */
    Point3(void);

    /**
     * @brief 构造函数
     * @param  _x               x 值
     * @param  _y               y 值
     * @param  _z               z 值
     */
    Point3(_T _x, _T _y, _T _z);

    /**
     * @brief 构造函数
     * @tparam _U               类型
     * @param  _p               另一个 Point3<_U>
     */
    template <class _U>
    explicit Point3(const Point3<_U> &_p);

    /**
     * @brief 构造函数
     * @param  _p               另一个 Point3<_T>
     */
    Point3(const Point3<_T> &_p);

    /**
     * @brief () 重载，类型转换
     * @tparam _U
     * @return Vector3<_U>
     */
    template <class _U>
    explicit operator Vector3<_U>() const;

    /**
     * @brief = 重载
     * @param  _p               另一个 Point3<_T>
     * @return Point3<_T>&      结果
     */
    Point3<_T> &operator=(const Point3<_T> &_p);

    /**
     * @brief == 重载
     * @param  _p               另一个 Point3<_T>
     * @return true             相等
     * @return false            不相等
     */
    bool operator==(const Point3<_T> &_p) const;

    /**
     * @brief != 重载
     * @param  _p               另一个 Point3<_T>
     * @return true             不相等
     * @return false            相等
     */
    bool operator!=(const Point3<_T> &_p) const;

    /**
     * @brief + 重载
     * @param  _p               另一个 Point3<_T>
     * @return Point3<_T>       结果
     */
    Point3<_T> operator+(const Point3<_T> &_p) const;

    /**
     * @brief += 重载
     * @param  _p               另一个 Point3<_T>
     * @return Point3<_T>&      结果
     */
    Point3<_T> &operator+=(const Point3<_T> &_p);

    /**
     * @brief + 重载
     * @param  _v               另一个 Vector3<_T>
     * @return Point3<_T>       结果
     */
    Point3<_T> operator+(const Vector3<_T> &_v) const;

    /**
     * @brief += 重载
     * @param  _v               另一个 Vector3<_T>
     * @return Point3<_T>&      结果
     */
    Point3<_T> &operator+=(const Vector3<_T> &_v);

    /**
     * @brief - 重载，取负
     * @return Point3<_T>       结果
     */
    Point3<_T> operator-() const;

    /**
     * @brief - 重载
     * @param  _p               另一个 Point3<_T>
     * @return Vector3<_T>      结果
     */
    Vector3<_T> operator-(const Point3<_T> &_p) const;

    /**
     * @brief - 重载
     * @param  _v               另一个 Vector3<_T>
     * @return Point3<_T>       结果
     */
    Point3<_T> operator-(const Vector3<_T> &_v) const;

    /**
     * @brief -= 重载
     * @param  _v               另一个 Vector3<_T>
     * @return Point3<_T>&      结果
     */
    Point3<_T> &operator-=(const Vector3<_T> &_v);

    /**
     * @brief * 重载，数乘
     * @tparam _U               数的类型
     * @param  _f               数
     * @return Point3<_T>       结果
     */
    template <class _U>
    Point3<_T> operator*(_U _f) const;

    /**
     * @brief *= 重载，数乘
     * @tparam _U               数的类型
     * @param  _f               数
     * @return Point3<_T>&      结果
     */
    template <class _U>
    Point3<_T> &operator*=(_U _f);

    /**
     * @brief / 重载，数除
     * @tparam _U               数的类型
     * @param  _f               数
     * @return Point3<_T>       结果
     */
    template <class _U>
    Point3<_T> operator/(_U _f) const;

    /**
     * @brief /= 重载，数除
     * @tparam _U               数的类型
     * @param  _f               数
     * @return Point3<_T>&      结果
     */
    template <class _U>
    Point3<_T> &operator/=(_U _f);

    /**
     * @brief [] 重载
     * @param  _idx             下标
     * @return _T               结果
     */
    _T operator[](int _idx) const;

    /**
     * @brief [] 重载
     * @param  _idx             下标
     * @return _T&              结果
     */
    _T &operator[](int _idx);

    /**
     * @brief 是否有非数值
     * @return true             有
     * @return false            无
     */
    bool HasNaNs(void) const;

    friend std::ostream &operator<<(std::ostream &_os, const Point3<_T> &_v);
};

template <class _T>
Point3<_T>::Point3(void) {
    x = 0;
    y = 0;
    z = 0;
    return;
}

template <class _T>
Point3<_T>::Point3(_T _x, _T _y, _T _z) : x(_x), y(_y), z(_z) {
    assert(!HasNaNs());
    return;
}

template <class _T>
template <class _U>
Point3<_T>::Point3(const Point3<_U> &_p)
    : x((_T)_p.x), y((_T)_p.y), z((_T)_p.z) {
    assert(!HasNaNs());
    return;
}

template <class _T>
Point3<_T>::Point3(const Point3<_T> &_p) {
    assert(!_p.HasNaNs());
    x = _p.x;
    y = _p.y;
    z = _p.z;
    return;
}

template <class _T>
template <class _U>
Point3<_T>::operator Vector3<_U>() const {
    return Vector3<_U>(x, y, z);
}

template <class _T>
Point3<_T> &Point3<_T>::operator=(const Point3<_T> &_p) {
    assert(!_p.HasNaNs());
    x = _p.x;
    y = _p.y;
    z = _p.z;
    return *this;
}

template <class _T>
bool Point3<_T>::operator==(const Point3<_T> &_p) const {
    return x == _p.x && y == _p.y && z == _p.z;
}

template <class _T>
bool Point3<_T>::operator!=(const Point3<_T> &_p) const {
    return x != _p.x || y != _p.y || z != _p.z;
}

template <class _T>
Point3<_T> Point3<_T>::operator+(const Point3<_T> &_p) const {
    assert(!_p.HasNaNs());
    return Point3<_T>(x + _p.x, y + _p.y, z + _p.z);
}

template <class _T>
Point3<_T> &Point3<_T>::operator+=(const Point3<_T> &_p) {
    assert(!_p.HasNaNs());
    x += _p.x;
    y += _p.y;
    z += _p.z;
    return *this;
}

template <class _T>
Point3<_T> Point3<_T>::operator+(const Vector3<_T> &_v) const {
    assert(!_v.HasNaNs());
    return Point3<_T>(x + _v.x, y + _v.y, z + _v.z);
}

template <class _T>
Point3<_T> &Point3<_T>::operator+=(const Vector3<_T> &_v) {
    assert(!_v.HasNaNs());
    x += _v.x;
    y += _v.y;
    z += _v.z;
    return *this;
}

template <class _T>
Point3<_T> Point3<_T>::operator-() const {
    return Point3<_T>(-x, -y, -z);
}

template <class _T>
Vector3<_T> Point3<_T>::operator-(const Point3<_T> &_p) const {
    assert(!_p.HasNaNs());
    return Vector3<_T>(x - _p.x, y - _p.y, z - _p.z);
}

template <class _T>
Point3<_T> Point3<_T>::operator-(const Vector3<_T> &_v) const {
    assert(!_v.HasNaNs());
    return Point3<_T>(x - _v.x, y - _v.y, z - _v.z);
}

template <class _T>
Point3<_T> &Point3<_T>::operator-=(const Vector3<_T> &_v) {
    assert(!_v.HasNaNs());
    x -= _v.x;
    y -= _v.y;
    z -= _v.z;
    return *this;
}

template <class _T>
template <class _U>
Point3<_T> Point3<_T>::operator*(_U _f) const {
    return Point3<_T>(_f * x, _f * y, _f * z);
}

template <class _T>
template <class _U>
Point3<_T> &Point3<_T>::operator*=(_U _f) {
    x *= _f;
    y *= _f;
    z *= _f;
    return *this;
}

template <class _T>
template <class _U>
Point3<_T> Point3<_T>::operator/(_U _f) const {
    assert(_f != 0);
    _T inv = (_T)1 / _f;
    return Point3<_T>(inv * x, inv * y, inv * z);
}

template <class _T>
template <class _U>
Point3<_T> &Point3<_T>::operator/=(_U _f) {
    assert(_f != 0);
    _T inv = (_T)1 / _f;
    x *= inv;
    y *= inv;
    z *= inv;
    return *this;
}

template <class _T>
_T Point3<_T>::operator[](int _idx) const {
    assert(_idx >= 0 && _idx <= 2);
    if (_idx == 0) {
        return x;
    }
    if (_idx == 1) {
        return y;
    }
    return z;
}

template <class _T>
_T &Point3<_T>::operator[](int _idx) {
    assert(_idx >= 0 && _idx <= 2);
    if (_idx == 0) {
        return x;
    }
    if (_idx == 1) {
        return y;
    }
    return z;
}

template <class _T>
bool Point3<_T>::HasNaNs(void) const {
    return isNaN(x) || isNaN(y) || isNaN(z);
}

template <class _T>
std::ostream &operator<<(std::ostream &_os, const Point3<_T> &_v) {
    _os << "[ " << _v.x << ", " << _v.y << ", " << _v.z << " ]";
    return _os;
}

typedef Point2<float> Point2f;
typedef Point2<int>   Point2i;
typedef Point3<float> Point3f;
typedef Point3<int>   Point3i;

#endif /* __POINT_HPP__ */
