
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

/**
 * @brief 相机抽象
 */
class camera_t {
private:
    /// @brief 默认在原点
    const vector4f_t pos_default    = vector4f_t(0, 0, 0);
    /// @brief 看向屏幕内
    const vector4f_t target_default = vector4f_t(0, 0, -1);
    /// @brief 上方向为屏幕向上
    const vector4f_t up_default     = vector4f_t(0, -1, 0);
    /// @brief 向右为正方向
    const vector4f_t right_default  = vector4f_t(1, 0, 0);
    /// @brief 屏幕宽高比
    const float      aspect_default = 1;
    /// @brief 相机移动速度
    const float      speed_default  = 20;

public:
    /// @brief 位置
    vector4f_t pos;
    /// @brief 方向
    vector4f_t target;
    /// @brief 上方向
    vector4f_t up;
    /// @brief 比例
    float      aspect;
    /// @brief 相机速度
    float      speed = 20;

public:
    /**
     * @brief 构造函数
     */
    camera_t(void);

    /**
     * @brief 构造函数
     * @param  _camera          另一个 camera_t
     */
    explicit camera_t(const camera_t& _camera);

    /**
     * @brief 构造函数
     * @param  _pos             位置
     * @param  _target          方向
     * @param  _aspect          比例
     */
    camera_t(const vector4f_t& _pos, const vector4f_t& _target,
             const float _aspect);

    /**
     * @brief = 重载
     * @param  _camera          另一个 camera_t
     * @return camera_t&        结果
     */
    camera_t&        operator=(const camera_t& _camera);

    /// @todo 补全注释
    vector4f_t&      get_pos(void);
    vector4f_t&      get_target(void);
    vector4f_t&      get_up(void);
    float            get_aspect(void);
    void             set_default(void);
    const matrix4f_t look_at(void);
};

#endif /* _CAMERA_H_ */