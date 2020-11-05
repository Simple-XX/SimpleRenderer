
// This file is a part of SimpleXX/SimpleRenderer
// (https://github.com/SimpleXX/SimpleRenderer).
//
// vector.hpp for SimpleXX/SimpleRenderer.

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
    Vector<T, N> operator-(void) const;
    // 范数
    double norm(void) const;
    // 单位向量
    Vector<double, N> unit(void) const;
    // 向量和
    Vector<T, N> operator+(const Vector<T, N> &_v) const;
    // 向量自加
    Vector<T, N> &operator+=(const Vector<T, N> &_v) const;
    // 向量差
    Vector<T, N> operator-(const Vector<T, N> &_v) const;
    // 向量自减
    Vector<T, N> &operator-=(const Vector<T, N> &_v) const;
    // 向量数乘
    Vector<T, N> operator*(const T _t) const;
    // 向量自乘
    Vector<T, N> &operator*=(const T _t) const;
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
    // coord.x = _vect[0];
    // coord.y = _vect[1];
    // if (N == 3) {
    //     coord.z = _vect[2];
    // }
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
Vector<T, N> Vector<T, N>::operator-(void) const {
    T tmp[N];
    tmp[0] = -coord.x;
    tmp[1] = -coord.y;
    if (N == 3) {
        tmp[2] = -coord.z;
    }
    return Vector<T, N>(tmp);
}

template <class T, size_t N>
double Vector<T, N>::norm(void) const {
    double res = 0;
    res += coord.x * coord.x;
    res += coord.y * coord.y;
    res += coord.z * coord.z;
    res = sqrt(res);
    return res;
}

template <class T, size_t N>
Vector<double, N> Vector<T, N>::unit(void) const {
    double tmp[N];
    double norm = this->norm();
    tmp[0]      = coord.x / norm;
    tmp[1]      = coord.y / norm;
    if (N == 3) {
        tmp[2] = coord.z / norm;
    }
    return Vector<double, N>(tmp);
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
Vector<T, N> &Vector<T, N>::operator+=(const Vector<T, N> &_v) const {
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
Vector<T, N> &Vector<T, N>::operator-=(const Vector<T, N> &_v) const {
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
Vector<T, N> &Vector<T, N>::operator*=(const T _t) const {
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

typedef Vector<int, 2>    Vectori2;
typedef Vector<int, 3>    Vectori3;
typedef Vector<double, 2> Vectord2;
typedef Vector<double, 3> Vectord3;
typedef Vector<double, 4> Vectord4;

#endif /* __VECTOR_HPP__ */
