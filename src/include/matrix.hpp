
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

#include "array"
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
    requires std::is_same<_T, float>::value
class matrix4_t {
private:
    /// @brief  阶数
    static constexpr const uint8_t ORDER = 4;
    /// @brief 矩阵元素
    std::array<_T, ORDER * ORDER>  mat_arr;

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

        _T tmp[ORDER * ORDER] = { 0 };

        tmp[0]                = _mat.mat_arr[0] * _v;
        tmp[1]                = _mat.mat_arr[1] * _v;
        tmp[2]                = _mat.mat_arr[2] * _v;
        tmp[3]                = _mat.mat_arr[3] * _v;

        tmp[4]                = _mat.mat_arr[4] * _v;
        tmp[5]                = _mat.mat_arr[5] * _v;
        tmp[6]                = _mat.mat_arr[6] * _v;
        tmp[7]                = _mat.mat_arr[7] * _v;

        tmp[8]                = _mat.mat_arr[8] * _v;
        tmp[9]                = _mat.mat_arr[9] * _v;
        tmp[10]               = _mat.mat_arr[10] * _v;
        tmp[11]               = _mat.mat_arr[11] * _v;

        tmp[12]               = _mat.mat_arr[12] * _v;
        tmp[13]               = _mat.mat_arr[13] * _v;
        tmp[14]               = _mat.mat_arr[14] * _v;
        tmp[15]               = _mat.mat_arr[15] * _v;

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

        _T tmp[ORDER * ORDER] = { 0 };

        tmp[0]                = _v * _mat.mat_arr[0];
        tmp[1]                = _v * _mat.mat_arr[1];
        tmp[2]                = _v * _mat.mat_arr[2];
        tmp[3]                = _v * _mat.mat_arr[3];

        tmp[4]                = _v * _mat.mat_arr[4];
        tmp[5]                = _v * _mat.mat_arr[5];
        tmp[6]                = _v * _mat.mat_arr[6];
        tmp[7]                = _v * _mat.mat_arr[7];

        tmp[8]                = _v * _mat.mat_arr[8];
        tmp[9]                = _v * _mat.mat_arr[9];
        tmp[10]               = _v * _mat.mat_arr[10];
        tmp[11]               = _v * _mat.mat_arr[11];

        tmp[12]               = _v * _mat.mat_arr[12];
        tmp[13]               = _v * _mat.mat_arr[13];
        tmp[14]               = _v * _mat.mat_arr[14];
        tmp[15]               = _v * _mat.mat_arr[15];

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
        auto new_x = _v.x * _mat.mat_arr[0] + _v.y * _mat.mat_arr[4]
                   + _v.z * _mat.mat_arr[8] + _v.w * _mat.mat_arr[12];
        auto new_y = _v.x * _mat.mat_arr[1] + _v.y * _mat.mat_arr[5]
                   + _v.z * _mat.mat_arr[9] + _v.w * _mat.mat_arr[13];
        auto new_z = _v.x * _mat.mat_arr[2] + _v.y * _mat.mat_arr[6]
                   + _v.z * _mat.mat_arr[10] + _v.w * _mat.mat_arr[14];
        auto new_w = _v.x * _mat.mat_arr[3] + _v.y * _mat.mat_arr[7]
                   + _v.z * _mat.mat_arr[11] + _v.w * _mat.mat_arr[15];
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
        auto new_x = _v.x * _mat.mat_arr[0] + _v.y * _mat.mat_arr[1]
                   + _v.z * _mat.mat_arr[2] + _v.w * _mat.mat_arr[3];
        auto new_y = _v.x * _mat.mat_arr[4] + _v.y * _mat.mat_arr[5]
                   + _v.z * _mat.mat_arr[6] + _v.w * _mat.mat_arr[7];
        auto new_z = _v.x * _mat.mat_arr[8] + _v.y * _mat.mat_arr[9]
                   + _v.z * _mat.mat_arr[10] + _v.w * _mat.mat_arr[11];
        auto new_w = _v.x * _mat.mat_arr[12] + _v.y * _mat.mat_arr[13]
                   + _v.z * _mat.mat_arr[14] + _v.w * _mat.mat_arr[15];
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
        auto new_x = _v.x * _mat.mat_arr[0] + _v.y * _mat.mat_arr[4]
                   + _v.z * _mat.mat_arr[8] + _v.w * _mat.mat_arr[12];
        auto new_y = _v.x * _mat.mat_arr[1] + _v.y * _mat.mat_arr[5]
                   + _v.z * _mat.mat_arr[9] + _v.w * _mat.mat_arr[13];
        auto new_z = _v.x * _mat.mat_arr[2] + _v.y * _mat.mat_arr[6]
                   + _v.z * _mat.mat_arr[10] + _v.w * _mat.mat_arr[14];
        auto new_w = _v.x * _mat.mat_arr[3] + _v.y * _mat.mat_arr[7]
                   + _v.z * _mat.mat_arr[11] + _v.w * _mat.mat_arr[15];

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
        auto new_x = _v.x * _mat.mat_arr[0] + _v.y * _mat.mat_arr[1]
                   + _v.z * _mat.mat_arr[2] + _v.w * _mat.mat_arr[3];
        auto new_y = _v.x * _mat.mat_arr[4] + _v.y * _mat.mat_arr[5]
                   + _v.z * _mat.mat_arr[6] + _v.w * _mat.mat_arr[7];
        auto new_z = _v.x * _mat.mat_arr[8] + _v.y * _mat.mat_arr[9]
                   + _v.z * _mat.mat_arr[10] + _v.w * _mat.mat_arr[11];
        auto new_w = _v.x * _mat.mat_arr[12] + _v.y * _mat.mat_arr[13]
                   + _v.z * _mat.mat_arr[14] + _v.w * _mat.mat_arr[15];

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
     * @brief 下标重载
     * @param  _idx             行
     * @return _T*              行指针
     * @note    注意不要越界访问
     */
    _T                  operator[](const uint8_t _idx);

