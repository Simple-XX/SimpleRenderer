
/**
 * @file matrix4.hpp
 * @brief 四维矩阵
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

#ifndef _MATRIX4_HPP_
#define _MATRIX4_HPP_

#include "cfloat"
#include "cstring"
#include "iomanip"
#include "iostream"
#include "vector"

#include "log.hpp"
#include "vector.hpp"

/**
 * @brief 四阶矩阵
 * @tparam _T 矩阵元素类型
 */
template <class _T>
class matrix4_t {
private:
    /// @brief  阶数
    static constexpr const uint8_t ORDER = 4;
    /// @brief 矩阵元素
    _T                             mat[ORDER][ORDER];

    /**
     * @brief 是否有非数值
     * @return true             有
     * @return false            无
     */
    bool                           HasNaNs(void) const;

    /**
     * @brief 递归求 n 阶行列式的值
     * @param  _order           阶数
     * @return _T               值
     */
    _T                             determ(const uint8_t _order) const;

    /**
     * @brief 代数余子式矩阵
     * @param  _row             行
     * @param  _col             列
     * @param  _order           当前阶
     * @return const matrix4_t<_T>  结果
     */
    const matrix4_t<_T> cofactor(const uint8_t _row, const uint8_t _col,
                                 const uint8_t _order) const;

    /**
     * @brief 伴随矩阵
     * @return const matrix4_t<_T>  结果
     */
    const matrix4_t<_T>            adjoint(void) const;

public:
    std::string name;
    /**
     * @brief 构造函数，默认为单位矩阵
     */
    matrix4_t(void);

    /**
     * @brief 构造函数
     * @param  _mat             另一个矩阵
     */
    explicit matrix4_t(const matrix4_t<_T>& _mat);

    /**
     * @brief 构造函数
     * @param  _arr             指针
     */
    explicit matrix4_t(const _T* const _arr);

    /**
     * @brief 构造函数
     * @param  _arr             数组
     */
    explicit matrix4_t(const _T _arr[ORDER][ORDER]);

    /**
     * @brief 构造函数，构造齐次坐标，多余位置补 0
     * @param  _v               四维向量
     */
    explicit matrix4_t(const vector4_t<_T>& _v);

    /**
     * @brief 赋值
     * @param  _mat             另一个 matrix4_t
     * @return matrix4_t<_T>&    结果
     */
    matrix4_t<_T>&      operator=(const matrix4_t<_T>& _mat);

    /**
     * @brief 矩阵间加法
     * @param  _mat             另一个 matrix4_t
     * @return const matrix4_t<_T>  结果
     */
    const matrix4_t<_T> operator+(const matrix4_t<_T>& _mat) const;

    /**
     * @brief 矩阵自加
     * @param  _mat             另一个 matrix4_t
     * @return matrix4_t<_T>&    结果
     */
    matrix4_t<_T>&      operator+=(const matrix4_t<_T>& _mat);

    /**
     * @brief 矩阵间减法
     * @param  _mat             另一个 matrix4_t
     * @return const matrix4_t<_T>  结果
     */
    const matrix4_t<_T> operator-(const matrix4_t<_T>& _mat) const;

    /**
     * @brief 矩阵自减
     * @param  _mat             另一个 matrix4_t
     * @return matrix4_t<_T>&    结果
     */
    matrix4_t<_T>&      operator-=(const matrix4_t<_T>& _mat);

    /**
     * @brief 矩阵数乘
     * @tparam _U 数类型
     * @param  _v               数
     * @param  _mat             矩阵
     * @return const matrix4_t<_T>  结果
     */
    template <class _U>
    friend const matrix4_t<_T>
    operator*(const _U& _v, const matrix4_t<_T>& _mat) {
        if (std::isnan(_v)) {
            throw std::invalid_argument(log("std::isnan(_v)"));
        }
        if (_mat.HasNaNs()) {
            throw std::invalid_argument(log("_mat.HasNaNs()"));
        }

        _T tmp[ORDER][ORDER] = { { 0 } };
        for (uint8_t i = 0; i < ORDER; i++) {
            for (uint8_t j = 0; j < ORDER; j++) {
                tmp[i][j] = _mat[i][j] * _v;
            }
        }
        return matrix4_t<_T>(tmp);
    }

