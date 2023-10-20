
/**
 * @file model.h
 * @brief 模型抽象
 * @author Zone.N (Zone.Niuzh@hotmail.com)
 * @version 1.0
 * @date 2022-06-06
 * @copyright MIT LICENSE
 * https://github.com/Simple-XX/SimpleRenderer
 * @par change log:
 * <table>
 * <tr><th>Date<th>Author<th>Description
 * <tr><td>2022-09-04<td>Zone.N<td>迁移到 doxygen
 * </table>
 */

#ifndef SIMPLERENDER_MODEL_H
#define SIMPLERENDER_MODEL_H

#include <string>
#include <vector>

#include <tiny_obj_loader.h>

#include "color.h"
#include "config.h"
#include "matrix.hpp"
#include "vector.h"

/**
 * 模型
 */
class model_t {
public:
  /// 顶点坐标
  using coord_t = vector3f_t;
  /// 法向量
  using normal_t = vector3f_t;
  /// 贴图
  using texture_coord_t = vector2f_t;

  struct material_t {
    /// 反光度
    float shininess = 0;
    /// 环境光照
    vector3f_t ambient;
    /// 漫反射光照
    vector3f_t diffuse;
    /// 镜面光照
    vector3f_t specular;

    /// @name 默认构造/析构函数
    /// @{
    material_t() = default;
    material_t(const material_t &_material) = default;
    material_t(material_t &&_material) = default;
    auto operator=(const material_t &_material) -> material_t & = default;
    auto operator=(material_t &&_material) -> material_t & = default;
    ~material_t() = default;
    /// @}
  };

  /**
   * obj/mtl 文件的原始数据
   * @todo 直接保存太浪费内存了
   */
  struct vertex_t {
    /// 坐标
    coord_t coord;
    /// 法线，顶点 v 的数量与 vn 的数量一样多
    normal_t normal;
    /// 贴图(纹理)，范围为 0~1，顶点 v 的个数不一定与纹理坐标 vt
    /// 的个数一样多， 因为有可能很多顶点公用一个纹理坐标的像素。
    texture_coord_t texture_coord;

    /// 颜色，最终每个三角面的颜色，是由构成这个三角面的三个顶点进行插值计算
    /// 如果 obj 文件中没有指定则设为 1(白色)
    /// 范围 [0, 1]
    color_t color;

    /**
     * 构造函数
     * @param _coord 坐标
     * @param _normal 法向量
     * @param _texture_coord 贴图
     * @param _color 颜色
     */
    explicit vertex_t(coord_t _coord, normal_t _normal,
                      texture_coord_t _texture_coord, const color_t &_color);

    /// @name 默认构造/析构函数
    /// @{
    vertex_t() = default;
    vertex_t(const vertex_t &_vertex) = default;
    vertex_t(vertex_t &&_vertex) = default;
    auto operator=(const vertex_t &_vertex) -> vertex_t & = default;
    auto operator=(vertex_t &&_vertex) -> vertex_t & = default;
    ~vertex_t() = default;
    /// @}

    /**
     * * 重载，对顶点应用变换矩阵
     * @param _tran 要对顶点进行的变换矩阵
     * @return 结果
     */
    [[nodiscard]] auto operator*(const matrix4f_t &_tran) const -> vertex_t;
  };

  /// @todo 直接保存太浪费内存了
  struct face_t {
    vertex_t v0;
    vertex_t v1;
    vertex_t v2;
    /// 面的法向量为三个点的法向量矢量和
    normal_t normal;
    // 面的颜色为三个点的颜色插值
    /// 材质信息
    material_t material;

    /**
     * 构造函数
     * @param _v0 第一个顶点
     * @param _v1 第二个顶点
     * @param _v2 第三个顶点
     * @param _material 材质
     */
    explicit face_t(const vertex_t &_v0, const vertex_t &_v1,
                    const vertex_t &_v2, material_t _material);

    /// @name 默认构造/析构函数
    /// @{
    face_t() = default;
    face_t(const face_t &_face) = default;
    face_t(face_t &&_face) = default;
    auto operator=(const face_t &_face) -> face_t & = default;
    auto operator=(face_t &&_face) -> face_t & = default;
    ~face_t() = default;
    /// @}

    /**
     * * 重载，对面应用变换矩阵
     * @param _tran 要对面进行的变换矩阵
     * @return 结果
     */
    [[nodiscard]] auto operator*(const matrix4f_t &_tran) const -> face_t;
  };

  /**
   * 碰撞盒
   */
  struct box_t {
    /// 最小点
    vector3f_t min;
    /// 最大点
    vector3f_t max;

    /// @name 默认构造/析构函数
    /// @{
    box_t() = default;
    box_t(const box_t &_box) = default;
    box_t(box_t &&_box) = default;
    auto operator=(const box_t &_box) -> box_t & = default;
    auto operator=(box_t &&_box) -> box_t & = default;
    ~box_t() = default;
    /// @}
  };

  /// obj 文件路径
  std::string obj_path = "";
  /// mtl 路径
  std::string mtl_path = "";

  /**
   * 构造函数
   * @param _obj_path obj 文件路径
   * @param _mtl_path mtl 文件路径
   * @todo 顶点去重
   */
  explicit model_t(const std::string &_obj_path,
                   const std::string &_mtl_path = "");

  /// @name 默认构造/析构函数
  /// @{
  model_t() = default;
  model_t(const model_t &_model) = default;
  model_t(model_t &&_model) = default;
  auto operator=(const model_t &_model) -> model_t & = default;
  auto operator=(model_t &&_model) -> model_t & = default;
  ~model_t() = default;
  /// @}

  /**
   * * 重载，对模型应用变换矩阵
   * @param _tran 要对模型进行的变换矩阵
   * @return 结果
   */
  [[nodiscard]] auto operator*(const matrix4f_t &_tran) const -> model_t;

  /**
   * 获取面
   * @return 所有面
   */
  [[nodiscard]] auto get_face() const -> const std::vector<face_t> &;

private:
  /// 三角形顶点数
  static constexpr const uint8_t TRIANGLE_FACE_VERTEX_COUNT = 3;

  std::vector<face_t> faces;

  /// 体积盒
  box_t box;

  /**
   * 计算 model 的体积盒
   */
  void set_box();

  /**
   * 缩放到显示区域较小的 1/2
   * @param _width 显示区域宽度
   * @param _height 显示区域高度
   */
  void scale_half(size_t _width = WIDTH, size_t _height = HEIGHT);
};

#endif /* SIMPLERENDER_MODEL_H */
