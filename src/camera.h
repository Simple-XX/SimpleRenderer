
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
#include "vector.hpp"
#include "matrix.hpp"

class camera_t {
private:
    /// 位置
    vector3f_t pos;
    /// 方向
    vector3f_t target;
    /// 比例
    float aspect;

public:
    camera_t(void);
    camera_t(const camera_t &_camera);
    camera_t(const vector3f_t &_pos, const vector3f_t &_target,
             const float _aspect);
    camera_t        &operator=(const camera_t &_camera);
    bool             operator==(const camera_t &_camera) const;
    bool             operator!=(const camera_t &_camera) const;
    const vector3f_t operator*(const vector3f_t &_v) const;
    const matrix4f_t operator*(const matrix4f_t &_m) const;

    vector3f_t &get_pos(void);
    vector3f_t &get_target(void);
    float       get_aspect(void) const;
};

static camera_t camera;

#endif /* _CAMERA_H_ */