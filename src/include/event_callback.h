
/**
 * @file event_callback.h
 * @brief 事件处理
 * @author Zone.N (Zone.Niuzh@hotmail.com)
 * @version 1.0
 * @date 2022-11-01
 * @copyright MIT LICENSE
 * https://github.com/Simple-XX/SimpleRenderer
 * @par change log:
 * <table>
 * <tr><th>Date<th>Author<th>Description
 * <tr><td>2022-11-01<td>Zone.N<td>创建文件
 * </table>
 */

#ifndef _EVENT_CALLBACK_H_
#define _EVENT_CALLBACK_H_

#include "camera.h"
#include "config.h"

class event_callback_t {
private:
    config_t& config;
    camera_t& camera;

public:
    event_callback_t(void) = delete;
    event_callback_t(config_t& _config, camera_t& _camera);
    ~event_callback_t(void);

    void key_a(void);
    void key_d(void);
    void key_w(void);
    void key_s(void);
    void key_z(void);
    void key_space(void);
    void key_left_ctrl(void);
    void key_left_shift(void);
    void key_mouse_motion(void);
};

#endif /* _EVENT_CALLBACK_H_ */
