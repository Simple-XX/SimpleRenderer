
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

/**
 * @brief 模型
 */
class model_t {
public:
    /// 顶点坐标
    typedef vector4f_t coord_t;
    /// 法向量
    typedef vector4f_t normal_t;
    /// 颜色
    typedef vector4f_t color_t;
    /// 贴图
    typedef vector4f_t texcoord_t;

    /**
     * @brief obj/mtl 文件的原始数据
     */
    struct vertex_t {
        /// 坐标
        coord_t coord;
        /// 法线
        normal_t normal;
        /// 贴图(纹理)，范围为 0~1
        texcoord_t texcoord;
        /// 颜色
        color_t color;
        /// 材质
        tinyobj::material_t material;

        vertex_t(void) = default;

        vertex_t(const coord_t &_coord, const normal_t &_normal,
                 const texcoord_t &_texcoord, const color_t &_color,
                 const tinyobj::material_t &_material);
    };

    struct face_t {
        vertex_t            v0;
        vertex_t            v1;
        vertex_t            v2;
        tinyobj::material_t material;
        // 面的法向量为三个点的法向量矢量和
        // 面的颜色为三个点的颜色插值
        face_t(const vertex_t &_v0, const vertex_t &_v1, const vertex_t &_v2,
               const tinyobj::material_t &_material);
    };

private:
    std::vector<face_t> face;

public:
    /**
     * @brief 构造函数
     * @param  _obj_path        obj 文件路径
     * @param  _mtl_path        mtl 文件路径
     * @todo 顶点去重
     */
    model_t(const std::string &_obj_path, const std::string &_mtl_path = "");

    /**
     * @brief 析构函数
     */
    ~model_t(void);

    /**
     * @brief 获取面
     * @return const std::vector<face_t>&   所有面
     */
    const std::vector<face_t> &get_face(void) const;
};

#endif /* _MODEL_H_ */
