
/**
 * @file device.h
 * @brief 设备抽象
 * @author Zone.N (Zone.Niuzh@hotmail.com)
 * @version 1.0
 * @date 2022-06-06
 * @copyright MIT LICENSE
 * https://github.com/Simple-XX/SimpleRenderer
 * @par change log:
 * <table>
 * <tr><th>Date<th>Author<th>Description
 * <tr><td>2022-06-06<td>Zone.N<td>迁移到 doxygen
 * </table>
 */

#ifndef _DEVICE_H_
#define _DEVICE_H_

#include "common.h"

class device_t {
public:
    // 坐标变换器
    // s_transform transform;
    // 窗口宽度
    int width;
    // 窗口高度
    int height;
    // 像素缓存
    int32_t *framebuffer;
    // 深度缓存：zbuffer[y] 为第 y 行指针
    float *zbuffer;
    // 纹理：同样是每行索引
    int32_t *texture;
    // 纹理宽度
    int tex_width;
    // 纹理高度
    int tex_height;
    // 纹理最大宽度：tex_width - 1
    float max_u;
    // 纹理最大高度：tex_height - 1
    float max_v;
    // 渲染状态
    int render_state;
    // 背景颜色
    int32_t background;
    // 线框颜色
    int32_t       foreground;
    point_light_t pointlight[20];
    s_material_t  material[100];
    v_material_t  vmaterial[100];
    int           v_m_num;
    s_camera_t    camera;
    int32_t      *texture_di;
    int           is_cull;
    s_PBR_t       PBR;
    // texture PBR
    t_PBR_t tPBR[30];
    int     now_state;
    float   m1;
    float   m2;
    // 设备初始化，fb为外部帧缓存，非 NULL 将引用外部帧缓存
    void init(int _width, int _height, void *_fb);
    // 删除设备
    void destory(void);
    // 设置当前纹理
    void set_texture(void);
    void set_texture_by_photo(char const *_path);
    void set_texture_by_diffuse(char const *_path, int _count);
    void set_texture_by_specular(char const *_path, int _count);
    void set_texture_by_normal(char const *_path, int _count);
    void set_texture_by_albedo(char const *_path, int _count);
    void set_texture_by_metallic(char const *_path, int _count);
    void set_texture_by_roughness(char const *_path, int _count);
    void set_texture_by_ao(char const *_path, int _count);
    void set_pointlight(s_vector_t &_pos, s_vector_t &_color, int _cnt);
    void set_pointlight(s_vector_t &_pos, s_vector_t &_color, s_vector_t &_am,
                        s_vector_t &_di, s_vector_t &_spe, int _cnt);

    // 清空 framebuffer 和 zbuffer
    void clear(int _mode);
    void pixel(int _x, int _y, int32_t _color);
    void draw_line(int _x1, int _y1, int _x2, int _y2, int32_t _c);
    // 根据坐标读取纹理
    int32_t texture_read(float _u, float _v) const;
    int32_t texture_read_from_material(const s_texture_t *_t_texture, float _u,
                                       float _v);
    // 绘制扫描线
    void draw_scanline(scanline_t *_scanline, s_vector_t &_point1,
                       s_vector_t &_point2, s_vector_t &_point3, for_fs_t *_ffs,
                       int _count);
    //主渲染函数
    void render_trap(trapezoid_t *_trap, s_vector_t &_point1,
                     s_vector_t &_point2, s_vector_t &_point3, for_fs_t *_ffs,
                     int _count);
    // 根据 render_state 绘制原始三角形
    void draw_primitive(vertex_t *_v1, vertex_t *_v2, vertex_t *_v3,
                        int _count);
    void camera_at_zero(s_vector_t _eye, s_vector_t _at, s_vector_t _up);
    void draw_plane(int _num, vertex_t *_mesh, int _count);

    void draw_plane(int _num, std::vector<vertex_t> &_mesh, int _count);
    void draw_plane_STRIP(std::vector<vertex_t> &_mesh,
                          std::vector<int> &_indices, int _count);
};

#endif /* _DEVICE_H_ */
