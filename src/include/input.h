
/**
 * @file input.h
 * @brief 输入处理
 * @author Zone.N (Zone.Niuzh@hotmail.com)
 * @version 1.0
 * @date 2022-12-15
 * @copyright MIT LICENSE
 * https://github.com/Simple-XX/SimpleRenderer
 * @par change log:
 * <table>
 * <tr><th>Date<th>Author<th>Description
 * <tr><td>2022-12-15<td>Zone.N<td>创建文件
 * </table>
 */

#ifndef SIMPLERENDER_INPUT_H
#define SIMPLERENDER_INPUT_H

#include "memory"

#include "SDL.h"

#include "scene.h"

/**
 * @brief 输入处理
 */
class input_t {
private:
    /// @brief SDL 事件
    SDL_Event    event;

    /**
     * @brief a 键
     * @param  _scene           要应用到的场景
     * @param  _delta_time      时间变化
     */
    virtual void key_a(scene_t& _scene, const uint32_t _delta_time) const;

    /**
     * @brief d 键
     * @param  _scene           要应用到的场景
     * @param  _delta_time      时间变化
     */
    virtual void key_d(scene_t& _scene, const uint32_t _delta_time) const;

    /**
     * @brief w 键
     * @param  _scene           要应用到的场景
     * @param  _delta_time      时间变化
     */
    virtual void key_w(scene_t& _scene, const uint32_t _delta_time) const;

    /**
     * @brief s 键
     * @param  _scene           要应用到的场景
     * @param  _delta_time      时间变化
     */
    virtual void key_s(scene_t& _scene, const uint32_t _delta_time) const;

    /**
     * @brief z 键
     * @param  _scene           要应用到的场景
     * @param  _delta_time      时间变化
     */
    virtual void key_z(scene_t& _scene, const uint32_t _delta_time) const;

    /**
     * @brief r 键
     * @param  _scene           要应用到的场景
     * @param  _delta_time      时间变化
     */
    virtual void key_r(scene_t& _scene, const uint32_t _delta_time) const;

    /**
     * @brief q 键
     * @param  _scene           要应用到的场景
     * @param  _delta_time      时间变化
     */
    virtual void key_q(scene_t& _scene, const uint32_t _delta_time) const;

    /**
     * @brief e 键
     * @param  _scene           要应用到的场景
     * @param  _delta_time      时间变化
     */
    virtual void key_e(scene_t& _scene, const uint32_t _delta_time) const;

    /**
     * @brief space 键
     * @param  _scene           要应用到的场景
     * @param  _delta_time      时间变化
     */
    virtual void key_space(scene_t& _scene, const uint32_t _delta_time) const;

    /**
     * @brief left_ctrl 键
     * @param  _scene           要应用到的场景
     * @param  _delta_time      时间变化
     */
    virtual void
    key_left_ctrl(scene_t& _scene, const uint32_t _delta_time) const;

    /**
     * @brief left_shift 键
     * @param  _scene           要应用到的场景
     * @param  _delta_time      时间变化
     */
    virtual void
    key_left_shift(scene_t& _scene, const uint32_t _delta_time) const;

    /**
     * @brief 鼠标移动
     * @param  _scene           要应用到的场景
     * @param  _x               横坐标变化
     * @param  _y               纵坐标变化
     * @param  _delta_time      时间变化
     */
    virtual void
    mouse_motion(scene_t& _scene, const int32_t _x, const int32_t _y,
                 const uint32_t _delta_time) const;

    /**
     * @brief 处理输入
     * @param  _scene           要应用到的场景
     * @param  _delta_time      时间变化
     * @return true             不退出
     * @return false            需要退出
     */
    virtual bool handle(scene_t& _scene, const uint32_t _delta_time) const;

public:
    /**
     * @brief 构造函数
     */
    input_t(void);

    /**
     * @brief 析构函数
     */
    virtual ~input_t(void);

    /**
     * @brief 处理输入, 如果程序退出返回 false
     * @param  _scene           要应用到的场景
     * @param  _delta_time      时间变化
     * @return true             不退出
     * @return false            退出
     */
    bool process(scene_t& _scene, const uint32_t _delta_time);
};

#endif /* SIMPLERENDER_INPUT_H */
