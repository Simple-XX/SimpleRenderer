
/**
 * @file camera.h
 * @brief 相机抽象
 * @author Zone.N (Zone.Niuzh@hotmail.com)
 * @version 1.0
 * @date 2022-09-19
 * @copyright MIT LICENSE
 * https://github.com/Simple-XX/SimpleRenderer
 * @par change log:
 * <table>
 * <tr><th>Date<th>Author<th>Description
 * <tr><td>2022-09-19<td>Zone.N<td>创建文件
 * </table>
 */

#ifndef SIMPLERENDER_CAMERA_H
#define SIMPLERENDER_CAMERA_H

#include "log.h"
#include "matrix.h"
#include "vector.h"

/**
 * 相机基类
 */
class camera_base_t {
public:
  /**
   * 用于标识相机移动的方向
   */
  enum move_to_t : uint8_t {
    RIGHT,
    LEFT,
    UP,
    DOWN,
    FORWARD,
    BACKWARD,
  };

  /**
   * 用于标识相机旋转的方向
   * @todo
   */
  enum course_to_t : uint8_t {
    PITCH_UP [[maybe_unused]],
    PITCH_DOWN [[maybe_unused]],
    YAW_LEFT [[maybe_unused]],
    YAW_RIGHT [[maybe_unused]],
    ROLL_1 [[maybe_unused]],
    ROLL_2 [[maybe_unused]],
  };

  /**
   * 构造函数
   * @param  _pos             位置
   * @param  _target          方向
   * @param  _aspect          比例
   */
  explicit camera_base_t(const vector3f_t &_pos, const vector3f_t &_target,
                         float _aspect);

  /// @name 默认构造/析构函数
  /// @{
  camera_base_t() = default;
  camera_base_t(const camera_base_t &_camera_base) = default;
  camera_base_t(camera_base_t &&_camera_base) = default;
  auto operator=(const camera_base_t &_camera_base)
      -> camera_base_t & = default;
  auto operator=(camera_base_t &&_camera_base) -> camera_base_t & = default;
  virtual ~camera_base_t() = default;
  /// @}

  /**
   * 相机复位
   */
  virtual void set_default();

  /**
   * 更新相机位置
   * @param  _to              移动的方向
   * @param  _delta_time      时间变化
   */
  virtual void move(const move_to_t &_to, uint32_t _delta_time);

  /**
   * 更新相机目标
   * @param  _to              移动的方向
   * @param  _delta_time      时间变化
   */
  virtual void update_target(const move_to_t &_to, uint32_t _delta_time);

  /**
   * 更新相机上方向
   * @param  _to              移动的方向
   * @param  _delta_time      时间变化
   */
  virtual void update_up(const move_to_t &_to, uint32_t _delta_time);

  /**
   * 获取视图变换矩阵, 将相机移动到原点，方向指向 -z，即屏幕里，up 为
   * -y，即屏幕向上
   * @return matrix4f_t       视图矩阵
   */
  [[nodiscard]] virtual auto look_at() const -> matrix4f_t;

protected:
  /// 默认在位置，在原点
  static const vector3f_t DEFAULT_POS;
  /// 默认看向屏幕内
  static const vector3f_t DEFAULT_TARGET;
  /// 默认上方向为屏幕向上
  static const vector3f_t DEFAULT_UP;
  /// 默认指向屏幕内为正方向
  static const vector3f_t DEFAULT_FRONT;
  /// 右方向通过计算得出，默认为 -1
  /// @todo 这里的右方向需要确认正确性
  static const vector3f_t DEFAULT_RIGHT;

  /// 屏幕宽高比
  static constexpr const float DEFAULT_ASPECT = 1;
  /// 相机移动速度
  static constexpr const float DEFAULT_SPEED = 1;

  /// 位置
  vector3f_t pos = DEFAULT_POS;
  /// 方向
  vector3f_t target = DEFAULT_TARGET;

  /// 上方向，单位向量
  vector3f_t up = DEFAULT_UP;
  /// 前方向，单位向量
  vector3f_t front = DEFAULT_FRONT;
  /// 右方向，单位向量，通过计算得出
  vector3f_t right = DEFAULT_RIGHT;

  /// 比例
  float aspect = DEFAULT_ASPECT;
  /// 相机速度
  float speed = DEFAULT_SPEED;
};

/**
 * 环绕相机
 */
class surround_camera_t : public camera_base_t {
public:
  /// @name 默认构造/析构函数
  /// @{
  surround_camera_t() = default;
  surround_camera_t(const surround_camera_t &_camera_base) = default;
  surround_camera_t(surround_camera_t &&_camera_base) = default;
  auto operator=(const surround_camera_t &_camera_base)
      -> surround_camera_t & = default;
  auto operator=(surround_camera_t &&_camera_base)
      -> surround_camera_t & = default;
  ~surround_camera_t() override = default;
  /// @}
};

#endif /* SIMPLERENDER_CAMERA_H */
