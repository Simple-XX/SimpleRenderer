
/**
 * @file matrix.hpp
 * @brief 矩阵计算
 * @author Zone.N (Zone.Niuzh@hotmail.com)
 * @version 1.0
 * @date 2022-09-07
 * @copyright MIT LICENSE
 * https://github.com/Simple-XX/SimpleRenderer
 * @par change log:
 * <table>
 * <tr><th>Date<th>Author<th>Description
 * <tr><td>2022-09-07<td>Zone.N<td>迁移到 doxygen
 * </table>
 */

#ifndef _MATRIX_HPP_
#define _MATRIX_HPP_

#include "iostream"
#include "iomanip"
#include "vector.hpp"

/**
 * @brief 四阶矩阵
 * @tparam _T 矩阵元素类型
 */
template <class _T>
class matrix_t {
private:
    /// @brief  阶数
    static constexpr const uint32_t ORDER = 4;
    /// @brief 矩阵元素
    _T mat[ORDER][ORDER];

    /**
     * @brief 矩阵行列式的值
     * @return _T               行列式的值
     */
    _T determ(void) const;

    /**
     * @brief 矩阵的余子式
     * @param  _r               第几行
     * @param  _c               第几列
     * @return _T               结果
     */
    _T minor(const uint32_t _r, const uint32_t _c) const;

    /**
     * @brief 代数余子式
     * @param  _r               第几行
     * @param  _c               第几列
     * @return _T               结果
     */
    _T cofactor(const uint32_t _r, const uint32_t _c) const;

public:
    /**
     * @brief 构造函数，默认为单位矩阵
     */
    matrix_t(void);

    /**
     * @brief 构造函数
     * @param  _mat             另一个矩阵
     */
    matrix_t(const matrix_t<_T> &_mat);

    /**
     * @brief 构造函数
     * @param  _arr             指针
     */
    matrix_t(const _T *const _arr);

    /**
     * @brief 构造函数
     * @param  _arr             数组
     */
    matrix_t(const _T _arr[ORDER][ORDER]);

    /**
     * @brief 构造函数，构造齐次坐标，多余位置补 0
     * @param  _v               二维向量
     */
    matrix_t(const vector2_t<_T> &_v);

    /**
     * @brief 构造函数，构造齐次坐标，多余位置补 0
     * @param  _v               三维向量
     */
    matrix_t(const vector3_t<_T> &_v);

    /**
     * @brief 赋值
     * @param  _mat             另一个 matrix_t
     * @return matrix_t<_T>&    结果
     */
    matrix_t<_T> &operator=(const matrix_t<_T> &_mat);

    /**
     * @brief 矩阵间加法
     * @param  _mat             另一个 matrix_t
     * @return const matrix_t<_T>   结果
     */
    const matrix_t<_T> operator+(const matrix_t<_T> &_mat) const;

    /**
     * @brief 矩阵自加
     * @param  _mat             另一个 matrix_t
     * @return matrix_t<_T>&    结果
     */
    matrix_t<_T> &operator+=(const matrix_t<_T> &_mat);

    /**
     * @brief 矩阵间减法
     * @param  _mat             另一个 matrix_t
     * @return const matrix_t<_T>   结果
     */
    const matrix_t<_T> operator-(const matrix_t<_T> &_mat) const;

    /**
     * @brief 矩阵自减
     * @param  _mat             另一个 matrix_t
     * @return matrix_t<_T>&    结果
     */
    matrix_t<_T> &operator-=(const matrix_t<_T> &_mat);

    /**
     * @brief 矩阵数乘
     * @param  _v               数
     * @return const matrix_t<_T>   结果
     */
    const matrix_t<_T> operator*(const _T &_v) const;

    /**
     * @brief 矩阵间乘法
     * @param  _mat             另一个 matrix_t
     * @return const matrix_t<_T>   结果
     */
    const matrix_t<_T> operator*(const matrix_t<_T> &_mat) const;

    /**
     * @brief 矩阵数乘的自乘
     * @param  _v               数
     * @return matrix_t<_T>&    结果
     */
    matrix_t<_T> &operator*=(const _T &_v);

    /**
     * @brief 矩阵间自乘
     * @param  _mat             另一个 matrix_t
     * @return matrix_t<_T>&    结果
     */
    matrix_t<_T> &operator*=(const matrix_t<_T> &_mat);

    /**
     * @brief 矩阵与二维向量乘法
     * @param  _v               向量
     * @return const vector2_t<_T>  结果
     * @note    只计算二维
     */
    const vector2_t<_T> operator*(const vector2_t<_T> &_v) const;