    /**
     * @brief 下标重载
     * @param  _idx             行
     * @return const _T*        行指针
     */
    const _T            operator[](const uint8_t _idx) const;

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
     * @note 旋转轴如无特殊需要应使用单位向量
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
        _os.setf(std::ios::right);
        _os.precision(16);

        _os << "{\n";

        _os << std::setw(4) << "{" << std::setw(25) << _mat.mat_arr[0] << ", "
            << _mat.mat_arr[1] << ", " << _mat.mat_arr[2] << ", "
            << _mat.mat_arr[3] << "},\n";

        _os << std::setw(4) << "{" << std::setw(25) << _mat.mat_arr[4] << ", "
            << _mat.mat_arr[5] << ", " << _mat.mat_arr[6] << ", "
            << _mat.mat_arr[7] << "},\n";

        _os << std::setw(4) << "{" << std::setw(25) << _mat.mat_arr[8] << ", "
            << _mat.mat_arr[9] << ", " << _mat.mat_arr[10] << ", "
            << _mat.mat_arr[11] << "},\n";

        _os << std::setw(4) << "{" << std::setw(25) << _mat.mat_arr[12] << ", "
            << _mat.mat_arr[13] << ", " << _mat.mat_arr[14] << ", "
            << _mat.mat_arr[15] << "}\n";

        _os << "}";

        _os.unsetf(std::ios::right);
        _os.precision(6);

        return _os;
    }

    /// @todo 逗号初始化
    /// @see https://gitee.com/aczz/cv-dbg/blob/master/comma_init/v4.cpp
    /// @see https://zhuanlan.zhihu.com/p/377573738
};

template <class _T>
bool matrix4_t<_T>::HasNaNs(void) const {
    for (const auto& i : mat_arr) {
        if (std::isnan(i) == true) {
            return true;
        }
    }

    return false;
}

template <class _T>
_T matrix4_t<_T>::determ(const uint8_t _order) const {
    // 递归返回条件
    if (_order == 1) {
        return mat_arr[0];
    }

    _T  res  = 0;
    // 当前正负
    int sign = 1;
    // 计算 mat[0][i] 的代数余子式
    for (uint8_t i = 0; i < _order; i++) {
        res  += sign * mat_arr[i] * cofactor(0, i, _order).determ(_order - 1);
        // 符号取反
        sign = -sign;
    }

    return res;
}

