
/**
 * @file scend.h
 * @brief 场景抽象
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

#ifndef SIMPLERENDER_SCENE_H
#define SIMPLERENDER_SCENE_H

#include "cstdint"
#include "memory"
#include "queue"
#include "vector"

#include "camera.h"
#include "config.h"
#include "light.h"
#include "matrix.hpp"
#include "model.h"
#include "vector.hpp"

/**
 * @brief 场景抽象
 */
class scene_t {
private:
    /// @brief 配置文件
    std::shared_ptr<config_t>  config;

    /// @brief 场景中的摄像机
    std::vector<camera_base_t> cameras;
    /// @brief 当前使用的摄像机
    camera_base_t              current_camera;

    /// @brief 场景中的所有模型
    std::vector<model_t>       models;
    /// @brief 场景中的可见模型，即要渲染的模型队列
    std::queue<model_t>        visible_models;

    /// @brief 场景中的所有光照
    std::vector<light_t>       lights;

public:
    /**
     * @brief 空构造函数
     */
    scene_t(void) = delete;

    /**
     * @brief 构造函数
     * @param  _config          配置信息
     */
    explicit scene_t(const std::shared_ptr<config_t>& _config);

    /**
     * @brief 构造函数
     * @param  _scene           另一个 scene
     */
    explicit scene_t(const scene_t& _scene);

    /**
     * @brief 析构函数
     */
    ~scene_t(void);

    /**
     * @brief 赋值
     * @param  _scene           另一个 light
     * @return scene_t&         结果
     */
    scene_t&  operator=(const scene_t& _scene);

    /**
     * @brief 将模型添加到场景中，缩放到合适大小，移到屏幕中央
     * @param  _model           要添加的 model
     */
    void      add_model(const model_t& _model);

    /**
     * @brief 将模型添加到场景中
     * @param  _model           要添加的 model
     * @param  _model_matrix    模型变换矩阵变换
     */
    void      add_model(const model_t& _model, const matrix4f_t& _model_matrix);

    /**
     * @brief 将光照添加到场景中
     * @param  _light           要添加的 light
     */
    void      add_light(const light_t& _light);

    /**
     * @brief 更新场景，根据时间变化更新，返回是否继续运行
     * @param  _delta_time      时间变化
     * @return true             运行
     * @return false            结束
     */
    bool      tick(const uint32_t _delta_time);

    /**
     * @brief 获取场景的配置信息
     * @return config_t&        场景的配置信息
     */
    config_t& get_config(void);

    /**
     * @brief 获取场景的配置信息
     * @return const config_t&  场景的配置信息
     */
    const config_t&            get_config(void) const;

    /**
     * @brief 获取当前相机
     * @return camera_base_t&   要渲染的模型队列
     */
    camera_base_t&             get_current_camera(void);

    /**
     * @brief 获取当前相机
     * @return const camera_base_t& 要渲染的模型队列
     */
    const camera_base_t&       get_current_camera(void) const;

    /**
     * @brief 获取要渲染的模型队列
     * @return const std::queue<model_t>&   要渲染的模型队列
     */
    const std::queue<model_t>& get_visible_models(void) const;
};

#endif /* SIMPLERENDER_SCENE_H */
