
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

scene_t::scene_t(const std::shared_ptr<config_t> &_config) : config(_config) {
  current_camera = surround_camera_t();
  cameras.push_back(current_camera);

  auto current_camera_shared = std::make_shared<camera_base_t>(current_camera);

  return;
}

scene_t::scene_t(const scene_t &_scene) {
  config = _scene.config;
  cameras = _scene.cameras;
  current_camera = _scene.current_camera;
  models = _scene.models;
  visible_models = _scene.visible_models;
  lights = _scene.lights;

  return;
}

scene_t::~scene_t(void) { return; }

scene_t &scene_t::operator=(const scene_t &_scene) {
  if (&_scene == this) {
    return *this;
  }

  config = _scene.config;
  cameras = _scene.cameras;
  current_camera = _scene.current_camera;
  models = _scene.models;
  visible_models = _scene.visible_models;
  lights = _scene.lights;

  return *this;
}

/**
 * @brief 计算从模型坐标(局部坐标)到世界坐标的变换矩阵，缩放 + 移动到 (0,
 * 0)，屏幕左上角
 * @param  _model           要被应用的模型
 * @bug 变换可能有问题
 * @param  _rotate          在默认变换的基础上进行变换的旋转矩阵，默认为 1
 * @param  _scale           在默认变换的基础上进行变换的缩放矩阵，默认为 1
 * @param  _translate       在默认变换的基础上进行变换的平移矩阵，默认为 1
 * @return std::pair<const matrix4f_t, const matrix4f_t>    变换矩阵
 */
inline std::pair<const matrix4f_t, const matrix4f_t>
model2world_tran(const model_t &_model, const matrix4f_t &_rotate,
                 const matrix4f_t &_scale, const matrix4f_t &_translate) {
  // 坐标变换矩阵
  matrix4f_t coord_mat;
  coord_mat = _rotate * coord_mat;
  // 法线变换矩阵
  matrix4f_t normal_mat;
  normal_mat = coord_mat.inverse().transpose();
  // 用旋转后的顶点计算极值
  auto tmp = _model.get_face()[0].v0.coord.x;
  auto x_max = std::numeric_limits<decltype(tmp)>::lowest();
  auto x_min = std::numeric_limits<decltype(tmp)>::max();
  auto y_max = x_max;
  auto y_min = x_min;
  auto z_max = x_max;
  auto z_min = x_min;
  for (auto &i : _model.get_face()) {
    auto v0 = i.v0.coord * coord_mat;
    auto v1 = i.v1.coord * coord_mat;
    auto v2 = i.v2.coord * coord_mat;
    x_max = std::max(x_max, v0.x);
    x_max = std::max(x_max, v1.x);
    x_max = std::max(x_max, v2.x);
    x_min = std::min(x_min, v0.x);
    x_min = std::min(x_min, v1.x);
    x_min = std::min(x_min, v2.x);

    y_max = std::max(y_max, v0.y);
    y_max = std::max(y_max, v1.y);
    y_max = std::max(y_max, v2.y);
    y_min = std::min(y_min, v0.y);
    y_min = std::min(y_min, v1.y);
    y_min = std::min(y_min, v2.y);

    z_max = std::max(z_max, v0.z);
    z_max = std::max(z_max, v1.z);
    z_max = std::max(z_max, v2.z);
    z_min = std::min(z_min, v0.z);
    z_min = std::min(z_min, v1.z);
    z_min = std::min(z_min, v2.z);
  }

  // 各分量的长度
  auto delta_x = (std::abs(x_max) + std::abs(x_min));
  auto delta_y = (std::abs(y_max) + std::abs(y_min));
  auto delta_z = (std::abs(z_max) + std::abs(z_min));
  auto delta_xy_max = std::max(delta_x, delta_y);
  auto delta_xyz_max = std::max(delta_xy_max, delta_z);

  // 缩放倍数
  auto multi = static_cast<float>((1080 + 1920) / 4.);
  // 归一化并乘倍数
  auto scale = multi / delta_xyz_max;
  // 缩放
  coord_mat = coord_mat.scale(scale);
  coord_mat = _scale * coord_mat;
  // 移动到左上角
  coord_mat =
      coord_mat.translate(std::abs(x_min) * scale, std::abs(y_min) * scale, 0);
  // 从左上角移动到指定位置
  coord_mat = _translate * coord_mat;
  return std::pair<const matrix4f_t, const matrix4f_t>{matrix4f_t(coord_mat),
                                                       matrix4f_t(normal_mat)};
}

void scene_t::add_model(const model_t &_model) {
  auto aaa = model2world_tran(_model, matrix4f_t(), matrix4f_t(), matrix4f_t());
  std::cout << aaa.first << std::endl;

  models.push_back(_model * aaa.first);

  return;
}

void scene_t::add_model(const model_t &_model,
                        const matrix4f_t &_model_matrix) {
  models.push_back(_model * _model_matrix);

  return;
}

void scene_t::add_light(const light_t &_light) {
  lights.push_back(_light);

  return;
}

bool scene_t::tick(uint32_t _delta_time) {
  (void)_delta_time;
  /// @todo 更新可见模型
  // 清空队列
  std::queue<model_t> empty;
  swap(visible_models, empty);
  for (const auto &i : models) {
    visible_models.push(i);
  }
  /// @todo 根据所有光照进行计算
  for (const auto &i : lights) {
    light = i;
  }
  /// @todo 移动摄像机

  return true;
}

config_t &scene_t::get_config(void) { return *config; }

const config_t &scene_t::get_config(void) const { return *config; }

camera_base_t &scene_t::get_current_camera(void) { return current_camera; }

const camera_base_t &scene_t::get_current_camera(void) const {
  return current_camera;
}

const std::queue<model_t> &scene_t::get_visible_models(void) const {
  return visible_models;
}

light_t &scene_t::get_light(void) { return light; }

const light_t &scene_t::get_light(void) const { return light; }