    /**
     * @brief 矩阵数乘
     * @tparam _U 数类型
     * @param  _mat             矩阵
     * @param  _v               数
     * @return const matrix4_t<_T>  结果
     */
    template <class _U>
    friend const matrix4_t<_T>
    operator*(const matrix4_t<_T>& _mat, const _U& _v) {
        if (std::isnan(_v)) {
            throw std::invalid_argument(log("std::isnan(_v)"));
        }
        if (_mat.HasNaNs()) {
            throw std::invalid_argument(log("_mat.HasNaNs()"));
        }

        _T tmp[ORDER][ORDER] = { { 0 } };
        for (uint8_t i = 0; i < ORDER; i++) {
            for (uint8_t j = 0; j < ORDER; j++) {
                tmp[i][j] = _v * _mat[i][j];
            }
        }
        return matrix4_t<_T>(tmp);
    }

    /**
     * @brief 行向量乘矩阵
     * @tparam _U 向量类型
     * @param  _v               向量
     * @param  _mat             矩阵
     * @return const vector4_t<_U>  结果
     */
    template <class _U>
    friend const vector4_t<_U>
    operator*(const vector4_t<_U>& _v, const matrix4_t<_T>& _mat) {
        if (_v.HasNaNs()) {
            throw std::invalid_argument(log("_v.HasNaNs()"));
        }
        if (_mat.HasNaNs()) {
            throw std::invalid_argument(log("_mat.HasNaNs()"));
        }

        auto new_x = _v.x * _mat[0][0] + _v.y * _mat[1][0] + _v.z * _mat[2][0]
                   + _v.w * _mat[3][0];
        auto new_y = _v.x * _mat[0][1] + _v.y * _mat[1][1] + _v.z * _mat[2][1]
                   + _v.w * _mat[3][1];
        auto new_z = _v.x * _mat[0][2] + _v.y * _mat[1][2] + _v.z * _mat[2][2]
                   + _v.w * _mat[3][2];
        auto new_w = _v.x * _mat[0][3] + _v.y * _mat[1][3] + _v.z * _mat[2][3]
                   + _v.w * _mat[3][3];
        return vector4_t<_U>(new_x, new_y, new_z, new_w);
    }

    /**
     * @brief 矩阵乘列向量
     * @tparam _U 向量类型
     * @param  _mat             矩阵
     * @param  _v               向量
     * @return const vector4_t<_U>  结果
     */
    template <class _U>
    friend const vector4_t<_U>
    operator*(const matrix4_t<_T>& _mat, const vector4_t<_U>& _v) {
        if (_mat.HasNaNs()) {
            throw std::invalid_argument(log("_mat.HasNaNs()"));
        }
        if (_v.HasNaNs()) {
            throw std::invalid_argument(log("_v.HasNaNs()"));
        }
        auto new_x = _v.x * _mat[0][0] + _v.y * _mat[0][1] + _v.z * _mat[0][2]
                   + _v.w * _mat[0][3];
        auto new_y = _v.x * _mat[1][0] + _v.y * _mat[1][1] + _v.z * _mat[1][2]
                   + _v.w * _mat[1][3];
        auto new_z = _v.x * _mat[2][0] + _v.y * _mat[2][1] + _v.z * _mat[2][2]
                   + _v.w * _mat[2][3];
        auto new_w = _v.x * _mat[3][0] + _v.y * _mat[3][1] + _v.z * _mat[3][2]
                   + _v.w * _mat[3][3];
        return vector4_t<_U>(new_x, new_y, new_z, new_w);
    }

    /**
     * @brief 矩阵间乘法
     * @param  _mat             另一个 matrix4_t
     * @return const matrix4_t<_T>  结果
     */
    const matrix4_t<_T> operator*(const matrix4_t<_T>& _mat) const;

    /**
     * @brief 矩阵数乘的自乘
     * @param  _v               数
     * @return matrix4_t<_T>&    结果
     */
    matrix4_t<_T>&      operator*=(const _T& _v);

