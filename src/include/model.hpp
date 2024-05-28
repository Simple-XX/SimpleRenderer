
/**
 * @file model.hpp
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

#ifndef SIMPLERENDER_SRC_INCLUDE_MODEL_HPP_
#define SIMPLERENDER_SRC_INCLUDE_MODEL_HPP_

#include <tiny_obj_loader.h>

#include <string>
#include <vector>

#include "color.h"
#include "config.h"
#include "log_system.h"
#include "matrix.hpp"
#include "vector.hpp"

namespace simple_renderer {

/**
 * 模型
 */
class Model {
 public:
  /// 顶点坐标
  using Coord = Vector3f;
  /// 法向量
  using Normal = Vector3f;
  /// 贴图
  using TextureCoord = Vector2f;

  class Material {
   public:
    /// 反光度
    float shininess = 0;
    /// 环境光照
    Vector3f ambient;
    /// 漫反射光照
    Vector3f diffuse;
    /// 镜面光照
    Vector3f specular;

    /// @name 默认构造/析构函数
    /// @{
    Material() = default;
    Material(const Material &_material) = default;
    Material(Material &&_material) = default;
    auto operator=(const Material &_material) -> Material & = default;
    auto operator=(Material &&_material) -> Material & = default;
    ~Material() = default;
    /// @}
  };

  /**
   * obj/mtl 文件的原始数据
   * @todo 直接保存太浪费内存了
   */
  class Vertex {
   public:
    /// 坐标
    Coord coord_;
    /// 法线，顶点 v 的数量与 vn 的数量一样多
    Normal normal_;
    /// 贴图(纹理)，范围为 0~1，顶点 v 的个数不一定与纹理坐标 vt
    /// 的个数一样多， 因为有可能很多顶点公用一个纹理坐标的像素。
    TextureCoord texture_coord_;

    /// 颜色，最终每个三角面的颜色，是由构成这个三角面的三个顶点进行插值计算
    /// 如果 obj 文件中没有指定则设为 1(白色)
    /// 范围 [0, 1]
    Color color_;

    /**
     * 构造函数
     * @param _coord 坐标
     * @param _normal 法向量
     * @param _texture_coord 贴图
     * @param _color 颜色
     */
    explicit Vertex(Coord _coord, Normal _normal, TextureCoord _texture_coord,
                    const Color &_color);

    /// @name 默认构造/析构函数
    /// @{
    Vertex() = default;
    Vertex(const Vertex &_vertex) = default;
    Vertex(Vertex &&_vertex) = default;
    auto operator=(const Vertex &_vertex) -> Vertex & = default;
    auto operator=(Vertex &&_vertex) -> Vertex & = default;
    ~Vertex() = default;
    /// @}

    /**
     * * 重载，对顶点应用变换矩阵
     * @param _tran 要对顶点进行的变换矩阵
     * @return 结果
     */
    [[nodiscard]] auto operator*(const Matrix4f &_tran) const -> Vertex;
  };

  /// @todo 直接保存太浪费内存了
  class Face {
   public:
    Vertex v0_;
    Vertex v1_;
    Vertex v2_;
    /// 面的法向量为三个点的法向量矢量和
    Normal normal_;
    // 面的颜色为三个点的颜色插值
    /// 材质信息
    Material material_;

    /**
     * 构造函数
     * @param _v0 第一个顶点
     * @param _v1 第二个顶点
     * @param _v2 第三个顶点
     * @param _material 材质
     */
    explicit Face(const Vertex &_v0, const Vertex &_v1, const Vertex &_v2,
                  Material _material);

    /// @name 默认构造/析构函数
    /// @{
    Face() = default;
    Face(const Face &_face) = default;
    Face(Face &&_face) = default;
    auto operator=(const Face &_face) -> Face & = default;
    auto operator=(Face &&_face) -> Face & = default;
    ~Face() = default;
    /// @}

    /**
     * * 重载，对面应用变换矩阵
     * @param _tran 要对面进行的变换矩阵
     * @return 结果
     */
    [[nodiscard]] auto operator*(const Matrix4f &_tran) const -> Face;
  };

  /**
   * 碰撞盒
   */
  class Box {
   public:
    /// 最小点
    Vector3f min_;
    /// 最大点
    Vector3f max_;

    /// @name 默认构造/析构函数
    /// @{
    Box() = default;
    Box(const Box &_box) = default;
    Box(Box &&_box) = default;
    auto operator=(const Box &_box) -> Box & = default;
    auto operator=(Box &&_box) -> Box & = default;
    ~Box() = default;
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
  explicit Model(const std::string &_obj_path,
                 const std::string &_mtl_path = "");

  /// @name 默认构造/析构函数
  /// @{
  Model() = default;
  Model(const Model &_model) = default;
  Model(Model &&_model) = default;
  auto operator=(const Model &_model) -> Model & = default;
  auto operator=(Model &&_model) -> Model & = default;
  ~Model() = default;
  /// @}

  /**
   * * 重载，对模型应用变换矩阵
   * @param _tran 要对模型进行的变换矩阵
   * @return 结果
   */
  [[nodiscard]] auto operator*(const Matrix4f &_tran) const -> Model;

  /**
   * 获取面
   * @return 所有面
   */
  [[nodiscard]] auto get_face() const -> const std::vector<Face> &;

 private:
  /// 三角形顶点数
  static constexpr const uint8_t TRIANGLE_FACE_VERTEX_COUNT = 3;

  std::vector<Face> faces_;

  /// 体积盒
  Box box_;

  /// 模型的中心点
  Coord center_;

  /**
   * 计算 model 的体积盒
   */
  void set_box();

  /**
   * 将模型归一化
   */
  void normalize();
};

}  // namespace simple_renderer

/**
 * spdlog 输出 Box 实现
 */
template <>
struct fmt::formatter<simple_renderer::Model::Box>
    : fmt::formatter<std::string> {
  auto format(simple_renderer::Model::Box _box, format_context &_format_context)
      const -> decltype(_format_context.out()) {
    return fmt::format_to(_format_context.out(), "max: {},\nmin: {}", _box.max_,
                          _box.min_);
  }
};

#endif /* SIMPLERENDER_SRC_INCLUDE_MODEL_HPP_ */
