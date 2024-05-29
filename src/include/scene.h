
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

#ifndef SIMPLERENDER_SRC_INCLUDE_SCENE_H_
#define SIMPLERENDER_SRC_INCLUDE_SCENE_H_

#include <queue>
#include <string>
#include <vector>

#include "camera.h"
#include "config.h"
#include "light.h"
#include "matrix.hpp"
#include "model.hpp"
#include "vector.hpp"

namespace simple_renderer {

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
   * @param _x 场景长
   * @param _y 场景宽
   * @param _z 场景高
   */
  explicit scene_t(const std::string &_name, uint64_t _x, uint64_t _y,
                   uint64_t _z);

  /// @name 默认构造/析构函数
  /// @{
  scene_t() = delete;
  scene_t(const scene_t &_scene) = default;
  scene_t(scene_t &&_scene) = default;
  auto operator=(const scene_t &_scene) -> scene_t & = default;
  auto operator=(scene_t &&_scene) -> scene_t & = default;
  virtual ~scene_t() = default;
  /// @}

  /**
   * 将模型添加到场景中，缩放到合适大小
   * @param _model 要添加的 model
   * @param _pos 模型在场景中的位置
   */
  void add_model(const model_t &_model, vector3f_t _pos);

  /**
   * 添加光照
   * @param _light 要设置的 light
   */
  void add_light(const light_t &_light);

  /**
   * 添加 camera
   * @param _camera 要添加的 camera
   */
  void add_camera(const camera_t &_camera);

private:
  /// 场景大小
  uint64_t x;
  uint64_t y;
  uint64_t z;
  /// 光照信息
  std::vector<light_t> lights;
  /// 模型信息
  std::vector<std::pair<model_t, vector3f_t>> models;
  /// 摄像机信息
  camera_t camera;
};

} // namespace SimpleRenderer

#endif /* SIMPLERENDER_SRC_INCLUDE_SCENE_H_ */