    /**
     * @brief 行向量乘矩阵
     * @tparam _U 向量类型
     * @param  _v               向量
     * @param  _mat             矩阵
     * @return vector4_t<_U>&   结果
     */
    template <class _U>
    friend vector4_t<_U>&
    operator*=(vector4_t<_U>& _v, const matrix4_t<_T>& _mat) {
        if (_v.HasNaNs()) {
            throw std::invalid_argument(log("_v.HasNaNs()"));
        }
        if (_mat.HasNaNs()) {
            throw std::invalid_argument(log("_mat.HasNaNs()"));
        }
        auto new_x = _v.x * _mat[0][0] + _v.y * _mat[1][0] + _v.z * _mat[2][0]
                   + _v.w * _mat[3][0];
        auto new_y = _v.x * _mat[0][1] + _v.y * _mat[1][1] + _v.z * _mat[2][1]
                   + _v.w * _mat[3][1];
        auto new_z = _v.x * _mat[0][2] + _v.y * _mat[1][2] + _v.z * _mat[2][2]
                   + _v.w * _mat[3][2];
        auto new_w = _v.x * _mat[0][3] + _v.y * _mat[1][3] + _v.z * _mat[2][3]
                   + _v.w * _mat[3][3];
        _v.x = new_x;
        _v.y = new_y;
        _v.z = new_z;
        _v.w = new_w;
        return _v;
    }

    /**
     * @brief 矩阵乘列向量
     * @tparam _U 向量类型
     * @param  _mat             矩阵
     * @param  _v               向量
     * @return vector4_t<_U>&   结果
     */
    template <class _U>
    friend vector4_t<_U>&
    operator*=(const matrix4_t<_T>& _mat, vector4_t<_U>& _v) {
        if (_v.HasNaNs()) {
            throw std::invalid_argument(log("_v.HasNaNs()"));
        }
        auto new_x = _v.x * _mat[0][0] + _v.y * _mat[0][1] + _v.z * _mat[0][2]
                   + _v.w * _mat[0][3];
        auto new_y = _v.x * _mat[1][0] + _v.y * _mat[1][1] + _v.z * _mat[1][2]
                   + _v.w * _mat[1][3];
        auto new_z = _v.x * _mat[2][0] + _v.y * _mat[2][1] + _v.z * _mat[2][2]
                   + _v.w * _mat[2][3];
        auto new_w = _v.x * _mat[3][0] + _v.y * _mat[3][1] + _v.z * _mat[3][2]
                   + _v.w * _mat[3][3];
        _v.x = new_x;
        _v.y = new_y;
        _v.z = new_z;
        _v.w = new_w;
        return _v;
    }

    /**
     * @brief 矩阵间自乘
     * @param  _mat             另一个 matrix4_t
     * @return matrix4_t<_T>&    结果
     */
    matrix4_t<_T>&      operator*=(const matrix4_t<_T>& _mat);

    /**
     * @brief 矩阵相等
     * @param  _mat             另一个 matrix4_t
     * @return true             相等
     * @return false            不等
     */
    bool                operator==(const matrix4_t<_T>& _mat) const;

    /**
     * @brief 矩阵不等
     * @param  _mat             另一个 matrix4_t
     * @return true             不等
     * @return false            相等
     */
    bool                operator!=(const matrix4_t<_T>& _mat) const;

    /**
     * @brief 下标重载
     * @param  _idx             行
     * @return _T*              行指针
     * @note    注意不要越界访问
     */
    _T*                 operator[](const uint8_t _idx);

    /**
     * @brief 下标重载
     * @param  _idx             行
     * @return const _T*        行指针
     */
    const _T*           operator[](const uint8_t _idx) const;

    /**
     * @brief 矩阵转置
     * @return const matrix4_t<_T>   转置矩阵
     */
    const matrix4_t<_T> transpose(void) const;

    /**
     * @brief 逆矩阵
     * @return const matrix4_t<_T>       逆矩阵
     * @see https://www.geeksforgeeks.org/adjoint-inverse-matrix/
     */
    const matrix4_t<_T> inverse(void) const;

