
// This file is a part of SimpleXX/SimpleRenderer (https://github.com/SimpleXX/SimpleRenderer).
//
// Vector.hpp for SimpleXX/SimpleRenderer.

#ifndef __VECTOR_HPP__
#define __VECTOR_HPP__

#include "vector"
#include "iostream"
#include "math.h"
#include "stdarg.h"
#include "assert.h"

template <class T, size_t N = 3>
class Vector {
private:
    std::vector<T> vect;
    // 范数
    double norm;
    // 单位向量
    std::vector<double> unit;

public:
    Vector(void);
    Vector(std::vector<T> _vect);
    ~Vector(void);
    // 获取 vect
    std::vector<T> get_vect(void) const;
    // 向量相等
    bool operator== (const Vector<T, N> &_v) const;
    // 向量不为零向量
    operator bool (void) const;
    // 向量取反
    Vector<T, N> operator- (void) const;
    // 范数
    double get_norm(void) const;
    // 单位向量
    std::vector<double> get_unit(void) const;
    // 向量和
    Vector<T, N> operator+ (const Vector<T, N> &_v) const;
    // 向量差
    Vector<T, N> operator- (const Vector<T, N> &_v) const;
    // 向量数乘
    Vector<T, N> operator* (T _t) const;
    // 点积
    T operator* (const Vector<T, N> &_v) const;
    // 叉积
    Vector<T, N> operator^ (const Vector<T, N> &_v) const;
    // 向量投影
};

template <class T, size_t N>
Vector<T, N>::Vector(void) {
    for(size_t i = 0 ; i < N ; i++) {
        vect.push_back(0);
        unit.push_back(0);
    }
    norm = 0;
    return;
}

template <class T, size_t N>
Vector<T, N>::Vector(std::vector<T> _vect) {
    assert(_vect.size() == N);
    vect = _vect;
    double square_sum = 0;
    for(auto i:vect) {
        square_sum += i * i;
    }
    norm = pow(square_sum, (1.0 / N) );
    for(auto i:vect) {
        unit.push_back(i / norm);
    }
    return;
}

template <class T, size_t N>
Vector<T, N>::~Vector(void) {
    return;
}

template <class T, size_t N>
std::vector<T> Vector<T, N>::get_vect(void) const {
    return vect;
}

template <class T, size_t N>
bool Vector<T, N>::operator== (const Vector<T, N> &_v) const {
    bool res = true;
    for(size_t i = 0 ; i < N ; i++) {
        if(vect.at(i) != (_v.get_vect() ).at(i) ) {
            res = false;
        }
    }
    return res;
}

template <class T, size_t N>
Vector<T, N>::operator bool (void) const {
    bool res = false;
    for(auto i:vect) {
        if(i != 0) {
            res = true;
        }
    }
    return res;
}

template <class T, size_t N>
Vector<T, N> Vector<T, N>::operator- (void) const {
    std::vector<T> tmp;
    for(auto &i:vect) {
        tmp.push_back(-i);
    }
    return Vector<T, N>(tmp);
}

template <class T, size_t N>
double Vector<T, N>::get_norm(void) const {
    return norm;
}

template <class T, size_t N>
std::vector<double> Vector<T, N>::get_unit(void) const {
    return unit;
}

template <class T, size_t N>
Vector<T, N> Vector<T, N>::operator+ (const Vector<T, N> &_v) const {
    assert(vect.size() == _v.size() );
    std::vector<T> tmp;
    for(size_t i = 0 ; i < vect.size() ; i++) {
        tmp.push_back(vect.at(i) + _v.at(i) );
    }
    return Vector<T, N>(tmp);
}

template <class T, size_t N>
Vector<T, N> Vector<T, N>::operator- (const Vector<T, N> &_v) const {
    assert(vect.size() == (_v.get_vect() ).size() );
    std::vector<T> tmp;
    for(size_t i = 0 ; i < N ; i++) {
        tmp.push_back(vect.at(i) - (_v.get_vect() ).at(i) );
    }
    return Vector<T, N>(tmp);
}

template <class T, size_t N>
Vector<T, N> Vector<T, N>::operator* (T _t) const {
    std::vector<T> tmp;
    for(auto i:vect) {
        tmp.push_back(i * _t);
    }
    return Vector<T, N>(tmp);
}

template <class T, size_t N>
T Vector<T, N>::operator* (const Vector<T, N> &_v) const {
    T res = 0;
    for(size_t i = 0 ; i < N ; i++) {
        res += vect.at(i) * ( (_v.get_vect() ) ).at(i);
    }
    return res;
}

// 叉积只存在 N == 3 与 N == 7 的情况
template <class T, size_t N>
Vector<T, N> Vector<T, N>::operator^ (const Vector<T, N> &_v) const {
    assert(N == 3 || N == 7);
    std::vector<T> tmp;
    // // 对于 N == 2，用 0 补全第 3 维
    // if(N == 2) {
    //     std::vector<T> plus_v = _v.get_vect();
    //     plus_v.push_back(0);
    //     std::vector<T> plus_vect = vect;
    //     plus_vect.push_back(0);
    //     tmp.push_back(plus_vect.at(1) * plus_v.at(2) - plus_vect.at(2) * plus_v.at(1) );
    //     tmp.push_back(plus_vect.at(2) * plus_v.at(0) - plus_vect.at(0) * plus_v.at(2) );
    // }
    if(N == 3) {
        tmp.push_back(vect.at(1) * (_v.get_vect() ).at(2) - vect.at(2) * (_v.get_vect() ).at(1) );
        tmp.push_back(vect.at(2) * (_v.get_vect() ).at(0) - vect.at(0) * (_v.get_vect() ).at(2) );
        tmp.push_back(vect.at(0) * (_v.get_vect() ).at(1) - vect.at(1) * (_v.get_vect() ).at(0) );
    }
    return Vector<T, N>(tmp);
}

// 输出
template <class T, size_t N>
std::ostream & operator<< (std::ostream &_os, const Vector<T, N> &_v) {
    _os << "(";
    for(size_t i = 0 ; i < N ; i++) {
        _os << (_v.get_vect() ).at(i);
        if(i == N - 1) {
            break;
        }
        _os << ", ";
    }
    _os << ")";
    return _os;
}

typedef Vector<int, 2> Vectori2;
typedef Vector<int, 3> Vectori3;
typedef Vector<double, 2> Vectord2;
typedef Vector<double, 3> Vectord3;

#endif /* __VECTOR_HPP__ */