    /**
     * @brief 矩阵与三维向量乘法
     * @param  _v               向量
     * @return const vector3_t<_T>  结果
     * @note    只计算三维
     */
    const vector3_t<_T> operator*(const vector3_t<_T> &_v) const;

    /**
     * @brief 矩阵相等
     * @param  _mat             另一个 matrix_t
     * @return true             相等
     * @return false            不等
     */
    bool operator==(const matrix_t<_T> &_mat) const;
    /**
     * @brief 矩阵不等
     * @param  _mat             另一个 matrix_t
     * @return true             不等
     * @return false            相等
     */
    bool operator!=(const matrix_t<_T> &_mat) const;

    /**
     * @brief 下标重载
     * @param  _idx             行
     * @return _T*              行指针
     * @note    注意不要越界访问
     */
    _T *operator[](const uint32_t _idx);

    /**
     * @brief 下标重载
     * @param  _idx             行
     * @return const _T*        行指针
     * @note    注意不要越界访问
     */
    const _T *operator[](const uint32_t _idx) const;

    /**
     * @brief 获取矩阵阶数
     * @return uint32_t         阶数
     */
    uint32_t get_order(void) const;

    /**
     * @brief 矩阵转置
     * @return const matrix_t<_T>   转置矩阵
     */
    const matrix_t<_T> transpose(void) const;

    /**
     * @brief 逆矩阵
     * @return const matrix_t<float>    逆矩阵
     */
    const matrix_t<float> inverse(void) const;

    /**
     * @brief PLU 分解
     * @param  _p               主元表
     * @return matrix_t<float>  分解好的矩阵
     */
    matrix_t<float> PLU(std::vector<uint32_t> &_p);

    /**
     * @brief 余子式矩阵
     * @return matrix_t<_T>     余子式矩阵
     */
    matrix_t<_T> minor(void) const;

    /**
     * @brief 代数余子式矩阵
     * @return matrix_t<_T>     代数余子式矩阵
     */
    matrix_t<_T> cofactor(void) const;

    /**
     * @brief 伴随矩阵
     * @return matrix_t<_T>     伴随矩阵
     */
    matrix_t<_T> adjugate(void) const;

    /**
     * @brief 平移矩阵
     * @param  _x               x 方向变换
     * @param  _y               y 方向变换
     * @param  _z               z 方向变换
     * @return matrix_t<_T>&    构造好的平移矩阵
     */
    matrix_t<_T> &translate(const float _x, const float _y, const float _z);

    /**
     * @brief 缩放矩阵
     * @param  _scale           缩放倍数
     * @return matrix_t<_T>&    构造好的旋转矩阵
     */
    matrix_t<_T> &scale(const float _scale);

    /**
     * @brief 缩放矩阵
     * @param  _x               x 方向缩放倍数
     * @param  _y               y 方向缩放倍数
     * @param  _z               z 方向缩放倍数
     * @return matrix_t<_T>&    构造好的旋转矩阵
     */
    matrix_t<_T> &scale(const float _x, const float _y, const float _z);

    /**
     * @brief 旋转矩阵
     * @param  _x               旋转中心的 x 坐标
     * @param  _y               旋转中心的 y 坐标
     * @param  _z               旋转中心的 z 坐标
     * @param  _angle           要旋转的弧度
     * @return matrix_t<_T>&    构造好的旋转矩阵
     * @see http://docs.gl/gl2/glRotate
     */
    matrix_t<_T> &rotate(const float _x, const float _y, const float _z,
                         const float _angle);

    /**
     * @brief 是否有非数值
     * @return true             有
     * @return false            无
     */
    bool HasNaNs(void) const;

    /**
     * @brief 角度转换为弧度
     * @param  _deg             角度
     * @return float            弧度
     */
    static float RAD(const float _deg);

    /**
     * @brief 弧度转换为角度
     * @param  _rad             弧度
     * @return float            角度
     */
    static float DEG(const float _rad);

    friend std::ostream &operator<<(std::ostream       &_os,
                                    const matrix_t<_T> &_mat) {
        _os << "[";
        for (uint32_t i = 0; i < matrix_t<_T>::ORDER; i++) {
            if (i != 0) {
                _os << "\n";
                _os << " ";
            }
            for (uint32_t j = 0; j < matrix_t<_T>::ORDER; j++) {
                _os << std::setw(7) << std::setprecision(4) << _mat[i][j];
                if (j != matrix_t<_T>::ORDER - 1) {
                    _os << " ";
                }
            }
        }
        _os << std::setw(4) << "]";
        return _os;
    }
};