    /**
     * @brief 缩放矩阵
     * @param  _scale           缩放倍数
     * @return const matrix4_t<_T>   构造好的旋转矩阵
     * @note 缩放的是顶点
     */
    const matrix4_t<_T> scale(const _T& _scale) const;

    /**
     * @brief 缩放矩阵
     * @param  _x               x 方向缩放倍数
     * @param  _y               y 方向缩放倍数
     * @param  _z               z 方向缩放倍数
     * @return const matrix4_t<_T>   构造好的旋转矩阵
     * @note 缩放的是顶点
     */
    const matrix4_t<_T> scale(const _T& _x, const _T& _y, const _T& _z) const;

    /**
     * @brief 旋转矩阵，Rodriguez 方法，左手系，x 向右，y 向下，z 向屏幕外
     * @param  _axis            旋转轴，起点为原点，单位向量
     * @param  _angle           要旋转的角度
     * @return const matrix4_t<_T>   构造好的旋转矩阵
     * @note 旋转的是顶点，逆时针为正方向
     * @see https://zhuanlan.zhihu.com/p/401806150
     * @todo 四元数实现
     * @see https://krasjet.github.io/quaternion/quaternion.pdf
     * R(_axis, _angle) = cos(_angle) * I
     *                  + (1 - cos(_angle)) * r * rt
     *                  + sin(_angle) * N
     * 其中，_axis 为旋转轴，_angle 为要旋转的弧度，I 为单位矩阵，
     * r 为 [_axis.x, _axis.y, _axis.z]，rt 为 r 的转置
     * N 为 {       0, -_axis.z,  _axis.y},
     *     { _axis.z,        0, -_axis.x},
     *     {-_axis.y,  _axis.x,        0}
     */
    const matrix4_t<_T>
    rotate(const vector4_t<_T>& _axis, const float& _angle) const;

    /**
     * @brief 从 _from 旋转到 _to，不需要单位向量
     * @param  _from            开始位置向量
     * @param  _to              目标位置向量
     * @return const matrix4_t<_T>  旋转矩阵
     */
    const matrix4_t<_T>
    rotate_from_to(const vector4f_t& _from, const vector4f_t& _to) const;

    /**
     * @brief 平移矩阵
     * @param  _x               x 方向变换
     * @param  _y               y 方向变换
     * @param  _z               z 方向变换
     * @return const matrix4_t<_T>   构造好的平移矩阵
     * @note 先旋转再平移
     * @note 平移的是顶点
     */

    const matrix4_t<_T>
                 translate(const _T& _x, const _T& _y, const _T& _z) const;
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

    friend std::ostream&
    operator<<(std::ostream& _os, const matrix4_t<_T>& _mat) {
        _os << "[";
        for (uint8_t i = 0; i < matrix4_t<_T>::ORDER; i++) {
            if (i != 0) {
                _os << "\n";
                _os << " ";
            }
            for (uint8_t j = 0; j < matrix4_t<_T>::ORDER; j++) {
                _os << std::setw(10) << std::setprecision(20) << _mat[i][j];
                if (j != matrix4_t<_T>::ORDER - 1) {
                    _os << " ";
                }
            }
        }
        _os << std::setw(4) << "]";
        return _os;
    }

    /// @todo 逗号初始化
    /// @see https://gitee.com/aczz/cv-dbg/blob/master/comma_init/v4.cpp
    /// @see https://zhuanlan.zhihu.com/p/377573738
};

template <class _T>
bool matrix4_t<_T>::HasNaNs(void) const {
    for (uint8_t i = 0; i < ORDER; i++) {
        for (uint8_t j = 0; j < ORDER; j++) {
            if (std::isnan(mat[i][j]) == true) {
                return true;
            }
        }
    }
    return false;
}

template <class _T>
_T matrix4_t<_T>::determ(const uint8_t _order) const {
    // 递归返回条件
    if (_order == 1) {
        return mat[0][0];
    }

    _T  res  = 0;
    // 当前正负
    int sign = 1;
    // 计算 mat[0][i] 的代数余子式
    for (uint8_t i = 0; i < _order; i++) {
        res  += sign * mat[0][i] * cofactor(0, i, _order).determ(_order - 1);
        // 符号取反
        sign = -sign;
    }
    return res;
}

