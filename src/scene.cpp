
/**
 * @file scend.cpp
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

#include "scene.h"

scene_t::scene_t(const std::shared_ptr<config_t>&      _config,
                 const std::shared_ptr<camera_base_t>& _camera,
                 const std::shared_ptr<input_t>&       _input)
    : config(_config), current_camera(_camera), input(_input) {
    cameras.push_back(current_camera);

    return;
}

scene_t::scene_t(const scene_t& _scene) {
    config         = _scene.config;
    cameras        = _scene.cameras;
    current_camera = _scene.current_camera;
    input          = _scene.input;
    models         = _scene.models;
    visible_models = _scene.visible_models;
    lights         = _scene.lights;

    return;
}

scene_t::~scene_t(void) {
    return;
}

scene_t& scene_t::operator=(const scene_t& _scene) {
    config         = _scene.config;
    cameras        = _scene.cameras;
    current_camera = _scene.current_camera;
    input          = _scene.input;
    models         = _scene.models;
    visible_models = _scene.visible_models;
    lights         = _scene.lights;

    return *this;
}

void scene_t::add_model(const model_t&    _model,
                        const matrix4f_t& _model_matrix) {
    models.push_back(_model * _model_matrix);

    return;
}

void scene_t::add_light(const light_t& _light) {
    lights.push_back(_light);

    return;
}

void scene_t::tick(const uint32_t _delta_time) {
    (void)_delta_time;
    /// @todo 更新可见模型
    /// @todo 计算光照
    /// @todo 移动摄像机

    return;
}
