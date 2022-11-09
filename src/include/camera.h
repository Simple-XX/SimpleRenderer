
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
 * @brief 相机基类
 */
class camera_base_t {
private:
    /// @brief 默认在位置，在原点
    const vector4f_t default_pos    = vector4f_t(0, 0, 0);
    /// @brief 默认看向屏幕内
    const vector4f_t default_target = vector4f_t(0, 0, -1);
    /// @brief 默认上方向为屏幕向上
    const vector4f_t default_up     = vector4f_t(0, -1, 0);
    /// @brief 默认向右为正方向
    const vector4f_t default_right  = vector4f_t(1, 0, 0);
    /// @brief 屏幕宽高比
    const float      default_aspect = 1;
    /// @brief 相机移动速度
    const float      default_speed  = 1;

protected:
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
    camera_base_t(void);

    /**
     * @brief 构造函数
     * @param  _camera          另一个 camera_base_t
     */
    explicit camera_base_t(const camera_base_t& _camera);

    /**
     * @brief 构造函数
     * @param  _pos             位置
     * @param  _target          方向
     * @param  _aspect          比例
     */
    explicit camera_base_t(const vector4f_t& _pos, const vector4f_t& _target,
                           const float _aspect);

    /**
     * @brief 析构函数
     */
    virtual ~camera_base_t(void);

    /**
     * @brief = 重载
     * @param  _camera          另一个 camera_base_t
     * @return camera_base_t&   结果
     */
    camera_base_t&           operator=(const camera_base_t& _camera);

    /// @todo 补全注释
    virtual void             set_default(void);
    virtual const matrix4f_t look_at(void);

    virtual void             update_pos_x(const bool _is_positive);
    virtual void             update_pos_y(const bool _is_positive);
    virtual void             update_pos_z(const bool _is_positive);
    /// @todo 完善
    virtual void             update_up_x(const bool _is_positive);
    /// @todo 完善
    virtual void             update_up_y(const bool _is_positive);
    virtual void             update_target(const int32_t _x, const int32_t _y);
};

/**
 * @brief 环绕相机
 */
class surround_camera_t : public camera_base_t {
private:
    /// @brief 默认在位置，在原点
    const vector4f_t default_pos    = vector4f_t(0, 0, 0);
    /// @brief 默认看向屏幕内
    const vector4f_t default_target = vector4f_t(0, 0, -1);
    /// @brief 默认上方向为屏幕向上
    const vector4f_t default_up     = vector4f_t(0, -1, 0);
    /// @brief 默认向右为正方向
    const vector4f_t default_right  = vector4f_t(1, 0, 0);
    /// @brief 屏幕宽高比
    const float      default_aspect = 1;
    /// @brief 相机环绕 1 度
    const float      default_speed  = ((M_PI / 180) * (1));
    // const float      default_speed  = 1;

public:
    /**
     * @brief 构造函数
     */
    surround_camera_t(void);

    /**
     * @brief 构造函数
     * @param  _camera          另一个 surround_camera_t
     */
    explicit surround_camera_t(const surround_camera_t& _camera);

    /**
     * @brief 构造函数
     * @param  _pos             位置
     * @param  _target          方向
     * @param  _aspect          比例
     */
    explicit surround_camera_t(const vector4f_t& _pos,
                               const vector4f_t& _target, const float _aspect);

    /**
     * @brief 析构函数
     */
    ~surround_camera_t(void);

    /**
     * @brief = 重载
     * @param  _camera          另一个 surround_camera_t
     * @return surround_camera_t& 结果
     */
    surround_camera_t& operator=(const surround_camera_t& _camera);

    /// @todo 补全注释
    void               set_default(void) override;
    void               update_pos_x(const bool _is_positive) override;
    void               update_pos_y(const bool _is_positive) override;
    void               update_pos_z(const bool _is_positive) override;
    /// @todo 完善
    void               update_up_x(const bool _is_positive) override;
    /// @todo 完善
    void               update_up_y(const bool _is_positive) override;
    void update_target(const int32_t _x, const int32_t _y) override;
};

#endif /* _CAMERA_H_ */