template <class _T>
const matrix4_t<_T>
matrix4_t<_T>::cofactor(const uint8_t _row, const uint8_t _col,
                        const uint8_t _order) const {
    _T   tmp[ORDER][ORDER] = { { 0 } };
    auto row_idx           = 0;
    auto col_idx           = 0;
    for (uint8_t i = 0; i < _order; i++) {
        for (uint8_t j = 0; j < _order; j++) {
            if (i != _row && j != _col) {
                tmp[row_idx][col_idx++] = mat[i][j];
                // 换行
                if (col_idx == _order - 1) {
                    col_idx = 0;
                    row_idx++;
                }
            }
        }
    }
    return matrix4_t<_T>(tmp);
}

template <class _T>
const matrix4_t<_T> matrix4_t<_T>::adjoint(void) const {
    _T  tmp[ORDER][ORDER] = { { 0 } };
    // 当前正负
    int sign              = 1;
    for (uint8_t i = 0; i < ORDER; i++) {
        for (uint8_t j = 0; j < ORDER; j++) {
            // 判断当前符号，行列索引之和为偶数时为正
            sign      = ((i + j) % 2 == 0) ? 1 : -1;
            // 计算代数余子式矩阵并转置
            tmp[j][i] = (sign) * (cofactor(i, j, ORDER).determ(ORDER - 1));
        }
    }
    return matrix4_t<_T>(tmp);
}

template <class _T>
matrix4_t<_T>::matrix4_t(void) {
    memset(mat, 0, ORDER * ORDER * sizeof(_T));
    for (uint8_t i = 0; i < ORDER; i++) {
        mat[i][i] = 1;
    }
    return;
}

template <class _T>
matrix4_t<_T>::matrix4_t(const matrix4_t<_T>& _mat) {
    if (_mat.HasNaNs()) {
        throw std::invalid_argument(log("_mat.HasNaNs()"));
    }
    memcpy(mat, _mat.mat, ORDER * ORDER * sizeof(_T));
    return;
}

template <class _T>
matrix4_t<_T>::matrix4_t(const _T* const _arr) {
    if (_arr == nullptr) {
        throw std::invalid_argument(log("_arr == nullptr"));
    }
    for (auto i = 0; i < ORDER * ORDER; i++) {
        if (std::isnan(_arr[i])) {
            throw std::invalid_argument(log("std::isnan(_arr[i])"));
        }
    }
    memcpy(mat, _arr, ORDER * ORDER * sizeof(_T));
    return;
}

template <class _T>
matrix4_t<_T>::matrix4_t(const _T _arr[ORDER][ORDER]) {
    if (_arr == nullptr) {
        throw std::invalid_argument(log("_arr == nullptr"));
    }
    for (uint8_t i = 0; i < ORDER; i++) {
        for (uint8_t j = 0; j < ORDER; j++) {
            if (std::isnan(_arr[i][j])) {
                throw std::invalid_argument(log("std::isnan(_arr[i][j])"));
            }
        }
    }
    memcpy(mat, _arr, ORDER * ORDER * sizeof(_T));
    return;
}

template <class _T>
matrix4_t<_T>::matrix4_t(const vector4_t<_T>& _v) {
    if (_v.HasNaNs()) {
        throw std::invalid_argument(log("_v.HasNaNs()"));
    }
    memset(mat, 0, ORDER * ORDER * sizeof(_T));
    mat[0][0] = _v.x;
    mat[1][1] = _v.y;
    mat[2][2] = _v.z;
    mat[3][3] = _v.w;
    return;
}

template <class _T>
matrix4_t<_T>& matrix4_t<_T>::operator=(const matrix4_t<_T>& _mat) {
    if (_mat.HasNaNs()) {
        throw std::invalid_argument(log("_mat.HasNaNs()"));
    }
    if (this == &_mat) {
        throw std::runtime_error(log("this == &_mat"));
    }
    memcpy(mat, _mat.mat, ORDER * ORDER * sizeof(_T));
    return *this;
}