template <class _T>
const matrix4_t<_T>
matrix4_t<_T>::cofactor(const uint8_t _row, const uint8_t _col,
                        const uint8_t _order) const {
    _T   tmp[ORDER * ORDER] = { 0 };
    auto row_idx            = 0;
    auto col_idx            = 0;
    for (uint8_t i = 0; i < _order; i++) {
        for (uint8_t j = 0; j < _order; j++) {
            if (i != _row && j != _col) {
                tmp[row_idx * ORDER + col_idx++] = mat_arr[i * ORDER + j];
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
    _T tmp[ORDER * ORDER] = { 0 };

    // 计算代数余子式矩阵并转置
    // 行列索引之和为奇数时为负
    tmp[0]                = cofactor(0, 0, 4).determ(3);
    tmp[1]                = -cofactor(1, 0, 4).determ(3);
    tmp[2]                = cofactor(2, 0, 4).determ(3);
    tmp[3]                = -cofactor(3, 0, 4).determ(3);

    tmp[4]                = -cofactor(0, 1, 4).determ(3);
    tmp[5]                = cofactor(1, 1, 4).determ(3);
    tmp[6]                = -cofactor(2, 1, 4).determ(3);
    tmp[7]                = cofactor(3, 1, 4).determ(3);

    tmp[8]                = cofactor(0, 2, 4).determ(3);
    tmp[9]                = -cofactor(1, 2, 4).determ(3);
    tmp[10]               = cofactor(2, 2, 4).determ(3);
    tmp[11]               = -cofactor(3, 2, 4).determ(3);

    tmp[12]               = -cofactor(0, 3, 4).determ(3);
    tmp[13]               = cofactor(1, 3, 4).determ(3);
    tmp[14]               = -cofactor(2, 3, 4).determ(3);
    tmp[15]               = cofactor(3, 3, 4).determ(3);

    return matrix4_t<_T>(tmp);
}

template <class _T>
matrix4_t<_T>::matrix4_t(void) {
    mat_arr.fill(0);
    mat_arr[0]  = 1;
    mat_arr[5]  = 1;
    mat_arr[10] = 1;
    mat_arr[15] = 1;

    return;
}

template <class _T>
matrix4_t<_T>::matrix4_t(const matrix4_t<_T>& _mat) {
    if (_mat.HasNaNs()) {
        throw std::invalid_argument(log("_mat.HasNaNs()"));
    }
    mat_arr = _mat.mat_arr;
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

    mat_arr[0]  = _arr[0];
    mat_arr[1]  = _arr[1];
    mat_arr[2]  = _arr[2];
    mat_arr[3]  = _arr[3];

    mat_arr[4]  = _arr[4];
    mat_arr[5]  = _arr[5];
    mat_arr[6]  = _arr[6];
    mat_arr[7]  = _arr[7];

    mat_arr[8]  = _arr[8];
    mat_arr[9]  = _arr[9];
    mat_arr[10] = _arr[10];
    mat_arr[11] = _arr[11];

    mat_arr[12] = _arr[12];
    mat_arr[13] = _arr[13];
    mat_arr[14] = _arr[14];
    mat_arr[15] = _arr[15];

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

    mat_arr[0]  = _arr[0][0];
    mat_arr[1]  = _arr[0][1];
    mat_arr[2]  = _arr[0][2];
    mat_arr[3]  = _arr[0][3];

    mat_arr[4]  = _arr[1][0];
    mat_arr[5]  = _arr[1][1];
    mat_arr[6]  = _arr[1][2];
    mat_arr[7]  = _arr[1][3];

    mat_arr[8]  = _arr[2][0];
    mat_arr[9]  = _arr[2][1];
    mat_arr[10] = _arr[2][2];
    mat_arr[11] = _arr[2][3];

    mat_arr[12] = _arr[3][0];
    mat_arr[13] = _arr[3][1];
    mat_arr[14] = _arr[3][2];
    mat_arr[15] = _arr[3][3];

    return;
}

template <class _T>
matrix4_t<_T>::matrix4_t(const vector4_t<_T>& _v) {
    mat_arr.fill(0);
    mat_arr[0]  = _v.x;
    mat_arr[5]  = _v.y;
    mat_arr[10] = _v.z;
    mat_arr[15] = _v.w;
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
    mat_arr = _mat.mat_arr;
    return *this;
}

template <class _T>
bool matrix4_t<_T>::operator==(const matrix4_t<_T>& _mat) const {
    return mat_arr == _mat.mat_arr;
}

template <class _T>
bool matrix4_t<_T>::operator!=(const matrix4_t<_T>& _mat) const {
    return mat_arr != _mat.mat_arr;
}

template <class _T>
const matrix4_t<_T> matrix4_t<_T>::operator+(const matrix4_t<_T>& _mat) const {
    _T tmp[ORDER * ORDER] = { 0 };

    tmp[0]                = mat_arr[0] + _mat.mat_arr[0];
    tmp[1]                = mat_arr[1] + _mat.mat_arr[1];
    tmp[2]                = mat_arr[2] + _mat.mat_arr[2];
    tmp[3]                = mat_arr[3] + _mat.mat_arr[3];

    tmp[4]                = mat_arr[4] + _mat.mat_arr[4];
    tmp[5]                = mat_arr[5] + _mat.mat_arr[5];
    tmp[6]                = mat_arr[6] + _mat.mat_arr[6];
    tmp[7]                = mat_arr[7] + _mat.mat_arr[7];

    tmp[8]                = mat_arr[8] + _mat.mat_arr[8];
    tmp[9]                = mat_arr[9] + _mat.mat_arr[9];
    tmp[10]               = mat_arr[10] + _mat.mat_arr[10];
    tmp[11]               = mat_arr[11] + _mat.mat_arr[11];

    tmp[12]               = mat_arr[12] + _mat.mat_arr[12];
    tmp[13]               = mat_arr[13] + _mat.mat_arr[13];
    tmp[14]               = mat_arr[14] + _mat.mat_arr[14];
    tmp[15]               = mat_arr[15] + _mat.mat_arr[15];

    return matrix4_t<_T>(tmp);
}

template <class _T>
matrix4_t<_T>& matrix4_t<_T>::operator+=(const matrix4_t<_T>& _mat) {
    mat_arr[0]  += _mat.mat_arr[0];
    mat_arr[1]  += _mat.mat_arr[1];
    mat_arr[2]  += _mat.mat_arr[2];
    mat_arr[3]  += _mat.mat_arr[3];

    mat_arr[4]  += _mat.mat_arr[4];
    mat_arr[5]  += _mat.mat_arr[5];
    mat_arr[6]  += _mat.mat_arr[6];
    mat_arr[7]  += _mat.mat_arr[7];

    mat_arr[8]  += _mat.mat_arr[8];
    mat_arr[9]  += _mat.mat_arr[9];
    mat_arr[10] += _mat.mat_arr[10];
    mat_arr[11] += _mat.mat_arr[11];

    mat_arr[12] += _mat.mat_arr[12];
    mat_arr[13] += _mat.mat_arr[13];
    mat_arr[14] += _mat.mat_arr[14];
    mat_arr[15] += _mat.mat_arr[15];

    return *this;
}

template <class _T>
const matrix4_t<_T> matrix4_t<_T>::operator-(const matrix4_t<_T>& _mat) const {
    _T tmp[ORDER * ORDER] = { 0 };

    tmp[0]                = mat_arr[0] - _mat.mat_arr[0];
    tmp[1]                = mat_arr[1] - _mat.mat_arr[1];
    tmp[2]                = mat_arr[2] - _mat.mat_arr[2];
    tmp[3]                = mat_arr[3] - _mat.mat_arr[3];

    tmp[4]                = mat_arr[4] - _mat.mat_arr[4];
    tmp[5]                = mat_arr[5] - _mat.mat_arr[5];
    tmp[6]                = mat_arr[6] - _mat.mat_arr[6];
    tmp[7]                = mat_arr[7] - _mat.mat_arr[7];

    tmp[8]                = mat_arr[8] - _mat.mat_arr[8];
    tmp[9]                = mat_arr[9] - _mat.mat_arr[9];
    tmp[10]               = mat_arr[10] - _mat.mat_arr[10];
    tmp[11]               = mat_arr[11] - _mat.mat_arr[11];

    tmp[12]               = mat_arr[12] - _mat.mat_arr[12];
    tmp[13]               = mat_arr[13] - _mat.mat_arr[13];
    tmp[14]               = mat_arr[14] - _mat.mat_arr[14];
    tmp[15]               = mat_arr[15] - _mat.mat_arr[15];

    return matrix4_t<_T>(tmp);
}

template <class _T>
matrix4_t<_T>& matrix4_t<_T>::operator-=(const matrix4_t<_T>& _mat) {
    mat_arr[0]  -= _mat.mat_arr[0];
    mat_arr[1]  -= _mat.mat_arr[1];
    mat_arr[2]  -= _mat.mat_arr[2];
    mat_arr[3]  -= _mat.mat_arr[3];

    mat_arr[4]  -= _mat.mat_arr[4];
    mat_arr[5]  -= _mat.mat_arr[5];
    mat_arr[6]  -= _mat.mat_arr[6];
    mat_arr[7]  -= _mat.mat_arr[7];

    mat_arr[8]  -= _mat.mat_arr[8];
    mat_arr[9]  -= _mat.mat_arr[9];
    mat_arr[10] -= _mat.mat_arr[10];
    mat_arr[11] -= _mat.mat_arr[11];

    mat_arr[12] -= _mat.mat_arr[12];
    mat_arr[13] -= _mat.mat_arr[13];
    mat_arr[14] -= _mat.mat_arr[14];
    mat_arr[15] -= _mat.mat_arr[15];

    return *this;
}

template <class _T>
const matrix4_t<_T> matrix4_t<_T>::operator*(const matrix4_t<_T>& _mat) const {
    _T tmp[ORDER * ORDER] = { 0 };
    tmp[0] = mat_arr[0] * _mat.mat_arr[0] + mat_arr[1] * _mat.mat_arr[4]
           + mat_arr[2] * _mat.mat_arr[8] + mat_arr[3] * _mat.mat_arr[12];
    tmp[1] = mat_arr[0] * _mat.mat_arr[1] + mat_arr[1] * _mat.mat_arr[5]
           + mat_arr[2] * _mat.mat_arr[9] + mat_arr[3] * _mat.mat_arr[13];
    tmp[2] = mat_arr[0] * _mat.mat_arr[2] + mat_arr[1] * _mat.mat_arr[6]
           + mat_arr[2] * _mat.mat_arr[10] + mat_arr[3] * _mat.mat_arr[14];
    tmp[3] = mat_arr[0] * _mat.mat_arr[3] + mat_arr[1] * _mat.mat_arr[7]
           + mat_arr[2] * _mat.mat_arr[11] + mat_arr[3] * _mat.mat_arr[15];

    tmp[4] = mat_arr[4] * _mat.mat_arr[0] + mat_arr[5] * _mat.mat_arr[4]
           + mat_arr[6] * _mat.mat_arr[8] + mat_arr[7] * _mat.mat_arr[12];
    tmp[5] = mat_arr[4] * _mat.mat_arr[1] + mat_arr[5] * _mat.mat_arr[5]
           + mat_arr[6] * _mat.mat_arr[9] + mat_arr[7] * _mat.mat_arr[13];
    tmp[6] = mat_arr[4] * _mat.mat_arr[2] + mat_arr[5] * _mat.mat_arr[6]
           + mat_arr[6] * _mat.mat_arr[10] + mat_arr[7] * _mat.mat_arr[14];
    tmp[7] = mat_arr[4] * _mat.mat_arr[3] + mat_arr[5] * _mat.mat_arr[7]
           + mat_arr[6] * _mat.mat_arr[11] + mat_arr[7] * _mat.mat_arr[15];

    tmp[8] = mat_arr[8] * _mat.mat_arr[0] + mat_arr[9] * _mat.mat_arr[4]
           + mat_arr[10] * _mat.mat_arr[8] + mat_arr[11] * _mat.mat_arr[12];
    tmp[9] = mat_arr[8] * _mat.mat_arr[1] + mat_arr[9] * _mat.mat_arr[5]
           + mat_arr[10] * _mat.mat_arr[9] + mat_arr[11] * _mat.mat_arr[13];
    tmp[10] = mat_arr[8] * _mat.mat_arr[2] + mat_arr[9] * _mat.mat_arr[6]
            + mat_arr[10] * _mat.mat_arr[10] + mat_arr[11] * _mat.mat_arr[14];
    tmp[11] = mat_arr[8] * _mat.mat_arr[3] + mat_arr[9] * _mat.mat_arr[7]
            + mat_arr[10] * _mat.mat_arr[11] + mat_arr[11] * _mat.mat_arr[15];

    tmp[12] = mat_arr[12] * _mat.mat_arr[0] + mat_arr[13] * _mat.mat_arr[4]
            + mat_arr[14] * _mat.mat_arr[8] + mat_arr[15] * _mat.mat_arr[12];
    tmp[13] = mat_arr[12] * _mat.mat_arr[1] + mat_arr[13] * _mat.mat_arr[5]
            + mat_arr[14] * _mat.mat_arr[9] + mat_arr[15] * _mat.mat_arr[13];
    tmp[14] = mat_arr[12] * _mat.mat_arr[2] + mat_arr[13] * _mat.mat_arr[6]
            + mat_arr[14] * _mat.mat_arr[10] + mat_arr[15] * _mat.mat_arr[14];
    tmp[15] = mat_arr[12] * _mat.mat_arr[3] + mat_arr[13] * _mat.mat_arr[7]
            + mat_arr[14] * _mat.mat_arr[11] + mat_arr[15] * _mat.mat_arr[15];

    return matrix4_t<_T>(tmp);
}

template <class _T>
matrix4_t<_T>& matrix4_t<_T>::operator*=(const _T& _v) {
    for (uint8_t i = 0; i < ORDER * ORDER; i++) {
        mat_arr[i] *= _v;
    }

    return *this;
}

template <class _T>
matrix4_t<_T>& matrix4_t<_T>::operator*=(const matrix4_t<_T>& _mat) {
    _T tmp[ORDER * ORDER] = { 0 };
    tmp[0] = mat_arr[0] * _mat.mat_arr[0] + mat_arr[1] * _mat.mat_arr[4]
           + mat_arr[2] * _mat.mat_arr[8] + mat_arr[3] * _mat.mat_arr[12];
    tmp[1] = mat_arr[0] * _mat.mat_arr[1] + mat_arr[1] * _mat.mat_arr[5]
           + mat_arr[2] * _mat.mat_arr[9] + mat_arr[3] * _mat.mat_arr[13];
    tmp[2] = mat_arr[0] * _mat.mat_arr[2] + mat_arr[1] * _mat.mat_arr[6]
           + mat_arr[2] * _mat.mat_arr[10] + mat_arr[3] * _mat.mat_arr[14];
    tmp[3] = mat_arr[0] * _mat.mat_arr[3] + mat_arr[1] * _mat.mat_arr[7]
           + mat_arr[2] * _mat.mat_arr[11] + mat_arr[3] * _mat.mat_arr[15];

    tmp[4] = mat_arr[4] * _mat.mat_arr[0] + mat_arr[5] * _mat.mat_arr[4]
           + mat_arr[6] * _mat.mat_arr[8] + mat_arr[7] * _mat.mat_arr[12];
    tmp[5] = mat_arr[4] * _mat.mat_arr[1] + mat_arr[5] * _mat.mat_arr[5]
           + mat_arr[6] * _mat.mat_arr[9] + mat_arr[7] * _mat.mat_arr[13];
    tmp[6] = mat_arr[4] * _mat.mat_arr[2] + mat_arr[5] * _mat.mat_arr[6]
           + mat_arr[6] * _mat.mat_arr[10] + mat_arr[7] * _mat.mat_arr[14];
    tmp[7] = mat_arr[4] * _mat.mat_arr[3] + mat_arr[5] * _mat.mat_arr[7]
           + mat_arr[6] * _mat.mat_arr[11] + mat_arr[7] * _mat.mat_arr[15];

    tmp[8] = mat_arr[8] * _mat.mat_arr[0] + mat_arr[9] * _mat.mat_arr[4]
           + mat_arr[10] * _mat.mat_arr[8] + mat_arr[11] * _mat.mat_arr[12];
    tmp[9] = mat_arr[8] * _mat.mat_arr[1] + mat_arr[9] * _mat.mat_arr[5]
           + mat_arr[10] * _mat.mat_arr[9] + mat_arr[11] * _mat.mat_arr[13];
    tmp[10] = mat_arr[8] * _mat.mat_arr[2] + mat_arr[9] * _mat.mat_arr[6]
            + mat_arr[10] * _mat.mat_arr[10] + mat_arr[11] * _mat.mat_arr[14];
    tmp[11] = mat_arr[8] * _mat.mat_arr[3] + mat_arr[9] * _mat.mat_arr[7]
            + mat_arr[10] * _mat.mat_arr[11] + mat_arr[11] * _mat.mat_arr[15];

    tmp[12] = mat_arr[12] * _mat.mat_arr[0] + mat_arr[13] * _mat.mat_arr[4]
            + mat_arr[14] * _mat.mat_arr[8] + mat_arr[15] * _mat.mat_arr[12];
    tmp[13] = mat_arr[12] * _mat.mat_arr[1] + mat_arr[13] * _mat.mat_arr[5]
            + mat_arr[14] * _mat.mat_arr[9] + mat_arr[15] * _mat.mat_arr[13];
    tmp[14] = mat_arr[12] * _mat.mat_arr[2] + mat_arr[13] * _mat.mat_arr[6]
            + mat_arr[14] * _mat.mat_arr[10] + mat_arr[15] * _mat.mat_arr[14];
    tmp[15] = mat_arr[12] * _mat.mat_arr[3] + mat_arr[13] * _mat.mat_arr[7]
            + mat_arr[14] * _mat.mat_arr[11] + mat_arr[15] * _mat.mat_arr[15];

    mat_arr[0]  = tmp[0];
    mat_arr[1]  = tmp[1];
    mat_arr[2]  = tmp[2];
    mat_arr[3]  = tmp[3];

    mat_arr[4]  = tmp[4];
    mat_arr[5]  = tmp[5];
    mat_arr[6]  = tmp[6];
    mat_arr[7]  = tmp[7];

    mat_arr[8]  = tmp[8];
    mat_arr[9]  = tmp[9];
    mat_arr[10] = tmp[10];
    mat_arr[11] = tmp[11];

    mat_arr[12] = tmp[12];
    mat_arr[13] = tmp[13];
    mat_arr[14] = tmp[14];
    mat_arr[15] = tmp[15];

    return *this;
}

template <class _T>
_T matrix4_t<_T>::operator[](const uint8_t _idx) {
    if (_idx > ORDER) {
        throw std::invalid_argument(log("_idx > ORDER"));
    }
    return mat_arr[_idx];
}

template <class _T>
const _T matrix4_t<_T>::operator[](const uint8_t _idx) const {
    if (_idx > ORDER) {
        throw std::invalid_argument(log("_idx > ORDER"));
    }
    return mat_arr[_idx];
}

template <class _T>
const matrix4_t<_T> matrix4_t<_T>::transpose(void) const {
    _T tmp[ORDER * ORDER] = { 0 };
    tmp[0]                = mat_arr[0];
    tmp[1]                = mat_arr[4];
    tmp[2]                = mat_arr[8];
    tmp[3]                = mat_arr[12];

    tmp[4]                = mat_arr[1];
    tmp[5]                = mat_arr[5];
    tmp[6]                = mat_arr[9];
    tmp[7]                = mat_arr[13];

    tmp[8]                = mat_arr[2];
    tmp[9]                = mat_arr[6];
    tmp[10]               = mat_arr[10];
    tmp[11]               = mat_arr[14];

    tmp[12]               = mat_arr[3];
    tmp[13]               = mat_arr[7];
    tmp[14]               = mat_arr[11];
    tmp[15]               = mat_arr[15];

    return matrix4_t<_T>(tmp);
}

template <class _T>
const matrix4_t<_T> matrix4_t<_T>::inverse(void) const {
    // 计算行列式
    _T det = determ(ORDER);
    if (std::abs(det) <= std::numeric_limits<_T>::epsilon()) {
        throw std::runtime_error(
          log("std::abs(det) <= std::numeric_limits<_T>::epsilon(), Singular "
              "matrix, can't find its inverse"));
    }

    // 逆矩阵 = 伴随矩阵 / 行列式
    auto tmp = adjoint() * (1 / det);

    return matrix4_t<_T>(tmp);
}

template <class _T>
const matrix4_t<_T> matrix4_t<_T>::scale(const _T& _scale) const {
    if (std::isnan(_scale)) {
        throw std::invalid_argument(log("std::isnan(_scale)"));
    }

    _T tmp[ORDER * ORDER] = { 0 };

    tmp[0]                = _scale * mat_arr[0];
    tmp[1]                = _scale * mat_arr[1];
    tmp[2]                = _scale * mat_arr[2];
    tmp[3]                = _scale * mat_arr[3];

    tmp[4]                = _scale * mat_arr[4];
    tmp[5]                = _scale * mat_arr[5];
    tmp[6]                = _scale * mat_arr[6];
    tmp[7]                = _scale * mat_arr[7];

    tmp[8]                = _scale * mat_arr[8];
    tmp[9]                = _scale * mat_arr[9];
    tmp[10]               = _scale * mat_arr[10];
    tmp[11]               = _scale * mat_arr[11];

    tmp[12]               = mat_arr[12];
    tmp[13]               = mat_arr[13];
    tmp[14]               = mat_arr[14];
    tmp[15]               = mat_arr[15];

    return matrix4_t<_T>(tmp);
}

template <class _T>
const matrix4_t<_T>
matrix4_t<_T>::scale(const _T& _x, const _T& _y, const _T& _z) const {
    if (std::isnan(_x) || std::isnan(_y) || std::isnan(_z)) {
        throw std::invalid_argument(
          log("std::isnan(_x) || std::isnan(_y) || std::isnan(_z)"));
    }

    _T tmp[ORDER * ORDER] = { 0 };

    tmp[0]                = _x * mat_arr[0];
    tmp[1]                = _x * mat_arr[1];
    tmp[2]                = _x * mat_arr[2];
    tmp[3]                = _x * mat_arr[3];

    tmp[4]                = _y * mat_arr[4];
    tmp[5]                = _y * mat_arr[5];
    tmp[6]                = _y * mat_arr[6];
    tmp[7]                = _y * mat_arr[7];

    tmp[8]                = _z * mat_arr[8];
    tmp[9]                = _z * mat_arr[9];
    tmp[10]               = _z * mat_arr[10];
    tmp[11]               = _z * mat_arr[11];

    tmp[12]               = mat_arr[12];
    tmp[13]               = mat_arr[13];
    tmp[14]               = mat_arr[14];
    tmp[15]               = mat_arr[15];

    return matrix4_t<_T>(tmp);
}

/// @todo 精度问题
template <class _T>
const matrix4_t<_T>
matrix4_t<_T>::rotate(const vector4_t<_T>& _axis, const float& _angle) const {
    if (std::isnan(_angle)) {
        throw std::invalid_argument(log("std::isnan(_angle)"));
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
    res.mat_arr[15]   = 1;

    return res * *this;
}

/// @todo 精度问题
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

    _T   tmp[ORDER * ORDER] = { 0 };

    tmp[0]                  = vt.x * v.x + ca;
    tmp[5]                  = vt.y * v.y + ca;
    tmp[10]                 = vt.z * v.z + ca;

    vt.x                    *= v.y;
    vt.z                    *= v.x;
    vt.y                    *= v.z;

    tmp[1]                  = vt.x + vs.z;
    tmp[2]                  = vt.z - vs.y;
    tmp[3]                  = 0;

    tmp[4]                  = vt.x - vs.z;
    tmp[6]                  = vt.y + vs.x;
    tmp[7]                  = 0;

    tmp[8]                  = vt.z + vs.y;
    tmp[9]                  = vt.y - vs.x;
    tmp[11]                 = 0;

    tmp[12]                 = 0;
    tmp[13]                 = 0;
    tmp[14]                 = 0;
    tmp[15]                 = 1;

    return matrix4_t<_T>(tmp);
}

template <class _T>
const matrix4_t<_T>
matrix4_t<_T>::translate(const _T& _x, const _T& _y, const _T& _z) const {
    if (std::isnan(_x) || std::isnan(_y) || std::isnan(_z)) {
        throw std::invalid_argument(
          log("std::isnan(_x) || std::isnan(_y) || std::isnan(_z)"));
    }

    _T tmp[ORDER * ORDER] = { 0 };

    tmp[0]                = mat_arr[0] + _x * mat_arr[12];
    tmp[1]                = mat_arr[1] + _x * mat_arr[13];
    tmp[2]                = mat_arr[2] + _x * mat_arr[14];
    tmp[3]                = mat_arr[3] + _x * mat_arr[15];

    tmp[4]                = mat_arr[4] + _y * mat_arr[12];
    tmp[5]                = mat_arr[5] + _y * mat_arr[13];
    tmp[6]                = mat_arr[6] + _y * mat_arr[14];
    tmp[7]                = mat_arr[7] + _y * mat_arr[15];

    tmp[8]                = mat_arr[8] + _z * mat_arr[12];
    tmp[9]                = mat_arr[9] + _z * mat_arr[13];
    tmp[10]               = mat_arr[10] + _z * mat_arr[14];
    tmp[11]               = mat_arr[11] + _z * mat_arr[15];

    tmp[12]               = mat_arr[12];
    tmp[13]               = mat_arr[13];
    tmp[14]               = mat_arr[14];
    tmp[15]               = mat_arr[15];

    return matrix4_t<_T>(tmp);
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