template <class _T>
_T matrix_t<_T>::determ(void) const {
    return mat[0][0] * mat[1][1] * mat[2][2] * mat[3][3] +
           mat[0][0] * mat[2][1] * mat[3][2] * mat[1][3] +
           mat[0][0] * mat[3][1] * mat[1][2] * mat[2][3] +

           mat[1][0] * mat[0][1] * mat[3][2] * mat[2][3] +
           mat[1][0] * mat[3][1] * mat[2][2] * mat[0][3] +
           mat[1][0] * mat[2][1] * mat[0][2] * mat[3][3] +

           mat[2][0] * mat[3][1] * mat[0][2] * mat[1][3] +
           mat[2][0] * mat[0][1] * mat[1][2] * mat[3][3] +
           mat[2][0] * mat[1][1] * mat[3][2] * mat[0][3] +

           mat[3][0] * mat[2][1] * mat[1][2] * mat[0][3] +
           mat[3][0] * mat[1][1] * mat[0][2] * mat[2][3] +
           mat[3][0] * mat[0][1] * mat[2][2] * mat[1][3] -

           mat[0][0] * mat[1][1] * mat[3][2] * mat[2][3] -
           mat[0][0] * mat[2][1] * mat[1][2] * mat[3][3] -
           mat[0][0] * mat[3][1] * mat[2][2] * mat[1][3] -

           mat[1][0] * mat[0][1] * mat[2][2] * mat[3][3] -
           mat[1][0] * mat[3][1] * mat[0][2] * mat[2][3] -
           mat[1][0] * mat[2][1] * mat[3][2] * mat[0][3] -

           mat[2][0] * mat[3][1] * mat[1][2] * mat[0][3] -
           mat[2][0] * mat[0][1] * mat[3][2] * mat[1][3] -
           mat[2][0] * mat[1][1] * mat[0][2] * mat[3][3] -

           mat[3][0] * mat[2][1] * mat[0][2] * mat[1][3] -
           mat[3][0] * mat[1][1] * mat[2][2] * mat[0][3] -
           mat[3][0] * mat[0][1] * mat[1][2] * mat[2][3];
}

template <class _T>
matrix_t<_T>::matrix_t(void) {
    memset(mat, 0, ORDER * ORDER * sizeof(_T));
    for (uint32_t i = 0; i < ORDER; i++) {
        mat[i][i] = 1;
    }
    return;
}

template <class _T>
matrix_t<_T>::matrix_t(const matrix_t<_T> &_mat) {
    assert(!_mat.HasNaNs());
    memcpy(mat, _mat.mat, ORDER * ORDER * sizeof(_T));
    return;
}

template <class _T>
matrix_t<_T>::matrix_t(const _T *const _arr) {
    assert(_arr != nullptr);
    memcpy(mat, _arr, ORDER * ORDER * sizeof(_T));
    return;
}

template <class _T>
matrix_t<_T>::matrix_t(const _T _arr[ORDER][ORDER]) {
    memcpy(mat, _arr, ORDER * ORDER * sizeof(_T));
    return;
}

template <class _T>
matrix_t<_T>::matrix_t(const vector2_t<_T> &_v) {
    memset(mat, 0, ORDER * ORDER * sizeof(_T));
    mat[0][0] = _v.x;
    mat[1][1] = _v.y;
    mat[2][2] = 0;
    return;
}

template <class _T>
matrix_t<_T>::matrix_t(const vector3_t<_T> &_v) {
    memset(mat, 0, ORDER * ORDER * sizeof(_T));
    mat[0][0] = _v.x;
    mat[1][1] = _v.y;
    mat[2][2] = _v.z;
    return;
}

template <class _T>
matrix_t<_T> &matrix_t<_T>::operator=(const matrix_t<_T> &_mat) {
    assert(!_mat.HasNaNs());
    if (this != &_mat) {
        memcpy(mat, _mat.mat, ORDER * ORDER * sizeof(_T));
    }
    return *this;
}

template <class _T>
const matrix_t<_T> matrix_t<_T>::operator+(const matrix_t<_T> &_mat) const {
    assert(!_mat.HasNaNs());
    _T tmp[ORDER][ORDER] = {{0}};
    for (uint32_t i = 0; i < ORDER; i++) {
        for (uint32_t j = 0; j < ORDER; j++) {
            tmp[i][j] = mat[i][j] + _mat[i][j];
        }
    }
    return matrix_t<_T>(tmp);
}