template <class _T>
const matrix4_t<_T> matrix4_t<_T>::operator+(const matrix4_t<_T>& _mat) const {
    if (_mat.HasNaNs()) {
        throw std::invalid_argument(log("_mat.HasNaNs()"));
    }
    _T tmp[ORDER][ORDER] = { { 0 } };
    for (uint8_t i = 0; i < ORDER; i++) {
        for (uint8_t j = 0; j < ORDER; j++) {
            tmp[i][j] = mat[i][j] + _mat[i][j];
        }
    }
    return matrix4_t<_T>(tmp);
}

template <class _T>
matrix4_t<_T>& matrix4_t<_T>::operator+=(const matrix4_t<_T>& _mat) {
    if (_mat.HasNaNs()) {
        throw std::invalid_argument(log("_mat.HasNaNs()"));
    }
    for (uint8_t i = 0; i < ORDER; i++) {
        for (uint8_t j = 0; j < ORDER; j++) {
            mat[i][j] += _mat[i][j];
        }
    }
    return *this;
}

template <class _T>
const matrix4_t<_T> matrix4_t<_T>::operator-(const matrix4_t<_T>& _mat) const {
    if (_mat.HasNaNs()) {
        throw std::invalid_argument(log("_mat.HasNaNs()"));
    }
    _T tmp[ORDER][ORDER] = { { 0 } };
    for (uint8_t i = 0; i < ORDER; i++) {
        for (uint8_t j = 0; j < ORDER; j++) {
            tmp[i][j] = mat[i][j] - _mat[i][j];
        }
    }
    return matrix4_t<_T>(tmp);
}

template <class _T>
matrix4_t<_T>& matrix4_t<_T>::operator-=(const matrix4_t<_T>& _mat) {
    if (_mat.HasNaNs()) {
        throw std::invalid_argument(log("_mat.HasNaNs()"));
    }
    for (uint8_t i = 0; i < ORDER; i++) {
        for (uint8_t j = 0; j < ORDER; j++) {
            mat[i][j] -= _mat[i][j];
        }
    }
    return *this;
}

template <class _T>
const matrix4_t<_T> matrix4_t<_T>::operator*(const matrix4_t<_T>& _mat) const {
    if (_mat.HasNaNs()) {
        throw std::invalid_argument(log("_mat.HasNaNs()"));
    }
    // std::cout << name << ", " << _mat.name << std::endl;
    // std::cout << *this << std::endl;
    // std::cout << _mat << std::endl;
    _T tmp[ORDER][ORDER] = { { 0 } };
    for (uint8_t i = 0; i < ORDER; i++) {
        for (uint8_t j = 0; j < ORDER; j++) {
            for (uint8_t k = 0; k < ORDER; k++) {
                tmp[i][j] += mat[i][k] * _mat[k][j];
            }
        }
    }
    return matrix4_t<_T>(tmp);
}

template <class _T>
matrix4_t<_T>& matrix4_t<_T>::operator*=(const _T& _v) {
    if (std::isnan(_v)) {
        throw std::invalid_argument(log("std::isnan(_v)"));
    }
    for (uint8_t i = 0; i < ORDER; i++) {
        for (uint8_t j = 0; j < ORDER; j++) {
            mat[i][j] *= _v;
        }
    }
    return *this;
}

template <class _T>
matrix4_t<_T>& matrix4_t<_T>::operator*=(const matrix4_t<_T>& _mat) {
    if (_mat.HasNaNs()) {
        throw std::invalid_argument(log("_mat.HasNaNs()"));
    }
    _T tmp[ORDER][ORDER] = { { 0 } };
    for (uint8_t i = 0; i < ORDER; i++) {
        for (uint8_t j = 0; j < ORDER; j++) {
            for (uint8_t k = 0; k < ORDER; k++) {
                tmp[i][j] += mat[i][k] * _mat[k][j];
            }
        }
    }
    memcpy(mat, tmp, ORDER * ORDER * sizeof(_T));
    return *this;
}

