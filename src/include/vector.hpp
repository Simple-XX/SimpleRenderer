
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
        T a[3];
        struct {
            T x;
            T y;
            T z;
        } coord;
    };
    // 范数
    double norm;
    // 单位向量
    double unit[N];

    Vector(void);
    Vector(T *_vect);
    Vector(std::vector<T> _vect);
    Vector(T _vect1, T _vect2);
    Vector(T _vect1, T _vect2, T _vect3);
    ~Vector(void);
    // 获取 vect
    const T *get_vect(void) const;
    // 更新 vect
    void set_vect(T _val, size_t _i);
    // 向量相等
    bool operator==(const Vector<T, N> &_v) const;
    // 向量不为零向量
    operator bool(void) const;
    // 比较每个分量
    bool operator>(const Vector<T, N> &_v) const;
    bool operator>=(const Vector<T, N> &_v) const;
    bool operator<(const Vector<T, N> &_v) const;
    bool operator<=(const Vector<T, N> &_v) const;
    // 向量取反
    Vector<T, N> operator-(void) const;
    // 范数
    double get_norm(void) const;
    // 单位向量
    const double *get_unit(void) const;
    // 向量和
    Vector<T, N> operator+(const Vector<T, N> &_v) const;
    // 自增，+1 还是 +单位向量？
    // Vector<T, N> & operator++ ();
    // Vector<T, N> operator++ (int);
    // 自减
    // Vector<T, N> & operator-- ();
    // Vector<T, N> operator-- (int);
    // 向量差
    Vector<T, N> operator-(const Vector<T, N> &_v) const;
    // 向量数乘
    Vector<T, N> operator*(T _t) const;
    // 点积
    T operator*(const Vector<T, N> &_v) const;
    // 叉积
    Vector<T, N> operator^(const Vector<T, N> &_v) const;
    // 向量投影
};

template <class T, size_t N>
Vector<T, N>::Vector(void) {
    for (size_t i = 0; i < N; i++) {
        vect[i] = 0;
        unit[i] = 0;
    }
    norm = 0;
    return;
}

template <class T, size_t N>
Vector<T, N>::Vector(T *_vect) {
    for (size_t i = 0; i < N; i++) {
        vect[i] = _vect[i];
    }
    T square_sum = 0;
    for (size_t i = 0; i < N; i++) {
        square_sum += vect[i] * vect[i];
    }
    norm = sqrt(square_sum);
    for (size_t i = 0; i < N; i++) {
        unit[i] = (vect[i] / norm);
    }
    return;
}

template <class T, size_t N>
Vector<T, N>::Vector(std::vector<T> _vect) {
    for (size_t i = 0; i < N; i++) {
        vect[i] = _vect.at(i);
    }
    T square_sum = 0;
    for (size_t i = 0; i < N; i++) {
        square_sum += vect[i] * vect[i];
    }
    norm = sqrt(square_sum);
    for (size_t i = 0; i < N; i++) {
        unit[i] = (vect[i] / norm);
    }
    return;
}

template <class T, size_t N>
Vector<T, N>::Vector(T _vect1, T _vect2) {
    assert(N == 2);
    vect[0]      = _vect1;
    vect[1]      = _vect2;
    T square_sum = vect[0] + vect[1];
    norm         = sqrt(square_sum);
    unit[0]      = vect[0] / norm;
    unit[1]      = vect[1] / norm;
    return;
}

template <class T, size_t N>
Vector<T, N>::Vector(T _vect1, T _vect2, T _vect3) {
    assert(N == 3);
    vect[0]      = _vect1;
    vect[1]      = _vect2;
    vect[2]      = _vect3;
    T square_sum = vect[0] + vect[1] + vect[2];
    norm         = sqrt(square_sum);
    unit[0]      = vect[0] / norm;
    unit[1]      = vect[1] / norm;
    unit[2]      = vect[2] / norm;
    return;
}

template <class T, size_t N>
Vector<T, N>::~Vector(void) {
    return;
}

template <class T, size_t N>
const T *Vector<T, N>::get_vect(void) const {
    return vect;
}

template <class T, size_t N>
void Vector<T, N>::set_vect(T _val, size_t _i) {
    vect[_i] = _val;
    return;
}

template <class T, size_t N>
bool Vector<T, N>::operator==(const Vector<T, N> &_v) const {
    bool res = true;
    for (size_t i = 0; i < N; i++) {
        if (vect[i] != _v.get_vect()[i]) {
            res = false;
        }
    }
    return res;
}

template <class T, size_t N>
Vector<T, N>::operator bool(void) const {
    bool res = false;
    for (size_t i = 0; i < N; i++) {
        if (vect[i] != 0) {
            res = true;
        }
    }
    return res;
}

template <class T, size_t N>
bool Vector<T, N>::operator>(const Vector<T, N> &_v) const {
    bool res = true;
    for (size_t i = 0; i < N; i++) {
        if (vect[i] <= _v.get_vect()[i]) {
            res = false;
        }
    }
    return res;
}

template <class T, size_t N>
bool Vector<T, N>::operator>=(const Vector<T, N> &_v) const {
    bool res = true;
    for (size_t i = 0; i < N; i++) {
        if (vect[i] != _v.get_vect()[i]) {
            res = false;
        }
    }
    return res;
}

