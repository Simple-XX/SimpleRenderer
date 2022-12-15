
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

#ifndef _INPUT_H_
#define _INPUT_H_

#include "memory"

#include "SDL.h"

#include "camera.h"
#include "config.h"

/**
 * @brief 输入处理
 */
class input_t {
private:
    /// @brief 配置信息
    std::shared_ptr<config_t>      config;
    /// @brief 要控制的相机
    std::shared_ptr<camera_base_t> camera;

    /**
     * @brief a 键
     * @param  _delta_time      时间变化
     */
    void                           key_a(const uint32_t _delta_time);

    /**
     * @brief d 键
     * @param  _delta_time      时间变化
     */
    void                           key_d(const uint32_t _delta_time);

    /**
     * @brief w 键
     * @param  _delta_time      时间变化
     */
    void                           key_w(const uint32_t _delta_time);

    /**
     * @brief s 键
     * @param  _delta_time      时间变化
     */
    void                           key_s(const uint32_t _delta_time);

    /**
     * @brief z 键
     * @param  _delta_time      时间变化
     */
    void                           key_z(const uint32_t _delta_time);

    /**
     * @brief r 键
     * @param  _delta_time      时间变化
     */
    void                           key_r(const uint32_t _delta_time);

    /**
     * @brief q 键
     * @param  _delta_time      时间变化
     */
    void                           key_q(const uint32_t _delta_time);

    /**
     * @brief e 键
     * @param  _delta_time      时间变化
     */
    void                           key_e(const uint32_t _delta_time);

    /**
     * @brief space 键
     * @param  _delta_time      时间变化
     */
    void                           key_space(const uint32_t _delta_time);

    /**
     * @brief left_ctrl 键
     * @param  _delta_time      时间变化
     */
    void                           key_left_ctrl(const uint32_t _delta_time);

    /**
     * @brief left_shift 键
     * @param  _delta_time      时间变化
     */
    void                           key_left_shift(const uint32_t _delta_time);

    /**
     * @brief 鼠标移动
     * @param  _x               横坐标变化
     * @param  _y               纵坐标变化
     * @param  _delta_time      时间变化
     */
    void mouse_motion(const int32_t _x, const int32_t _y,
                      const uint32_t _delta_time);

    /**
     * @brief 处理输入
     * @param  _event           sdl 事件
     * @param  _delta_time      时间变化
     * @return true             不退出
     * @return false            需要退出
     */
    bool handle(SDL_Event* _event, const uint32_t _delta_time);

public:
    /**
     * @brief 构造函数
     */
    input_t(void) = delete;

    /**
     * @brief 构造函数
     * @param  _config          配置信息
     * @param  _camera          要控制的相机
     */
    input_t(std::shared_ptr<config_t>      _config,
            std::shared_ptr<camera_base_t> _camera);

    /**
     * @brief 析构函数
     */
    ~input_t(void);

    /**
     * @brief 处理输入, 如果程序退出返回 false
     * @param  _delta_time      时间变化
     * @return true             不退出
     * @return false            退出
     */
    bool process(const uint32_t _delta_time);
};

#endif /* _INPUT_H_ */
