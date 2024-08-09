
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
  using TextureCoord = glm::vec2;

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
    Material(const Material &material) = default;
    Material(Material &&material) = default;
    auto operator=(const Material &material) -> Material & = default;
    auto operator=(Material &&material) -> Material & = default;
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
     * @param coord 坐标
     * @param normal 法向量
     * @param texture_coord 贴图
     * @param color 颜色
     */
    explicit Vertex(Coord coord, Normal normal, TextureCoord texture_coord,
                    const Color &color);

    /// @name 默认构造/析构函数
    /// @{
    Vertex() = default;
    Vertex(const Vertex &vertex) = default;
    Vertex(Vertex &&vertex) = default;
    auto operator=(const Vertex &vertex) -> Vertex & = default;
    auto operator=(Vertex &&vertex) -> Vertex & = default;
    ~Vertex() = default;
    /// @}

    /**
     * * 重载，对顶点应用变换矩阵
     * @param tran 要对顶点进行的变换矩阵
     * @return 结果
     */
    [[nodiscard]] auto operator*(const Matrix4f &tran) const -> Vertex;
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
     * @param v0 第一个顶点
     * @param v1 第二个顶点
     * @param v2 第三个顶点
     * @param material 材质
     */
    explicit Face(const Vertex &v0, const Vertex &v1, const Vertex &v2,
                  Material material);

    /// @name 默认构造/析构函数
    /// @{
    Face() = default;
    Face(const Face &face) = default;
    Face(Face &&face) = default;
    auto operator=(const Face &face) -> Face & = default;
    auto operator=(Face &&face) -> Face & = default;
    ~Face() = default;
    /// @}

    /**
     * * 重载，对面应用变换矩阵
     * @param tran 要对面进行的变换矩阵
     * @return 结果
     */
    [[nodiscard]] auto operator*(const Matrix4f &tran) const -> Face;
  };

  /// obj 文件路径
  std::string obj_path_ = "";
  /// mtl 路径
  std::string mtl_path_ = "";

  /**
   * 构造函数
   * @param obj_path obj 文件路径
   * @param mtl_path mtl 文件路径
   * @todo 顶点去重
   */
  explicit Model(const std::string &obj_path, const std::string &mtl_path = "");

  /// @name 默认构造/析构函数
  /// @{
  Model() = default;
  Model(const Model &model) = default;
  Model(Model &&model) = default;
  auto operator=(const Model &model) -> Model & = default;
  auto operator=(Model &&model) -> Model & = default;
  ~Model() = default;
  /// @}

  /**
   * * 重载，对模型应用变换矩阵
   * @param tran 要对模型进行的变换矩阵
   * @return 结果
   */
  [[nodiscard]] auto operator*(const Matrix4f &tran) const -> Model;

  /**
   * 获取面
   * @return 所有面
   */
  [[nodiscard]] auto GetFace() const -> const std::vector<Face> &;

 private:
  /// 三角形顶点数
  static constexpr const uint8_t kTriangleFaceVertexCount = 3;

  /// 保存模型的所有面
  std::vector<Face> faces_;

  /**
   * 获取模型的 xyz 最大值/最小值
   */
  std::pair<Coord, Coord> GetMaxMinXYX();

  /**
   * 将模型归一化，所有坐标在 [-1, 1] 内
   */
  void Normalize();
};

}  // namespace simple_renderer

#endif /* SIMPLERENDER_SRC_INCLUDE_MODEL_HPP_ */
