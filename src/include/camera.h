
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

#ifndef SIMPLERENDER_CAMERA_H
#define SIMPLERENDER_CAMERA_H

#include "log.hpp"
#include "matrix.hpp"
#include "vector.hpp"

/**
 * @brief 相机基类
 */
class camera_base_t {
private:

protected:
    /// @brief 默认在位置，在原点
    const vector4f_t DEFAULT_POS    = vector4f_t(0, 0, 0);
    /// @brief 默认看向屏幕内
    const vector4f_t DEFAULT_TARGET = vector4f_t(0, 0, 0);

    /// @brief 默认上方向为屏幕向上
    const vector4f_t DEFAULT_UP     = vector4f_t(0, -1, 0);
    /// @brief 默认指向屏幕内为正方向
    const vector4f_t DEFAULT_FRONT  = vector4f_t(0, 0, -1);
    /// @brief 右方向通过计算得出，默认为 0
    const vector4f_t DEFAULT_RIGHT  = vector4f_t(0, 0, 0);

    /// @brief 屏幕宽高比
    const float      DEFAULT_ASPECT = 1;
    /// @brief 相机移动速度
    const float      DEFAULT_SPEED  = 1;

    /// @brief 位置
    vector4f_t       pos;
    /// @brief 方向
    vector4f_t       target;

    /// @brief 上方向，单位向量
    vector4f_t       up;
    /// @brief 前方向，单位向量
    vector4f_t       front;
    /// @brief 右方向，单位向量，通过计算得出
    vector4f_t       right;

    /// @brief 比例
    float            aspect;
    /// @brief 相机速度
    float            speed;

public:
    /**
     * @brief 用于标识相机移动的方向
     */
    enum move_to_t : uint8_t {
        RIGHT,
        LEFT,
        UP,
        DOWN,
        FORWARD,
        BACKWARD,
    };

    /**
     * @brief 用于标识相机旋转的方向
     * @todo
     */
    enum course_to_t : uint8_t {
        PITCH_UP,
        PITCH_DOWN,
        YAW_LEFT,
        YAW_RIGHT,
        ROLL_1,
        ROLL_2,
    };

    /**
     * @brief 构造函数
     */
    camera_base_t(void);

    /**
     * @brief 构造函数
     * @param  _camera          另一个 camera_base_t
     */
    camera_base_t(const camera_base_t& _camera);

    /**
     * @brief 构造函数
     * @param  _pos             位置
     * @param  _target          方向
     * @param  _aspect          比例
     */
    explicit camera_base_t(const vector4f_t& _pos, const vector4f_t& _target,
                           float _aspect);

    /**
     * @brief 析构函数
     */
    virtual ~camera_base_t(void);

    /**
     * @brief = 重载
     * @param  _camera          另一个 camera_base_t
     * @return camera_base_t&   结果
     */
    camera_base_t& operator=(const camera_base_t& _camera);

    /**
     * @brief 相机复位
     */
    virtual void   set_default(void);

    /**
     * @brief 更新相机位置
     * @param  _to              移动的方向
     * @param  _delta_time      时间变化
     */
    virtual void       move(const move_to_t& _to, uint32_t _delta_time);

    /**
     * @brief 更新相机目标
     * @param  _to              移动的方向
     * @param  _delta_time      时间变化
     */
    virtual void   update_target(const move_to_t& _to, uint32_t _delta_time);

    /**
     * @brief 更新相机上方向
     * @param  _to              移动的方向
     * @param  _delta_time      时间变化
     */
    virtual void       update_up(const move_to_t& _to, uint32_t _delta_time);

    /**
     * @brief 获取视图变换矩阵, 将相机移动到原点，方向指向 -z，即屏幕里，up 为
     * -y，即屏幕向上
     * @return matrix4f_t       视图矩阵
     */
    virtual matrix4f_t look_at(void) const;
};

/**
 * @brief 环绕相机
 */
class surround_camera_t : public camera_base_t {
private:

public:
    /**
     * @brief 构造函数
     */
    surround_camera_t(void);

    /**
     * @brief 构造函数
     * @param  _camera          另一个 surround_camera_t
     */
    surround_camera_t(const surround_camera_t& _camera);

    /**
     * @brief 构造函数
     * @param  _pos             位置
     * @param  _target          方向
     * @param  _aspect          比例
     */
    explicit surround_camera_t(const vector4f_t& _pos,
                               const vector4f_t& _target, float _aspect);

    /**
     * @brief 析构函数
     */
    ~surround_camera_t(void) override;
};

#endif /* SIMPLERENDER_CAMERA_H */
