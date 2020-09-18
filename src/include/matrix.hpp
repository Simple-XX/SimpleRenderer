
// This file is a part of SimpleXX/SimpleRenderer
// (https://github.com/SimpleXX/SimpleRenderer).
//
// matrix.hpp for SimpleXX/SimpleRenderer.

#ifndef __MATRIX_HPP__
#define __MATRIX_HPP__

#include "vector"
#include "iomanip"

template <class T, size_t ROWS, size_t COLS>
class Matrix {
private:
    std::vector<std::vector<T>> mat;
    // 矩阵求行列式
    // 矩阵求余子式
public:
    Matrix(void);
    Matrix(const T *const _mat);
    Matrix(const std::vector<std::vector<T>> &_mat);
    Matrix(const Matrix<T, ROWS, COLS> &_matrix);
    ~Matrix(void);
    // 矩阵间加法
    Matrix<T, ROWS, COLS> operator+(const Matrix<T, ROWS, COLS> &_matrix) const;
    // 矩阵间自加
    Matrix<T, ROWS, COLS> &operator+=(const Matrix<T, ROWS, COLS> &_matrix);
    // 矩阵之间的减法运算
    Matrix<T, ROWS, COLS> operator-(const Matrix<T, ROWS, COLS> &_matrix) const;
    // 矩阵之间自减运算
    Matrix<T, ROWS, COLS> &operator-=(const Matrix<T, ROWS, COLS> &_matrix);
    // 矩阵与整数乘法
    Matrix<T, ROWS, COLS> operator*(const T _v);
    // 矩阵间乘法
    Matrix<T, ROWS, COLS> operator*(const Matrix<T, ROWS, COLS> &_matrix) const;
    // 矩阵与整数自乘
    Matrix<T, ROWS, COLS> &operator*=(const T _v);
    // 矩阵间自乘
    Matrix<T, ROWS, COLS> &operator*=(const Matrix<T, ROWS, COLS> &_matrix);
    // 矩阵相等
    bool operator==(const Matrix<T, ROWS, COLS> &_matrix);
    // 访问矩阵行/列
    std::vector<T> &operator[](const size_t _idx);
    // 矩阵转置
    Matrix<T, COLS, ROWS> transpose(void) const;
    // 矩阵求逆
    Matrix<T, ROWS, COLS> inverse(void) const;
    // _row 行赋值为 _v
    void set_row(const size_t _row, const T _v);
    // _col 列赋值为 _v
    void set_col(const size_t _col, const T _v);
    // 将矩阵全部赋值为 _v
    void set_all(const T _v);
    // 转换为数组
    size_t to_arr(T *_arr) const;
    // 转换为向量
    std::vector<std::vector<T>> to_vector(void) const;
};

template <class T, size_t ROWS, size_t COLS>
Matrix<T, ROWS, COLS>::Matrix(void)
    : mat(std::vector<std::vector<T>>(ROWS, std::vector<T>(COLS, 0))) {
    return;
}

template <class T, size_t ROWS, size_t COLS>
Matrix<T, ROWS, COLS>::Matrix(const T *const _mat)
    : mat(std::vector<std::vector<T>>(ROWS, std::vector<T>(COLS, 0))) {
    for (size_t i = 0; i < ROWS; i++) {
        for (size_t j = 0; j < COLS; j++) {
            mat.at(i).at(j) = _mat[i * COLS + j];
        }
    }
    return;
}

template <class T, size_t ROWS, size_t COLS>
Matrix<T, ROWS, COLS>::Matrix(const std::vector<std::vector<T>> &_mat)
    : mat(_mat) {
    return;
}

template <class T, size_t ROWS, size_t COLS>
Matrix<T, ROWS, COLS>::Matrix(const Matrix<T, ROWS, COLS> &_matrix)
    : mat(_matrix.mat) {
    return;
}

template <class T, size_t ROWS, size_t COLS>
Matrix<T, ROWS, COLS>::~Matrix(void) {
    return;
}

template <class T, size_t ROWS, size_t COLS>
Matrix<T, ROWS, COLS>
Matrix<T, ROWS, COLS>::operator+(const Matrix<T, ROWS, COLS> &_matrix) const {
    std::vector<std::vector<T>> tmp(ROWS, std::vector<T>(COLS, 0));
    for (size_t i = 0; i < ROWS; i++) {
        for (size_t j = 0; j < COLS; j++) {
            tmp.at(i).at(j) = mat.at(i).at(j) + _matrix.to_vector().at(i).at(j);
        }
    }
    return Matrix<T, ROWS, COLS>(tmp);
}

template <class T, size_t ROWS, size_t COLS>
Matrix<T, ROWS, COLS> &
Matrix<T, ROWS, COLS>::operator+=(const Matrix<T, ROWS, COLS> &_matrix) {
    for (size_t i = 0; i < ROWS; i++) {
        for (size_t j = 0; j < COLS; j++) {
            mat.at(i).at(j) += _matrix.to_vector().at(i).at(j);
        }
    }
    return *this;
}

template <class T, size_t ROWS, size_t COLS>
Matrix<T, ROWS, COLS>
Matrix<T, ROWS, COLS>::operator-(const Matrix<T, ROWS, COLS> &_matrix) const {
    std::vector<std::vector<T>> tmp(ROWS, std::vector<T>(COLS, 0));
    for (size_t i = 0; i < ROWS; i++) {
        for (size_t j = 0; j < COLS; j++) {
            tmp.at(i).at(j) = mat.at(i).at(j) - _matrix.to_vector().at(i).at(j);
        }
    }
    return Matrix<T, ROWS, COLS>(tmp);
}

