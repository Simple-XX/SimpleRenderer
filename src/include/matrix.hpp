
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
    T det(void) const;
    // 矩阵求余子式
    T minor(const size_t _r, const size_t _c) const;
    // 矩阵求代数余子式
    T cofactor(const size_t _r, const size_t _c) const;

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
    Matrix<double> inverse(void) const;
    // 转换为数组
    size_t to_arr(T *_arr) const;
    // 转换为向量
    std::vector<std::vector<T>> to_vector(void) const;
    // PLU 分解，返回分解好的矩阵，参数用于获取主元表
    Matrix<double> PLU(std::vector<size_t> &_p);
    // 矩阵求余子式矩阵
    Matrix<T> minor(void) const;
    // 矩阵求代数余子式矩阵
    Matrix<T> cofactor(void) const;
    // 矩阵求伴随矩阵
    Matrix<T> adjugate(void) const;
};

template <class T>
Matrix<double> Matrix<T>::PLU(std::vector<size_t> &_p) {
    std::vector<std::vector<double>> tmp(rows, std::vector<double>(cols, 0));
    // 转换为 double 类型
    for (size_t i = 0; i < rows; i++) {
        for (size_t j = 0; j < cols; j++) {
            tmp.at(i).at(j) = (double)mat.at(i).at(j);
        }
    }

    // 对于非方阵，选取较小的
    size_t n = std::min(rows, cols);
    // 初始化置换矩阵 _p
    _p = std::vector<size_t>(n, 0);
    for (size_t i = 0; i < n; i++) {
        _p.at(i) = i;
    }

    // 如果主元所在列的绝对值最大值不是当前行，进行行交换
    for (size_t i = 0; i < n; i++) {
        T imax = 0;
        for (size_t j = i; j < n; j++) {
            if (imax < std::abs(tmp.at(j).at(i))) {
                imax = std::abs(tmp.at(j).at(i));
                // 交换行
                if (i != j) {
                    std::swap(tmp.at(i), tmp.at(j));
                    // 记录 _p
                    _p.at(i) = j;
                    _p.at(j) = i;
                }
            }
        }
    }
    // 选主元完成
    // L
    // for (size_t i = n - 1; i >= 0; i--) {
    //     // 变为单位下三角矩阵
    //     // ii 归一化
    //     if (tmp.at(i).at(i) != 1) {
    //         T ii = tmp.at(i).at(i);
    //         for (size_t j = n - 1; j >= 0; j--) {
    //             assert(j < 0);
    //             tmp.at(i).at(j) /= ii;
    //         }
    //     }
    //     return Matrix<double>(tmp);
    // }

    // U
    for (size_t i = 0; i < n; i++) {
        // 变为上三角矩阵
        // 首先确定主元
        double ii = tmp.at(i).at(i);
        // 下面一行-主元行*(行列/主元)
        for (size_t j = i + 1; j < n; j++) {
            for (size_t k = 0; k < n; k++) {
                double scale = tmp.at(j).at(k) / ii;
                tmp.at(j).at(k) -= tmp.at(i).at(k) * scale;
            }
        }

        // return Matrix<double>(tmp);
    }

    return Matrix<double>(tmp);
}