template <class _T>
bool matrix4_t<_T>::operator==(const matrix4_t<_T>& _mat) const {
    if (_mat.HasNaNs()) {
        throw std::invalid_argument(log("_mat.HasNaNs()"));
    }
    for (uint8_t i = 0; i < ORDER; i++) {
        for (uint8_t j = 0; j < ORDER; j++) {
            if (std::abs(mat[i][j] - _mat[i][j])
                > std::numeric_limits<_T>::epsilon()) {
                return false;
            }
        }
    }
    return true;
}

template <class _T>
bool matrix4_t<_T>::operator!=(const matrix4_t<_T>& _mat) const {
    if (_mat.HasNaNs()) {
        throw std::invalid_argument(log("_mat.HasNaNs()"));
    }
    for (uint8_t i = 0; i < ORDER; i++) {
        for (uint8_t j = 0; j < ORDER; j++) {
            if (std::abs(mat[i][j] - _mat[i][j])
                > std::numeric_limits<_T>::epsilon()) {
                return true;
            }
        }
    }
    return false;
}

template <class _T>
_T* matrix4_t<_T>::operator[](const uint8_t _idx) {
    if (_idx > ORDER) {
        throw std::invalid_argument(log("_idx > ORDER"));
    }
    return mat[_idx];
}

template <class _T>
const _T* matrix4_t<_T>::operator[](const uint8_t _idx) const {
    if (_idx > ORDER) {
        throw std::invalid_argument(log("_idx > ORDER"));
    }
    return mat[_idx];
}

template <class _T>
const matrix4_t<_T> matrix4_t<_T>::transpose(void) const {
    _T tmp[ORDER][ORDER] = { { 0 } };
    for (uint8_t i = 0; i < ORDER; i++) {
        for (uint8_t j = 0; j < ORDER; j++) {
            tmp[j][i] = mat[i][j];
        }
    }
    return matrix4_t<_T>(tmp);
}

template <class _T>
const matrix4_t<_T> matrix4_t<_T>::inverse(void) const {
    if (HasNaNs()) {
        throw std::invalid_argument(log("HasNaNs()"));
    }
    // 计算行列式
    _T det = determ(ORDER);
    if (std::abs(det) <= std::numeric_limits<_T>::epsilon()) {
        throw std::runtime_error(
          log("std::abs(det) <= std::numeric_limits<_T>::epsilon(), Singular "
              "matrix, can't find its inverse"));
    }

    // 计算伴随矩阵
    auto adj = adjoint();

    // 逆矩阵=伴随矩阵/行列式
    auto tmp = adj * (1 / det);

    return matrix4_t<_T>(tmp);
}

template <class _T>
const matrix4_t<_T> matrix4_t<_T>::scale(const _T& _scale) const {
    if (std::isnan(_scale)) {
        throw std::invalid_argument(log("std::isnan(_scale)"));
    }
    matrix4_t<_T> tmp;
    tmp.mat[0][0] = _scale;
    tmp.mat[1][1] = _scale;
    tmp.mat[2][2] = _scale;
    tmp.mat[3][3] = 1;
    return tmp * *this;
}

template <class _T>
const matrix4_t<_T>
matrix4_t<_T>::scale(const _T& _x, const _T& _y, const _T& _z) const {
    if (std::isnan(_x) || std::isnan(_y) || std::isnan(_z)) {
        throw std::invalid_argument(
          log("std::isnan(_x) || std::isnan(_y) || std::isnan(_z)"));
    }
    matrix4_t<_T> tmp;
    tmp.mat[0][0] = _x;
    tmp.mat[1][1] = _y;
    tmp.mat[2][2] = _z;
    tmp.mat[3][3] = 1;
    return tmp * *this;
}

