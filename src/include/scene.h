
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

#ifndef _SCENE_H_
#define _SCENE_H_

#include "cstdint"
#include "vector"

#include "camera.h"
#include "light.h"
#include "matrix.hpp"
#include "model.h"
#include "vector.hpp"

/**
 * @brief 场景抽象
 */
class scene_t {
private:
    /// @brief 场景中的摄像机
    std::vector<camera_base_t> cameras;
    /// @brief 当前使用的摄像机
    camera_base_t              current_camera;

    /// @brief 场景中的所有模型
    std::vector<model_t>       models;
    /// @brief 场景中的可见模型
    std::vector<model_t>       visible_models;

    /// @brief 场景中的所有光照
    std::vector<light_t>       lights;

public:
    /**
     * @brief 空构造函数
     */
    scene_t(void);

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
    scene_t& operator=(const scene_t& _scene);

    /**
     * @brief 将模型添加到场景中
     * @param  _model           要添加的 model
     * @param  _model_matrix    模型变换矩阵变换
     */
    void     add_model(const model_t& _model, const matrix4f_t& _model_matrix);

    /**
     * @brief 将光照添加到场景中
     * @param  _light           要添加的 light
     */
    void     add_light(const light_t& _light);

    /**
     * @brief 更新场景，根据时间变化更新
     * @param  _delta_time      时间变化
     */
    void     tick(const uint32_t _delta_time);
};

#endif /* _SCENE_H_ */