template <class T>
T Matrix<T>::det(void) const {
    assert(rows == cols);
    assert(rows != 0);
    T res = 0;
    if (rows == 1) {
        res = mat.at(0).at(0);
    }
    else if (rows == 2) {
        res = mat.at(0).at(0) * mat.at(1).at(1) -
              mat.at(0).at(1) * mat.at(1).at(0);
    }
    else if (rows == 3) {
        res = mat.at(0).at(0) * mat.at(1).at(1) * mat.at(2).at(2) +
              mat.at(1).at(0) * mat.at(2).at(1) * mat.at(0).at(2) +
              mat.at(2).at(0) * mat.at(0).at(1) * mat.at(1).at(2) -
              mat.at(1).at(0) * mat.at(0).at(1) * mat.at(2).at(2) -
              mat.at(2).at(0) * mat.at(1).at(1) * mat.at(0).at(2) -
              mat.at(0).at(0) * mat.at(2).at(1) * mat.at(1).at(2);
    }
    else if (rows == 4) {
        res = mat.at(0).at(0) * mat.at(1).at(1) * mat.at(2).at(2) *
                  mat.at(3).at(3) +
              mat.at(0).at(0) * mat.at(2).at(1) * mat.at(3).at(2) *
                  mat.at(1).at(3) +
              mat.at(0).at(0) * mat.at(3).at(1) * mat.at(1).at(2) *
                  mat.at(2).at(3) +

              mat.at(1).at(0) * mat.at(0).at(1) * mat.at(3).at(2) *
                  mat.at(2).at(3) +
              mat.at(1).at(0) * mat.at(3).at(1) * mat.at(2).at(2) *
                  mat.at(0).at(3) +
              mat.at(1).at(0) * mat.at(2).at(1) * mat.at(0).at(2) *
                  mat.at(3).at(3) +

              mat.at(2).at(0) * mat.at(3).at(1) * mat.at(0).at(2) *
                  mat.at(1).at(3) +
              mat.at(2).at(0) * mat.at(0).at(1) * mat.at(1).at(2) *
                  mat.at(3).at(3) +
              mat.at(2).at(0) * mat.at(1).at(1) * mat.at(3).at(2) *
                  mat.at(0).at(3) +

              mat.at(3).at(0) * mat.at(2).at(1) * mat.at(1).at(2) *
                  mat.at(0).at(3) +
              mat.at(3).at(0) * mat.at(1).at(1) * mat.at(0).at(2) *
                  mat.at(2).at(3) +
              mat.at(3).at(0) * mat.at(0).at(1) * mat.at(2).at(2) *
                  mat.at(1).at(3) -

              mat.at(0).at(0) * mat.at(1).at(1) * mat.at(3).at(2) *
                  mat.at(2).at(3) -
              mat.at(0).at(0) * mat.at(2).at(1) * mat.at(1).at(2) *
                  mat.at(3).at(3) -
              mat.at(0).at(0) * mat.at(3).at(1) * mat.at(2).at(2) *
                  mat.at(1).at(3) -

              mat.at(1).at(0) * mat.at(0).at(1) * mat.at(2).at(2) *
                  mat.at(3).at(3) -
              mat.at(1).at(0) * mat.at(3).at(1) * mat.at(0).at(2) *
                  mat.at(2).at(3) -
              mat.at(1).at(0) * mat.at(2).at(1) * mat.at(3).at(2) *
                  mat.at(0).at(3) -

              mat.at(2).at(0) * mat.at(3).at(1) * mat.at(1).at(2) *
                  mat.at(0).at(3) -
              mat.at(2).at(0) * mat.at(0).at(1) * mat.at(3).at(2) *
                  mat.at(1).at(3) -
              mat.at(2).at(0) * mat.at(1).at(1) * mat.at(0).at(2) *
                  mat.at(3).at(3) -

              mat.at(3).at(0) * mat.at(2).at(1) * mat.at(0).at(2) *
                  mat.at(1).at(3) -
              mat.at(3).at(0) * mat.at(1).at(1) * mat.at(2).at(2) *
                  mat.at(0).at(3) -
              mat.at(3).at(0) * mat.at(0).at(1) * mat.at(1).at(2) *
                  mat.at(2).at(3);
    }
    return res;
}

template <class T>
T Matrix<T>::minor(const size_t _r, const size_t _c) const {
    std::vector<std::vector<T>> tmp(rows - 1, std::vector<T>(cols - 1, 0));
    for (size_t i = 0; i < rows - 1; i++) {
        for (size_t j = 0; j < cols - 1; j++) {
            size_t idx_r = i;
            size_t idx_c = j;
            if (i >= _r) {
                idx_r += 1;
            }
            if (j >= _c) {
                idx_c += 1;
            }
            tmp.at(i).at(j) = mat.at(idx_r).at(idx_c);
        }
    }
    // 计算行列式 tmp 的值
    T res = Matrix<T>(tmp).det();
    return res;
}

template <class T>
Matrix<T> Matrix<T>::minor(void) const {
    std::vector<std::vector<T>> tmp(rows, std::vector<T>(cols, 0));
    for (size_t i = 0; i < rows; i++) {
        for (size_t j = 0; j < cols; j++) {
            tmp.at(i).at(j) = minor(i, j);
        }
    }
    return Matrix<T>(tmp);
}

template <class T>
T Matrix<T>::cofactor(const size_t _r, const size_t _c) const {
    assert(std::is_signed<T>::value);
    return minor(_r, _c) * ((_r + _c) % 2 ? -1 : 1);
}

template <class T>
Matrix<T> Matrix<T>::cofactor(void) const {
    std::vector<std::vector<T>> tmp(rows, std::vector<T>(cols, 0));
    for (size_t i = 0; i < rows; i++) {
        for (size_t j = 0; j < cols; j++) {
            tmp.at(i).at(j) = cofactor(i, j);
        }
    }
    return Matrix<T>(tmp);
}

template <class T>
Matrix<T> Matrix<T>::adjugate(void) const {
    return cofactor().transpose();
}

template <class T>
Matrix<T>::Matrix(size_t _rows, size_t _cols)
    : mat(std::vector<std::vector<T>>(_rows, std::vector<T>(_cols, 0))),
      rows(_rows), cols(_cols) {
    for (size_t i = 0; i < rows; i++) {
        mat.at(i).at(i) = 1;
    }
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

// TODO: 换成 LU 分解法
template <class T>
Matrix<double> Matrix<T>::inverse(void) const {
    assert(rows == cols);
    std::vector<std::vector<double>> tmp(rows, std::vector<double>(cols, 0));
    T                                d = det();
    if (d == 0) {
        return Matrix<double>(tmp);
    }
    Matrix<T> adj = adjugate();
    for (size_t i = 0; i < rows; i++) {
        for (size_t j = 0; j < cols; j++) {
            tmp.at(i).at(j) = adj.mat.at(i).at(j) * (1. / d);
        }
    }
    return Matrix<double>(tmp);
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
            _os << std::setw(7) << std::setprecision(4)
                << _mat.to_vector().at(i).at(j);
            if (j != _mat.get_cols() - 1) {
                _os << " ";
            }
        }
    }
    _os << std::setw(4) << "]";
    return _os;
}

typedef Matrix<double> Matrix4;

#endif /* __MATRIX_HPP__ */
