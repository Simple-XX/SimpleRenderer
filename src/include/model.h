
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

#ifndef _MODEL_H_
#define _MODEL_H_

#include "vector"
#include "string"
#include "tiny_obj_loader.h"
#include "vector.hpp"

/// 顶点
typedef vector3f_t vertex_t;
/// 法向量
typedef vector3f_t normal_t;
/// 颜色
typedef vector3f_t color_t;
/// 贴图
typedef vector2f_t texcoord_t;

struct vvv_t {
    /// 坐标
    /// 法线
    /// 贴图
    /// 颜色
    /// 材质
};

/**
 * @brief obj/mtl 文件的原始数据
 */
struct mesh_t {
    /// 顶点
    std::vector<vertex_t> vertices;
    /// 法线
    std::vector<normal_t> normals;
    /// 贴图
    std::vector<texcoord_t> texcoords;
    /// 颜色
    std::vector<color_t> colors;
    /// 材质
    std::vector<tinyobj::material_t> materials;
};

/**
 * @brief 模型
 */
class model_t {
private:
    /// 原始数据
    mesh_t mesh;

public:
    /**
     * @brief 构造函数
     * @param  _obj_path        obj 文件路径
     * @param  _mtl_path        mtl 文件路径
     */
    model_t(const std::string &_obj_path, const std::string &_mtl_path = "");

    /**
     * @brief 析构函数
     */
    ~model_t(void);

    /**
     * @brief 获取所有顶点
     * @return const std::vector<vertex_t>& 所有顶点
     */
    const std::vector<vertex_t> &get_vertex(void) const;

    /**
     * @brief 获取所有法线
     * @return const std::vector<normal_t>& 所有法线
     */
    const std::vector<normal_t> &get_normal(void) const;

    /**
     * @brief 获取所有贴图
     * @return const std::vector<texcoord_t>&   所有贴图
     */
    const std::vector<texcoord_t> &get_texcoord(void) const;

    /**
     * @brief 获取所有材质
     * @return const std::vector<tinyobj::material_t>&  所有材质
     */
    const std::vector<tinyobj::material_t> &get_material(void) const;
};

#endif /* _MODEL_H_ */