template <class _T>
const matrix4_t<_T>
matrix4_t<_T>::rotate(const vector4_t<_T>& _axis, const float& _angle) const {
    if (_axis.HasNaNs() || std::isnan(_angle)) {
        throw std::invalid_argument(
          log("std::isnan(_x) || std::isnan(_y) || std::isnan(_z)|| "
              "std::isnan(_angle)"));
    }

    // 角度转弧度
    auto rad          = RAD(_angle);

    // 计算公式中的参数
    auto c            = std::cos(rad);
    auto s            = std::sin(rad);
    auto t            = 1 - c;

    auto tx           = t * _axis.x;
    auto ty           = t * _axis.y;
    auto tz           = t * _axis.z;

    auto sx           = s * _axis.x;
    auto sy           = s * _axis.y;
    auto sz           = s * _axis.z;

    // 计算结果
    // cos(_angle) * I
    _T   cI_arr[4][4] = {
        {c, 0, 0, 0},
        {0, c, 0, 0},
        {0, 0, c, 0},
        {0, 0, 0, 0}
    };
    auto cI            = matrix4_t<_T>(cI_arr);

    // (1 - cos(_angle)) * r * rt
    _T   rrt_arr[4][4] = {
        {tx * _axis.x, tx * _axis.y, tx * _axis.z, 0},
        {tx * _axis.y, ty * _axis.y, ty * _axis.z, 0},
        {tx * _axis.z, ty * _axis.z, tz * _axis.z, 0},
        {           0,            0,            0, 0}
    };
    auto  rrt          = matrix4_t<_T>(rrt_arr);

    // sin(_angle) * N
    float sN_arr[4][4] = {
        {  0, -sz,  sy, 0},
        { sz,   0, -sx, 0},
        {-sy,  sx,   0, 0},
        {  0,   0,   0, 0}
    };
    auto          sN  = matrix4_t<_T>(sN_arr);

    matrix4_t<_T> res = cI + rrt + sN;
    res[3][3]         = 1;

    return res * *this;
}

template <class _T>
const matrix4_t<_T> matrix4_t<_T>::rotate_from_to(const vector4f_t& _from,
                                                  const vector4f_t& _to) const {
    auto f = _from.normalize();
    auto t = _to.normalize();

    // axis multiplication by sin
    auto vs(t ^ f);

    // axis of rotation
    auto v(vs);
    v       = v.normalize();

    // cosinus angle
    auto ca = f * t;

    auto vt(v * (1 - ca));

    _T   M[16] = { 0 };

    M[0]       = vt.x * v.x + ca;
    M[5]       = vt.y * v.y + ca;
    M[10]      = vt.z * v.z + ca;

    vt.x       *= v.y;
    vt.z       *= v.x;
    vt.y       *= v.z;

    M[1]       = vt.x + vs.z;
    M[2]       = vt.z - vs.y;
    M[3]       = 0;

    M[4]       = vt.x - vs.z;
    M[6]       = vt.y + vs.x;
    M[7]       = 0;

    M[8]       = vt.z + vs.y;
    M[9]       = vt.y - vs.x;
    M[11]      = 0;

    M[12]      = 0;
    M[13]      = 0;
    M[14]      = 0;
    M[15]      = 1;

    return matrix4_t<_T>(M);
}

template <class _T>
const matrix4_t<_T>
matrix4_t<_T>::translate(const _T& _x, const _T& _y, const _T& _z) const {
    if (std::isnan(_x) || std::isnan(_y) || std::isnan(_z)) {
        throw std::invalid_argument(
          log("std::isnan(_x) || std::isnan(_y) || std::isnan(_z)"));
    }
    matrix4_t<_T> tmp;
    tmp.mat[0][3] = _x;
    tmp.mat[1][3] = _y;
    tmp.mat[2][3] = _z;
    tmp.mat[3][3] = 1;
    return tmp * *this;
}

template <class _T>
float matrix4_t<_T>::RAD(const float _deg) {
    if (std::isnan(_deg)) {
        throw std::invalid_argument(log("std::isnan(_deg)"));
    }
    return ((M_PI / 180) * (_deg));
}

template <class _T>
float matrix4_t<_T>::DEG(const float _rad) {
    if (std::isnan(_rad)) {
        throw std::invalid_argument(log("std::isnan(_rad)"));
    }
    return ((180 / M_PI) * (_rad));
}

typedef matrix4_t<float> matrix4f_t;

#endif /* _MATRIX4_HPP_ */
