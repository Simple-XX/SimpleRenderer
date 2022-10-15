
/**
 * @file camera.h
 * @brief 相机抽象
 * @author Zone.N (Zone.Niuzh@hotmail.com)
 * @version 1.0
 * @date 2022-09-19
 * @copyright MIT LICENSE
 * https://github.com/Simple-XX/SimpleRenderer
 * @par change log:
 * <table>
 * <tr><th>Date<th>Author<th>Description
 * <tr><td>2022-09-19<td>Zone.N<td>创建文件
 * </table>
 */

#ifndef _CAMERA_H_
#define _CAMERA_H_

#include "log.hpp"
#include "matrix.hpp"
#include "vector.hpp"

class camera_t {
private:

public:
    /// 位置
    vector4f_t pos;
    /// 方向
    vector4f_t target;
    /// 上方向
    vector4f_t up;
    /// 比例
    float      aspect;

public:
    camera_t(void);
    camera_t(const camera_t& _camera);
    camera_t(const vector4f_t& _pos, const vector4f_t& _target,
             const float _aspect);
    camera_t&        operator=(const camera_t& _camera);
    bool             operator==(const camera_t& _camera) const;
    bool             operator!=(const camera_t& _camera) const;
    const vector4f_t operator*(const vector4f_t& _v) const;
    const matrix4f_t operator*(const matrix4f_t& _m) const;

    vector4f_t&      get_pos(void);
    vector4f_t&      get_target(void);
    vector4f_t&      get_up(void);
    float            get_aspect(void);
};

extern camera_t camera;

#endif /* _CAMERA_H_ */