template <class T, size_t N>
bool Vector<T, N>::operator<(const Vector<T, N> &_v) const {
    bool res = true;
    for (size_t i = 0; i < N; i++) {
        if (vect[i] >= _v.get_vect()[i]) {
            res = false;
        }
    }
    return res;
}

template <class T, size_t N>
bool Vector<T, N>::operator<=(const Vector<T, N> &_v) const {
    bool res = true;
    for (size_t i = 0; i < N; i++) {
        if (vect[i] > _v.get_vect()[i]) {
            res = false;
        }
    }
    return res;
}

template <class T, size_t N>
Vector<T, N> Vector<T, N>::operator-(void) const {
    T tmp[N];
    for (size_t i = 0; i < N; i++) {
        tmp[i] = -vect[i];
    }
    return Vector<T, N>(tmp);
}

template <class T, size_t N>
double Vector<T, N>::get_norm(void) const {
    return norm;
}

template <class T, size_t N>
const double *Vector<T, N>::get_unit(void) const {
    return unit;
}

template <class T, size_t N>
Vector<T, N> Vector<T, N>::operator+(const Vector<T, N> &_v) const {
    T tmp[N];
    for (size_t i = 0; i < N; i++) {
        tmp[i] = vect[i] + _v.get_vect()[i];
    }
    return Vector<T, N>(tmp);
}

template <class T, size_t N>
Vector<T, N> Vector<T, N>::operator-(const Vector<T, N> &_v) const {
    T tmp[N];
    for (size_t i = 0; i < N; i++) {
        tmp[i] = vect[i] - _v.get_vect()[i];
    }
    return Vector<T, N>(tmp);
}

// template <class T, size_t N>
// Vector<T, N> & Vector<T, N>::operator++ () {
//     std::vector<T> tmp;
//     for(auto i:vect) {
//         tmp.push_back(i + 1);
//     }
//     vect = tmp;
//     return *this;
// }
//
// template <class T, size_t N>
// Vector<T, N> Vector<T, N>::operator++ (int) {
//     std::vector<T> tmp;
//     std::vector<T> res = vect;
//     for(auto i:vect) {
//         tmp.push_back(i + 1);
//     }
//     vect = tmp;
//     return res;
// }
//
// template <class T, size_t N>
// Vector<T, N> & Vector<T, N>::operator-- () {
//     std::vector<T> tmp;
//     for(auto i:vect) {
//         tmp.push_back(i - 1);
//     }
//     vect = tmp;
//     return *this;
// }
//
// template <class T, size_t N>
// Vector<T, N> Vector<T, N>::operator-- (int) {
//     std::vector<T> tmp;
//     std::vector<T> res = vect;
//     for(auto i:vect) {
//         tmp.push_back(i - 1);
//     }
//     vect = Vector<T, N>(tmp);
//     return res;
// }

template <class T, size_t N>
Vector<T, N> Vector<T, N>::operator*(T _t) const {
    T tmp[N];
    for (size_t i = 0; i < N; i++) {
        tmp[i] = vect[i] * _t;
    }
    return Vector<T, N>(tmp);
}

template <class T, size_t N>
T Vector<T, N>::operator*(const Vector<T, N> &_v) const {
    T res = 0;
    for (size_t i = 0; i < N; i++) {
        res += vect[i] * _v.get_vect()[i];
    }
    return res;
}

// 叉积只存在 N == 3 与 N == 7 的情况
template <class T, size_t N>
Vector<T, N> Vector<T, N>::operator^(const Vector<T, N> &_v) const {
    assert(N == 3 || N == 7);
    T tmp[N];
    // // 对于 N == 2，用 0 补全第 3 维
    // if(N == 2) {
    //     std::vector<T> plus_v = _v.get_vect();
    //     plus_v.push_back(0);
    //     std::vector<T> plus_vect = vect;
    //     plus_vect.push_back(0);
    //     tmp.push_back(plus_vect.at(1) * plus_v.at(2) - plus_vect.at(2) *
    //     plus_v.at(1) ); tmp.push_back(plus_vect.at(2) * plus_v.at(0) -
    //     plus_vect.at(0) * plus_v.at(2) );
    // }
    if (N == 3) {
        tmp[0] = vect[1] * _v.get_vect()[2] - vect[2] * _v.get_vect()[1];
        tmp[1] = vect[2] * _v.get_vect()[0] - vect[0] * _v.get_vect()[2];
        tmp[2] = vect[0] * _v.get_vect()[1] - vect[1] * _v.get_vect()[0];
    }
    return Vector<T, N>(tmp);
}

// 输出
template <class T, size_t N>
std::ostream &operator<<(std::ostream &_os, const Vector<T, N> &_v) {
    _os << "(";
    for (size_t i = 0; i < N; i++) {
        _os << _v.get_vect()[i];
        if (i == N - 1) {
            break;
        }
        _os << ", ";
    }
    _os << ")";
    return _os;
}

typedef Vector<int, 2>    Vectori2;
typedef Vector<int, 3>    Vectori3;
typedef Vector<double, 2> Vectord2;
typedef Vector<double, 3> Vectord3;

#endif /* __VECTOR_HPP__ */