template <class T, size_t ROWS, size_t COLS>
Matrix<T, ROWS, COLS> &
Matrix<T, ROWS, COLS>::operator-=(const Matrix<T, ROWS, COLS> &_matrix) {
    for (size_t i = 0; i < ROWS; i++) {
        for (size_t j = 0; j < COLS; j++) {
            mat.at(i).at(j) -= _matrix.to_vector().at(i).at(j);
        }
    }
    return *this;
}

template <class T, size_t ROWS, size_t COLS>
Matrix<T, ROWS, COLS> Matrix<T, ROWS, COLS>::operator*(const T _v) {
    std::vector<std::vector<T>> tmp(ROWS, std::vector<T>(COLS, 0));
    for (size_t i = 0; i < ROWS; i++) {
        for (size_t j = 0; j < COLS; j++) {
            tmp.at(i).at(j) = mat.at(i).at(j) * _v;
        }
    }
    return Matrix<T, ROWS, COLS>(tmp);
}

template <class T, size_t ROWS, size_t COLS>
Matrix<T, ROWS, COLS>
Matrix<T, ROWS, COLS>::operator*(const Matrix<T, ROWS, COLS> &_matrix) const {
    std::vector<std::vector<T>> tmp(ROWS, std::vector<T>(COLS, 0));
    return Matrix<T, ROWS, COLS>(tmp);
}

template <class T, size_t ROWS, size_t COLS>
Matrix<T, ROWS, COLS> &Matrix<T, ROWS, COLS>::operator*=(const T _v) {
    for (size_t i = 0; i < ROWS; i++) {
        for (size_t j = 0; j < COLS; j++) {
            mat.at(i).at(j) *= _v;
        }
    }
    return *this;
}

template <class T, size_t ROWS, size_t COLS>
Matrix<T, ROWS, COLS> &
Matrix<T, ROWS, COLS>::operator*=(const Matrix<T, ROWS, COLS> &_matrix) {
    for (size_t i = 0; i < ROWS; i++) {
        for (size_t j = 0; j < COLS; j++) {
            mat.at(i).at(j) *= _matrix.to_vector().at(i).at(j);
        }
    }
    return *this;
}

template <class T, size_t ROWS, size_t COLS>
bool Matrix<T, ROWS, COLS>::operator==(const Matrix<T, ROWS, COLS> &_matrix) {
    bool res = true;
    for (size_t i = 0; i < ROWS; i++) {
        for (size_t j = 0; j < COLS; j++) {
            if (mat.at(i).at(j) != _matrix.to_vector().at(i).at(j)) {
                res = false;
                break;
            }
        }
    }
    return res;
}

template <class T, size_t ROWS, size_t COLS>
std::vector<T> &Matrix<T, ROWS, COLS>::operator[](const size_t _idx) {
    assert(_idx >= 0 && _idx < ROWS);
    return mat.at(_idx);
}

template <class T, size_t ROWS, size_t COLS>
Matrix<T, COLS, ROWS> Matrix<T, ROWS, COLS>::transpose(void) const {
    std::vector<std::vector<T>> tmp(COLS, std::vector<T>(ROWS, 0));
    for (size_t i = 0; i < ROWS; i++) {
        for (size_t j = 0; j < COLS; j++) {
            tmp.at(j).at(i) = to_vector().at(i).at(j);
        }
    }
    return Matrix<T, COLS, ROWS>(tmp);
}

template <class T, size_t ROWS, size_t COLS>
Matrix<T, ROWS, COLS> Matrix<T, ROWS, COLS>::inverse(void) const {
    std::vector<std::vector<T>> tmp;
    return Matrix<T, COLS, ROWS>(tmp);
}

template <class T, size_t ROWS, size_t COLS>
void Matrix<T, ROWS, COLS>::set_row(const size_t _row, const T _v) {
}

template <class T, size_t ROWS, size_t COLS>
void Matrix<T, ROWS, COLS>::set_col(const size_t _col, const T _v) {
}

template <class T, size_t ROWS, size_t COLS>
void Matrix<T, ROWS, COLS>::set_all(const T _v) {
}

template <class T, size_t ROWS, size_t COLS>
size_t Matrix<T, ROWS, COLS>::to_arr(T *_arr) const {
    for (size_t i = 0; i < ROWS; i++) {
        for (size_t j = 0; j < COLS; j++) {
            _arr[i * COLS + j] = to_vector().at(i).at(j);
        }
    }
    return ROWS * COLS;
}

template <class T, size_t ROWS, size_t COLS>
std::vector<std::vector<T>> Matrix<T, ROWS, COLS>::to_vector(void) const {
    return mat;
}

// 输出
template <class T, size_t ROWS, size_t COLS>
std::ostream &operator<<(std::ostream &_os, const Matrix<T, ROWS, COLS> &_mat) {
    _os << "[";
    for (size_t i = 0; i < ROWS; i++) {
        if (i != 0) {
            _os << "\n";
            _os << " ";
        }
        for (size_t j = 0; j < COLS; j++) {
            _os << std::setw(4) << _mat.to_vector().at(i).at(j);
            if (j != COLS - 1) {
                _os << " ";
            }
        }
    }
    _os << std::setw(4) << "]";
    return _os;
}

#endif /* __MATRIX_HPP__ */
