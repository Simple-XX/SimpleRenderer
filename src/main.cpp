
/**
 * @file main.cpp
 * @brief 入口
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

#include "thread"

#include "camera.h"
#include "color.h"
#include "config.h"
#include "default_shader.h"
#include "display.h"
#include "draw3d.h"
#include "event_callback.h"
#include "matrix.hpp"
#include "model.h"

/// @todo 参数传递，指针，引用等
/// @bug 在坐标系上可能有问题，设计的部分：法向量计算，光照方向，屏幕原点

static constexpr const uint32_t WIDTH  = 1920;
static constexpr const uint32_t HEIGHT = 1080;
config_t                        config;
auto             framebuffer = std::make_shared<framebuffer_t>(WIDTH, HEIGHT);
auto             camera      = std::make_shared<camera_t>();
auto             default_shader = std::make_shared<default_shader_t>();
event_callback_t event_callback(config, *camera.get());
std::vector<model_t> models;
display_t            display(framebuffer, camera, event_callback);

// 投影变换矩阵
matrix4f_t get_projection_matrix(float eye_fov, float aspect_ratio, float zNear,
                                 float zFar) {
    // 透视投影矩阵
    float proj_arr[4][4] = {
        {zNear,     0,            0,             0},
        {    0, zNear,            0,             0},
        {    0,     0, zNear + zFar, -zNear * zFar},
        {    0,     0,            1,             0}
    };
    auto  proj            = matrix4f_t(proj_arr);

    float h               = zNear * tan(eye_fov / 2) * 2;
    float w               = h * aspect_ratio;
    float z               = zFar - zNear;
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

void draw(std::shared_ptr<framebuffer_t> _framebuffer,
          std::shared_ptr<shader_base_t> _shader, config_t* _config) {
    draw3d_t draw3d(_framebuffer, _shader, *_config);

    auto     obj_path  = "../../obj/utah-teapot/utah-teapot.obj";
    auto     obj_path2 = "../../obj/cube3.obj";

    auto     model     = model_t(obj_path);
    auto     model2    = model_t(obj_path2);
    camera->pos        = vector4f_t(0, 0, 0);
    camera->target     = vector4f_t(0, 0, 0);
    camera->up         = vector4f_t(0, 1, 0);
    while (1) {
        draw3d.line(0, HEIGHT - 1, WIDTH - 1, 0, color_t::WHITE);
        draw3d.line(WIDTH - 1, HEIGHT - 1, 0, 0, color_t::WHITE);
        draw3d.line(WIDTH - 1, HEIGHT / 2, 0, HEIGHT / 2, color_t::WHITE);
        draw3d.line(WIDTH / 2, 0, WIDTH / 2, HEIGHT - 1, color_t::WHITE);

        _shader->shader_data.model_matrix
          = get_model_matrix(vector4f_t(10, 10, 10),
                             vector4f_t(0, 0, 1).normalize(), 180,
                             vector4f_t(WIDTH / 2, HEIGHT / 2, 0));
        // auto view_mat   = camera.look_at();
        _shader->shader_data.view_matrix    = matrix4f_t();
        _shader->shader_data.project_matrix = matrix4f_t();
        draw3d.model(model);

        _shader->shader_data.model_matrix
          = get_model_matrix(vector4f_t(1000, 1000, 1000),
                             vector4f_t(0, 1, 1).normalize(), 45,
                             vector4f_t(WIDTH / 2, HEIGHT / 2, 0));

        _shader->shader_data.view_matrix    = matrix4f_t();
        // auto view_mat2  = camera.look_at();
        _shader->shader_data.project_matrix = matrix4f_t();
        draw3d.model(model2);
    }
    return;
}

int main(int _argc, char** _argv) {
    // obj 路径
    std::vector<std::string> obj_path;
    // 如果没有指定那么使用默认值
    if (_argc == 1) {
        // obj_path = "../../obj/helmet.obj";
        //  obj_path = "../../obj/cube.obj";
        //  obj_path = "../../obj/cube2.obj";
        obj_path.push_back("../../obj/cube3.obj");
        // obj_path = "../../obj/cornell_box.obj";
        // obj_path.push_back("../../obj/helmet.obj");
        // obj_path.push_back("../../obj/african_head.obj");
        obj_path.push_back("../../obj/utah-teapot/utah-teapot.obj");
    }
    // 否则使用指定的
    else {
        for (auto i = 1; i < _argc; i++) {
            obj_path.push_back(_argv[i]);
        }
    }

    // 读取模型与材质
    for (auto& i : obj_path) {
        model_t model(i);
        models.push_back(model);
    }

    std::thread draw_thread
      = std::thread(draw, framebuffer, default_shader, &config);
    draw_thread.detach();

    display.loop();

    return 0;
}