template <class _T>
matrix_t<_T> &matrix_t<_T>::operator+=(const matrix_t<_T> &_mat) {
    assert(!_mat.HasNaNs());
    for (uint32_t i = 0; i < ORDER; i++) {
        for (uint32_t j = 0; j < ORDER; j++) {
            mat[i][j] += _mat[i][j];
        }
    }
    return *this;
}

template <class _T>
const matrix_t<_T> matrix_t<_T>::operator-(const matrix_t<_T> &_mat) const {
    assert(!_mat.HasNaNs());
    _T tmp[ORDER][ORDER] = {{0}};
    for (uint32_t i = 0; i < ORDER; i++) {
        for (uint32_t j = 0; j < ORDER; j++) {
            tmp[i][j] = mat[i][j] - _mat[i][j];
        }
    }
    return matrix_t<_T>(tmp);
}

template <class _T>
matrix_t<_T> &matrix_t<_T>::operator-=(const matrix_t<_T> &_mat) {
    assert(!_mat.HasNaNs());
    for (uint32_t i = 0; i < ORDER; i++) {
        for (uint32_t j = 0; j < ORDER; j++) {
            mat[i][j] -= _mat[i][j];
        }
    }
    return *this;
}

template <class _T>
const matrix_t<_T> matrix_t<_T>::operator*(const _T &_v) const {
    _T tmp[ORDER][ORDER] = {{0}};
    for (uint32_t i = 0; i < ORDER; i++) {
        for (uint32_t j = 0; j < ORDER; j++) {
            tmp[i][j] = mat[i][j] * _v;
        }
    }
    return matrix_t<_T>(tmp);
}

template <class _T>
const matrix_t<_T> matrix_t<_T>::operator*(const matrix_t<_T> &_mat) const {
    assert(!_mat.HasNaNs());
    _T tmp[ORDER][ORDER] = {{0}};
    for (uint32_t i = 0; i < ORDER; i++) {
        for (uint32_t j = 0; j < ORDER; j++) {
            for (uint32_t k = 0; k < ORDER; k++) {
                tmp[i][j] += mat[i][k] * _mat[k][j];
            }
        }
    }
    return matrix_t<_T>(tmp);
}

template <class _T>
matrix_t<_T> &matrix_t<_T>::operator*=(const _T &_v) {
    for (uint32_t i = 0; i < ORDER; i++) {
        for (uint32_t j = 0; j < ORDER; j++) {
            mat[i][j] *= _v;
        }
    }
    return *this;
}

template <class _T>
matrix_t<_T> &matrix_t<_T>::operator*=(const matrix_t<_T> &_mat) {
    assert(!_mat.HasNaNs());
    _T tmp[ORDER][ORDER] = {{0}};
    for (uint32_t i = 0; i < ORDER; i++) {
        for (uint32_t j = 0; j < _mat.ORDER; j++) {
            for (uint32_t k = 0; k < ORDER; k++) {
                tmp[i][j] += mat[i][k] * _mat[k][j];
            }
        }
    }
    mat = tmp;
    return *this;
}

template <class _T>
const vector2_t<_T> matrix_t<_T>::operator*(const vector2_t<_T> &_v) const {
    vector2_t<_T> res;
    res.x = _v.x * mat[0][0] + _v.y * mat[0][1] + 1 * mat[0][2] + 1 * mat[0][3];
    res.y = _v.x * mat[1][0] + _v.y * mat[1][1] + 1 * mat[1][2] + 1 * mat[1][3];
    return res;
}

template <class _T>
const vector3_t<_T> matrix_t<_T>::operator*(const vector3_t<_T> &_v) const {
    vector3_t<_T> res;
    res.x =
        _v.x * mat[0][0] + _v.y * mat[0][1] + _v.z * mat[0][2] + 1 * mat[0][3];
    res.y =
        _v.x * mat[1][0] + _v.y * mat[1][1] + _v.z * mat[1][2] + 1 * mat[1][3];
    res.z =
        _v.x * mat[2][0] + _v.y * mat[2][1] + _v.z * mat[2][2] + 1 * mat[2][3];
    return res;
}

template <class _T>
bool matrix_t<_T>::operator==(const matrix_t<_T> &_mat) const {
    assert(!_mat.HasNaNs());
    for (uint32_t i = 0; i < ORDER; i++) {
        for (uint32_t j = 0; j < ORDER; j++) {
            if (mat[i][j] != _mat[i][j]) {
                return false;
            }
        }
    }
    return true;
}

