
// This file is a part of SimpleXX/SimpleRenderer
// (https://github.com/SimpleXX/SimpleRenderer).
//
// matrix.hpp for SimpleXX/SimpleRenderer.

#ifndef __MATRIX_HPP__
#define __MATRIX_HPP__

#include "vector"
#include "iomanip"

template <class T>
class Matrix {
private:
    std::vector<std::vector<T>> mat;
    size_t                      rows;
    size_t                      cols;
    // 矩阵求行列式
    // 矩阵求余子式
public:
    Matrix(size_t _rows = 4, size_t _cols = 4);
    Matrix(size_t _rows, size_t _cols, const T *const _mat);
    Matrix(const std::vector<std::vector<T>> &_mat);
    Matrix(const Matrix<T> &_matrix);
    ~Matrix(void);
    // 矩阵间加法
    Matrix<T> operator+(const Matrix<T> &_matrix) const;
    // 矩阵间自加
    Matrix<T> &operator+=(const Matrix<T> &_matrix);
    // 矩阵之间的减法运算
    Matrix<T> operator-(const Matrix<T> &_matrix) const;
    // 矩阵之间自减运算
    Matrix<T> &operator-=(const Matrix<T> &_matrix);
    // 矩阵与整数乘法
    Matrix<T> operator*(const T _v);
    // 矩阵间乘法
    Matrix<T> operator*(const Matrix<T> &_matrix) const;
    // 矩阵与整数自乘
    Matrix<T> &operator*=(const T _v);
    // 矩阵间自乘
    Matrix<T> &operator*=(const Matrix<T> &_matrix);
    // 矩阵相等
    bool operator==(const Matrix<T> &_matrix) const;
    // 访问矩阵行/列
    std::vector<T> &operator[](const size_t _idx);
    // 获取行数
    size_t get_rows(void) const;
    // 获取列数
    size_t get_cols(void) const;
    // 矩阵转置
    Matrix<T> transpose(void) const;
    // 矩阵求逆
    Matrix<T> inverse(void) const;
    // 转换为数组
    size_t to_arr(T *_arr) const;
    // 转换为向量
    std::vector<std::vector<T>> to_vector(void) const;
};

template <class T>
Matrix<T>::Matrix(size_t _rows, size_t _cols)
    : mat(std::vector<std::vector<T>>(_rows, std::vector<T>(_cols, 0))),
      rows(_rows), cols(_cols) {
    return;
}

template <class T>
Matrix<T>::Matrix(size_t _rows, size_t _cols, const T *const _mat)
    : mat(std::vector<std::vector<T>>(_rows, std::vector<T>(_cols, 0))),
      rows(_rows), cols(_cols) {
    for (size_t i = 0; i < rows; i++) {
        for (size_t j = 0; j < cols; j++) {
            mat.at(i).at(j) = _mat[i * cols + j];
        }
    }
    return;
}

template <class T>
Matrix<T>::Matrix(const std::vector<std::vector<T>> &_mat)
    : mat(_mat), rows(_mat.size()), cols(_mat.at(0).size()) {
    return;
}

template <class T>
Matrix<T>::Matrix(const Matrix<T> &_matrix)
    : mat(_matrix.mat), rows(_matrix.rows), cols(_matrix.cols) {
    return;
}

template <class T>
Matrix<T>::~Matrix(void) {
    return;
}

template <class T>
Matrix<T> Matrix<T>::operator+(const Matrix<T> &_matrix) const {
    std::vector<std::vector<T>> tmp(rows, std::vector<T>(cols, 0));
    for (size_t i = 0; i < rows; i++) {
        for (size_t j = 0; j < cols; j++) {
            tmp.at(i).at(j) = mat.at(i).at(j) + _matrix.to_vector().at(i).at(j);
        }
    }
    return Matrix<T>(tmp);
}

template <class T>
Matrix<T> &Matrix<T>::operator+=(const Matrix<T> &_matrix) {
    for (size_t i = 0; i < rows; i++) {
        for (size_t j = 0; j < cols; j++) {
            mat.at(i).at(j) += _matrix.to_vector().at(i).at(j);
        }
    }
    return *this;
}

template <class T>
Matrix<T> Matrix<T>::operator-(const Matrix<T> &_matrix) const {
    std::vector<std::vector<T>> tmp(rows, std::vector<T>(cols, 0));
    for (size_t i = 0; i < rows; i++) {
        for (size_t j = 0; j < cols; j++) {
            tmp.at(i).at(j) = mat.at(i).at(j) - _matrix.to_vector().at(i).at(j);
        }
    }
    return Matrix<T>(tmp);
}

