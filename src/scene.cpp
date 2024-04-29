
/**
 * @file scene.cpp
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
#include "log.h"
#include "matrix.hpp"

namespace SimpleRenderer {

scene_t::scene_t(const std::string &_name, uint64_t _x, uint64_t _y,
                 uint64_t _z)
    : name(_name), x(_x), y(_y), z(_z) {}

void scene_t::add_model(const model_t &_model, const vector3f_t _pos) {
  models.push_back(std::pair<model_t, vector3f_t>(_model, _pos));
  SPDLOG_LOGGER_INFO(SRLOG, "add_model: {} to: {}, total: {}", _model.obj_path,
                     name, models.size());
}

void scene_t::add_light(const light_t &_light) {
  lights.push_back(_light);
  SPDLOG_LOGGER_INFO(SRLOG, "add_light: {} to: {}", _light.name, name);
}

void scene_t::add_camera(const SimpleRenderer::camera_t &_camera) {
  camera = _camera;
  SPDLOG_LOGGER_INFO(SRLOG, "add_camera: {} to: {}", _camera.name, name);
}

} // namespace SimpleRenderer
