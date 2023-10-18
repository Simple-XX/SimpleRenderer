
/**
 * @file scene.h
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

#include <queue>
#include <string>
#include <vector>

#include "camera.h"
#include "config.h"
#include "light.h"
#include "matrix.h"
#include "model.h"
#include "vector.h"

/**
 * 场景抽象
 */
class scene_t {
public:
  /// 场景名称
  std::string name = "default scene name";

  /**
   * 构造函数
   * @param _name 场景名称
   */
  explicit scene_t(const std::string &_name);

  /// @name 默认构造/析构函数
  /// @{
  scene_t() = default;
  scene_t(const scene_t &_scene) = default;
  scene_t(scene_t &&_scene) = default;
  auto operator=(const scene_t &_scene) -> scene_t & = default;
  auto operator=(scene_t &&_scene) -> scene_t & = default;
  virtual ~scene_t() = default;
  /// @}

  /**
   * 将模型添加到场景中，缩放到合适大小，移到屏幕中央
   * @param _model 要添加的 model
   */
  void add_model(const model_t &_model);

  /**
   * 将模型添加到场景中
   * @param _model 要添加的 model
   * @param _model_matrix 模型变换矩阵变换
   */
  void add_model(const model_t &_model, const matrix4f_t &_model_matrix);

  /**
   * 将光照添加到场景中
   * @param _light 要添加的 light
   */
  void add_light(const light_t &_light);

  /**
   * 更新场景，根据时间变化更新，返回是否继续运行
   * @param _delta_time 时间变化
   * @return 返回 false 时结束运行
   */
  auto tick(uint32_t _delta_time) -> bool;

  /**
   * 获取当前相机
   * @return 要使用相机
   */
  auto get_current_camera() -> camera_base_t &;

  /**
   * 获取当前相机
   * @return 当前相机
   */
  [[nodiscard]] auto get_current_camera() const -> const camera_base_t &;

  /**
   * 获取要渲染的模型队列
   * @return 要渲染的模型队列
   */
  [[nodiscard]] auto get_visible_models() const -> const std::queue<model_t> &;

  /**
   * 获取场景的光照
   * @return 场景的光照信息
   */
  auto get_light() -> light_t &;

  /**
   * 获取场景的光照
   * @return 场景的配置信息
   */
  [[nodiscard]] auto get_light() const -> const light_t &;

private:
  /// 场景中的摄像机
  std::vector<camera_base_t> cameras;
  /// 当前使用的摄像机
  camera_base_t camera;

  /// 场景中的所有模型
  std::vector<model_t> models;
  /// 场景中的可见模型，即要渲染的模型队列
  std::queue<model_t> visible_models;

  /// 场景中的所有光照
  std::vector<light_t> lights;
  /// 场景中的光照合并值
  light_t light;
};

#endif /* SIMPLERENDER_SCENE_H */