template <class _T>
bool matrix_t<_T>::operator!=(const matrix_t<_T> &_mat) const {
    assert(!_mat.HasNaNs());
    for (uint32_t i = 0; i < ORDER; i++) {
        for (uint32_t j = 0; j < ORDER; j++) {
            if (mat[i][j] != _mat[i][j]) {
                return true;
            }
        }
    }
    return false;
}

template <class _T>
_T *matrix_t<_T>::operator[](const uint32_t _idx) {
    assert(_idx < ORDER);
    return mat[_idx];
}

template <class _T>
const _T *matrix_t<_T>::operator[](const uint32_t _idx) const {
    assert(_idx < ORDER);
    return mat[_idx];
}

template <class _T>
uint32_t matrix_t<_T>::get_order(void) const {
    return ORDER;
}

template <class _T>
const matrix_t<_T> matrix_t<_T>::transpose(void) const {
    _T tmp[ORDER][ORDER] = {{0}};
    for (uint32_t i = 0; i < ORDER; i++) {
        for (uint32_t j = 0; j < ORDER; j++) {
            tmp[j][i] = mat[i][j];
        }
    }
    return matrix_t<_T>(tmp);
}

template <class _T>
const matrix_t<float> matrix_t<_T>::inverse(void) const {
    _T tmp[ORDER][ORDER] = {{0}};
    _T d                 = determ();
    if (d == 0) {
        return matrix_t<float>(tmp);
    }
    matrix_t<_T> adj = adjugate();
    for (uint32_t i = 0; i < ORDER; i++) {
        for (uint32_t j = 0; j < ORDER; j++) {
            tmp[i][j] = adj.mat[i][j] * (1. / d);
        }
    }
    return matrix_t<float>(tmp);
}

// template <class _T>
// matrix_t<float> matrix_t<_T>::PLU(std::vector<uint32_t> &_p) {
//     std::vector<std::vector<float>> tmp(ORDER, std::vector<float>(ORDER, 0));
//     // 转换为 float 类型
//     for (uint32_t i = 0; i < ORDER; i++) {
//         for (uint32_t j = 0; j < ORDER; j++) {
//             tmp[i][j] = (float)mat[i][j];
//         }
//     }
//
//     // 对于非方阵，选取较小的
//     uint32_t n = std::min(ORDER, ORDER);
//     // 初始化置换矩阵 _p
//     _p = std::vector<uint32_t>(n, 0);
//     for (uint32_t i = 0; i < n; i++) {
//         _p.at(i) = i;
//     }
//
//     // 如果主元所在列的绝对值最大值不是当前行，进行行交换
//     for (uint32_t i = 0; i < n; i++) {
//         _T imax = 0;
//         for (uint32_t j = i; j < n; j++) {
//             if (imax < std::abs(tmp.at(j).at(i))) {
//                 imax = std::abs(tmp.at(j).at(i));
//                 // 交换行
//                 if (i != j) {
//                     std::swap(tmp.at(i), tmp.at(j));
//                     // 记录 _p
//                     _p.at(i) = j;
//                     _p.at(j) = i;
//                 }
//             }
//         }
//     }
//     // 选主元完成
//     // L
//     // for (uint32_t i = n - 1; i >= 0; i--) {
//     //     // 变为单位下三角矩阵
//     //     // ii 归一化
//     //     if (tmp.at(i).at(i) != 1) {
//     //         _T ii = tmp.at(i).at(i);
//     //         for (uint32_t j = n - 1; j >= 0; j--) {
//     //             assert(j < 0);
//     //             tmp[i][j]] /= ii;
//     //         }
//     //     }
//     //     return matrix_t<double>(tmp);
//     // }
//
//     // U
//     for (uint32_t i = 0; i < n; i++) {
//         // 变为上三角矩阵
//         // 首先确定主元
//         float ii = tmp.at(i).at(i);
//         // 下面一行-主元行*(行列/主元)
//         for (uint32_t j = i + 1; j < n; j++) {
//             for (uint32_t k = 0; k < n; k++) {
//                 float scale = tmp.at(j).at(k) / ii;
//                 tmp.at(j).at(k) -= tmp.at(i).at(k) * scale;
//             }
//         }
//
//         // return matrix_t<double>(tmp);
//     }
//
//     return matrix_t<float>(tmp);
// }

