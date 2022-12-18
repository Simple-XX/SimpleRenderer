
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

#include "string"
#include "vector"

#include "tiny_obj_loader.h"

#include "color.h"
#include "matrix.hpp"
#include "vector.hpp"

/**
 * @brief 模型
 */
class model_t {
public:
    /// @brief 顶点坐标
    typedef vector4f_t coord_t;
    /// @brief 法向量
    typedef vector4f_t normal_t;
    /// @brief 贴图
    typedef vector4f_t texcoord_t;

    struct material_t {
        /// @brief 反光度
        float      shininess;
        /// @brief 环境光照
        vector4f_t ambient;
        /// @brief 漫反射光照
        vector4f_t diffuse;
        /// @brief 镜面光照
        vector4f_t specular;

        /**
         * @brief 构造函数
         */
        material_t(void);

        /**
         * @brief 构造函数
         * @param  _material        另一个 material_t
         */
        material_t(const material_t& _material);

        /**
         * @brief 析构函数
         */
        ~material_t(void);

        /**
         * @brief = 重载
         * @param  _material        另一个 material_t
         * @return material_t&      结果
         */
        material_t& operator=(const material_t& _material);
    };

    /**
     * @brief obj/mtl 文件的原始数据
     * @todo 直接保存太浪费内存了
     */
    struct vertex_t {
        /// @brief 坐标
        coord_t    coord;
        /// @brief 法线，顶点 v 的数量与 vn 的数量一样多
        normal_t   normal;
        /// @brief 贴图(纹理)，范围为 0~1，顶点 v 的个数不一定与纹理坐标 vt
        /// 的个数一样多， 因为有可能很多顶点公用一个纹理坐标的像素。
        texcoord_t texcoord;
        /// @brief
        /// 颜色，最终每个三角面的颜色，是由构成这个三角面的三个顶点进行插值计算
        /// 如果 obj 文件中没有指定则设为 1(白色)
        /// 范围 [0, 1]
        color_t    color;

        /**
         * @brief 构造函数
         */
        vertex_t(void);

        /**
         * @brief 构造函数
         * @param  _vertex          另一个 vertex_t
         */
        vertex_t(const vertex_t& _vertex);

        /**
         * @brief 构造函数
         * @param  _coord           坐标
         * @param  _normal          法向量
         * @param  _texcoord        贴图
         * @param  _color           颜色
         */
        vertex_t(const coord_t& _coord, const normal_t& _normal,
                 const texcoord_t& _texcoord, const color_t& _color);

        /**
         * @brief 析构函数
         */
        ~vertex_t(void);

        /**
         * @brief = 重载
         * @param  _vertex          另一个 vertex_t
         * @return vertex_t&        结果
         */
        vertex_t&       operator=(const vertex_t& _vertex);

        /**
         * @brief 顶点与矩阵进行运算，效果是对顶点进行变换
         * @param  _matrices        变换矩阵，第一个是坐标变换，第二个是法线变换
         * @param  vertex           要变换的 vertex_t
         * @return vertex_t         结果
         * @todo 确认这里的乘法顺序
         */
        friend vertex_t operator*(
          const std::pair<const matrix4f_t, const matrix4f_t>& _matrices,
          const vertex_t&                                      _vertex);
    };

    /// @todo 直接保存太浪费内存了
    struct face_t {
        vertex_t   v0;
        vertex_t   v1;
        vertex_t   v2;
        /// @brief 面的法向量为三个点的法向量矢量和
        normal_t   normal;
        // 面的颜色为三个点的颜色插值
        /// @brief 材质信息
        material_t material;

        /**
         * @brief 构造函数
         */
        face_t(void);

        /**
         * @brief 构造函数
         * @param  _face            另一个 face_t
         */
        face_t(const face_t& _face);

        /**
         * @brief 构造函数
         * @param  _v0              第一个顶点
         * @param  _v1              第二个顶点
         * @param  _v2              第三个顶点
         * @param  _material        材质
         */
        face_t(const vertex_t& _v0, const vertex_t& _v1, const vertex_t& _v2,
               const material_t& _material);

        /**
         * @brief 析构函数
         */
        ~face_t(void);