template <class T>
Matrix<T> &Matrix<T>::operator-=(const Matrix<T> &_matrix) {
    for (size_t i = 0; i < rows; i++) {
        for (size_t j = 0; j < cols; j++) {
            mat.at(i).at(j) -= _matrix.to_vector().at(i).at(j);
        }
    }
    return *this;
}

template <class T>
Matrix<T> Matrix<T>::operator*(const T _v) {
    std::vector<std::vector<T>> tmp(rows, std::vector<T>(cols, 0));
    for (size_t i = 0; i < rows; i++) {
        for (size_t j = 0; j < cols; j++) {
            tmp.at(i).at(j) = mat.at(i).at(j) * _v;
        }
    }
    return Matrix<T>(tmp);
}

template <class T>
Matrix<T> &Matrix<T>::operator*=(const T _v) {
    for (size_t i = 0; i < rows; i++) {
        for (size_t j = 0; j < cols; j++) {
            mat.at(i).at(j) *= _v;
        }
    }
    return *this;
}

template <class T>
Matrix<T> Matrix<T>::operator*(const Matrix<T> &_matrix) const {
    assert(cols == _matrix.rows);
    std::vector<std::vector<T>> tmp(rows, std::vector<T>(_matrix.cols, 0));
    for (size_t i = 0; i < rows; i++) {
        for (size_t j = 0; j < _matrix.cols; j++) {
            for (size_t k = 0; k < cols; k++) {
                tmp.at(i).at(j) += mat.at(i).at(k) * _matrix.mat.at(k).at(j);
            }
        }
    }
    return Matrix<T>(tmp);
}

template <class T>
Matrix<T> &Matrix<T>::operator*=(const Matrix<T> &_matrix) {
    assert(cols == _matrix.rows);
    std::vector<std::vector<T>> tmp(rows, std::vector<T>(_matrix.cols, 0));
    for (size_t i = 0; i < rows; i++) {
        for (size_t j = 0; j < _matrix.cols; j++) {
            for (size_t k = 0; k < cols; k++) {
                tmp.at(i).at(j) += mat.at(i).at(k) * _matrix.mat.at(k).at(j);
            }
        }
    }
    mat  = tmp;
    cols = _matrix.cols;
    return *this;
}

template <class T>
bool Matrix<T>::operator==(const Matrix<T> &_matrix) const {
    bool res = true;
    for (size_t i = 0; i < rows; i++) {
        for (size_t j = 0; j < cols; j++) {
            if (_matrix.get_rows() != rows || _matrix.get_cols() != cols) {
                res = false;
                break;
            }
            if (mat.at(i).at(j) != _matrix.to_vector().at(i).at(j)) {
                res = false;
                break;
            }
        }
    }
    return res;
}

template <class T>
std::vector<T> &Matrix<T>::operator[](const size_t _idx) {
    assert(_idx >= 0 && _idx < rows);
    return mat.at(_idx);
}

template <class T>
size_t Matrix<T>::get_rows(void) const {
    return rows;
}

template <class T>
size_t Matrix<T>::get_cols(void) const {
    return cols;
}

template <class T>
Matrix<T> Matrix<T>::transpose(void) const {
    std::vector<std::vector<T>> tmp(cols, std::vector<T>(rows, 0));
    for (size_t i = 0; i < rows; i++) {
        for (size_t j = 0; j < cols; j++) {
            tmp.at(j).at(i) = to_vector().at(i).at(j);
        }
    }
    return Matrix<T>(tmp);
}

template <class T>
Matrix<T> Matrix<T>::inverse(void) const {
    std::vector<std::vector<T>> tmp;
    return Matrix<T>(tmp);
}

template <class T>
size_t Matrix<T>::to_arr(T *_arr) const {
    for (size_t i = 0; i < rows; i++) {
        for (size_t j = 0; j < cols; j++) {
            _arr[i * cols + j] = to_vector().at(i).at(j);
        }
    }
    return rows * cols;
}

template <class T>
std::vector<std::vector<T>> Matrix<T>::to_vector(void) const {
    return mat;
}

// 输出
template <class T>
std::ostream &operator<<(std::ostream &_os, const Matrix<T> &_mat) {
    _os << "[";
    for (size_t i = 0; i < _mat.get_rows(); i++) {
        if (i != 0) {
            _os << "\n";
            _os << " ";
        }
        for (size_t j = 0; j < _mat.get_cols(); j++) {
            _os << std::setw(4) << _mat.to_vector().at(i).at(j);
            if (j != _mat.get_cols() - 1) {
                _os << " ";
            }
        }
    }
    _os << std::setw(4) << "]";
    return _os;
}

#endif /* __MATRIX_HPP__ */
