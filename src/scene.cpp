
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

scene_t::scene_t(const std::string &_name) : name(_name) {}

void scene_t::add_model(const model_t &_model) {
  models.push_back(_model);
  SPDLOG_LOGGER_INFO(SRLOG, "add_model: {} to: {}, total: {}", _model.obj_path,
                     name, models.size());
}

auto scene_t::get_models() const -> const std::vector<model_t> & {
  return models;
}

void scene_t::set_light(const light_t &_light) {
  light = _light;
  SPDLOG_LOGGER_INFO(SRLOG, "set_light: {} to: {}", light.name, name);
}

auto scene_t::tick(uint32_t _delta_time) -> bool {
  (void)_delta_time;
  return true;
}

auto scene_t::get_light() -> light_t & { return light; }

auto scene_t::get_light() const -> const light_t & { return light; }