        /**
         * @brief = 重载
         * @param  _face            另一个 face_t
         * @return face_t&          结果
         */
        face_t&       operator=(const face_t& _face);

        /**
         * @brief 模型与矩阵进行运算，效果是对模型进行变换
         * @param  _matrices        变换矩阵，第一个是坐标变换，第二个是法线变换
         * @param  _face            要变换的 face_t
         * @return face_t           结果
         * @todo 确认这里的乘法顺序
         */
        friend face_t operator*(
          const std::pair<const matrix4f_t, const matrix4f_t>& _matrices,
          const face_t&                                        _face);
    };

    /**
     * @brief 碰撞盒
     */
    struct box_t {
        /// @brief 最小点
        vector4f_t min;
        /// @brief 最大点
        vector4f_t max;

        /**
         * @brief 构造函数
         */
        box_t(void);

        /**
         * @brief 构造函数
         * @param  _box             另一个 box
         */
        box_t(const box_t& _box);

        /**
         * @brief 析构函数
         */
        ~box_t(void);

        /**
         * @brief = 重载
         * @param  _box             另一个 box
         * @return box_t&           结果
         */
        box_t& operator=(const box_t& _box);
    };

private:
    std::vector<face_t> face;
    box_t               box;

public:
    /**
     * @brief 构造函数
     */
    model_t(void);

    /**
     * @brief 构造函数
     * @param  _model           另一个 model_t
     */
    model_t(const model_t& _model);

    /**
     * @brief 构造函数
     * @param  _obj_path        obj 文件路径
     * @param  _mtl_path        mtl 文件路径
     * @todo 顶点去重
     */
    explicit model_t(const std::string& _obj_path,
                     const std::string& _mtl_path = "");

    /**
     * @brief 析构函数
     */
    ~model_t(void);

    /**
     * @brief = 重载
     * @param  _model           另一个 model_t
     * @return model_t&         结果
     */
    model_t&                   operator=(const model_t& _model);

    /**
     * @brief * 重载，对模型应用变换矩阵
     * @param  _tran            另一个 要对模型进行的变换矩阵
     * @return model_t          结果
     */
    model_t                    operator*(const matrix4f_t& _tran) const;

    /**
     * @brief 获取面
     * @return const std::vector<face_t>&   所有面
     */
    const std::vector<face_t>& get_face(void) const;
};

// 模型变换
inline matrix4f_t
get_model_matrix(const vector4f_t& _scale, const vector4f_t& _rotate,
                 const float& _angle, const vector4f_t& _translate) {
    // 缩放
    auto scale    = matrix4f_t().scale(_scale.x, _scale.y, _scale.z);

    // 旋转
    auto rotation = matrix4f_t().rotate(_rotate, _angle);

    // 平移
    auto translate
      = matrix4f_t().translate(_translate.x, _translate.y, _translate.z);

    // 应用到向量上时先线性变换(缩放，旋转)再平移
    return translate * rotation * scale;
}

// 投影变换矩阵
inline matrix4f_t get_projection_matrix(float eye_fov, float aspect_ratio,
                                        float zNear, float zFar) {
    // 透视投影矩阵
    float proj_arr[4][4] = {
        {zNear,     0,            0,             0},
        {    0, zNear,            0,             0},
        {    0,     0, zNear + zFar, -zNear * zFar},
        {    0,     0,            1,             0}
    };
    auto proj = matrix4f_t(proj_arr);

    auto h
      = zNear * static_cast<float>(tan(static_cast<double>(eye_fov / 2))) * 2;
    auto  w               = h * aspect_ratio;
    auto  z               = zFar - zNear;
    // 正交投影矩阵，因为在观测投影时x0y平面视角默认是中心，所以这里的正交投影就不用平移x和y了
    float ortho_arr[4][4] = {
        {2 / w,     0,     0,                   0},
        {    0, 2 / h,     0,                   0},
        {    0,     0, 2 / z, -(zFar + zNear) / 2},
        {    0,     0,     0,                   1}
    };

    auto ortho = matrix4f_t(ortho_arr);

    return ortho * proj;
}

#endif /* SIMPLERENDER_MODEL_H */