// template <class _T>
//_T matrix_t<_T>::minor(const uint32_t _r, const uint32_t _c) const {
//     std::vector<std::vector<_T>> tmp(ORDER - 1, std::vector<_T>(ORDER - 1,
//     0)); for (uint32_t i = 0; i < ORDER - 1; i++) {
//         for (uint32_t j = 0; j < ORDER - 1; j++) {
//             uint32_t idx_r = i;
//             uint32_t idx_c = j;
//             if (i >= _r) {
//                 idx_r += 1;
//             }
//             if (j >= _c) {
//                 idx_c += 1;
//             }
//             tmp[i][j] = mat.at(idx_r).at(idx_c);
//         }
//     }
//     // 计算行列式 tmp 的值
//     _T res = matrix_t<_T>(tmp).determ();
//     return res;
// }
//
// template <class _T>
// matrix_t<_T> matrix_t<_T>::minor(void) const {
//     std::vector<std::vector<_T>> tmp(ORDER, std::vector<_T>(ORDER, 0));
//     for (uint32_t i = 0; i < ORDER; i++) {
//         for (uint32_t j = 0; j < ORDER; j++) {
//             tmp[i][j] = minor(i, j);
//         }
//     }
//     return matrix_t<_T>(tmp);
// }
//
// template <class _T>
//_T matrix_t<_T>::cofactor(const uint32_t _r, const uint32_t _c) const {
//    assert(std::is_signed<_T>::value);
//    return minor(_r, _c) * ((_r + _c) % 2 ? -1 : 1);
//}
//
// template <class _T>
// matrix_t<_T> matrix_t<_T>::cofactor(void) const {
//    std::vector<std::vector<_T>> tmp(ORDER, std::vector<_T>(ORDER, 0));
//    for (uint32_t i = 0; i < ORDER; i++) {
//        for (uint32_t j = 0; j < ORDER; j++) {
//            tmp[i][j] = cofactor(i, j);
//        }
//    }
//    return matrix_t<_T>(tmp);
//}

// template <class _T>
// matrix_t<_T> matrix_t<_T>::adjugate(void) const {
//     return cofactor().transpose();
// }

template <class _T>
matrix_t<_T> &matrix_t<_T>::translate(const float _x, const float _y,
                                      const float _z) {
    mat[0][3] = _x;
    mat[1][3] = _y;
    mat[2][3] = _z;
    mat[3][3] = 1;
    return *this;
}

template <class _T>
matrix_t<_T> &matrix_t<_T>::scale(const float _scale) {
    mat[0][0] = _scale;
    mat[1][1] = _scale;
    mat[2][2] = _scale;
    mat[3][3] = 1;
    return *this;
}

template <class _T>
matrix_t<_T> &matrix_t<_T>::scale(const float _x, const float _y,
                                  const float _z) {
    mat[0][0] = _x;
    mat[1][1] = _y;
    mat[2][2] = _z;
    mat[3][3] = 1;
    return *this;
}

template <class _T>
matrix_t<_T> &matrix_t<_T>::rotate(const float _x, const float _y,
                                   const float _z, const float _angle) {
    auto n = vector3_t(_x, _y, _z).normalize();
    auto c = std::cos(_angle);
    auto s = std::sin(_angle);

    mat[0][0] = n.x * n.x * (1 - c) + c;
    mat[0][1] = n.y * n.x * (1 - c) - s * n.z;
    mat[0][2] = n.z * n.x * (1 - c) + s * n.y;

    mat[1][0] = n.x * n.y * (1 - c) + s * n.z;
    mat[1][1] = n.y * n.y * (1 - c) + c;
    mat[1][2] = n.z * n.y * (1 - c) - s * n.x;

    mat[2][0] = n.x * n.z * (1 - c) - s * n.y;
    mat[2][1] = n.y * n.z * (1 - c) + s * n.x;
    mat[2][2] = n.z * n.z * (1 - c) + c;

    return *this;
}

template <class _T>
bool matrix_t<_T>::HasNaNs(void) const {
    for (auto i : mat) {
        if (std::isnan(*i) == true) {
            return true;
        }
    }
    return false;
}

template <class _T>
float matrix_t<_T>::RAD(const float _deg) {
    return ((M_PI / 180) * (_deg));
}

template <class _T>
float matrix_t<_T>::DEG(const float _rad) {
    return ((180 / M_PI) * (_rad));
}

typedef matrix_t<float> matrix4f_t;

#endif /* _MATRIX_HPP_ */
