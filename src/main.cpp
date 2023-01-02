
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

#include "camera.h"
#include "config.h"
#include "default_shader.h"
#include "display.h"
#include "input.h"
#include "render.h"

/// @bug 在坐标系上可能有问题，设计的部分：法向量计算，光照方向，屏幕原点

auto config  = std::make_shared<config_t>();
auto display = std::make_shared<display_t>(config->WIDTH, config->HEIGHT);
auto scene   = std::make_shared<scene_t>(config);
auto input   = std::make_shared<input_t>();
auto shader  = std::make_shared<default_shader_t>();
auto render  = std::make_shared<render_t>(config, scene, display, input);

// void draw(const std::shared_ptr<framebuffer_t>& _framebuffer,
//           const std::shared_ptr<shader_base_t>& _shader,
//           const std::shared_ptr<config_t>&      _config) {
//     draw3d_t draw3d(_framebuffer, *_shader, _config);
//
//     auto     obj_path  = "../../obj/utah-teapot/utah-teapot.obj";
//     auto     obj_path2 = "../../obj/cube3.obj";
//
//     auto     model     = model_t(obj_path);
//     auto     model2    = model_t(obj_path2);
//     while (1) {
//         // 右对角线
//         draw3d.line(0, config_t::HEIGHT - 1, config_t::WIDTH - 1, 0,
//                     color_t::WHITE);
//         // 左对角线
//         draw3d.line(config_t::WIDTH - 1, config_t::HEIGHT - 1, 0, 0,
//                     color_t::WHITE);
//         // 水平平分线
//         draw3d.line(config_t::WIDTH - 1, config_t::HEIGHT / 2, 0,
//                     config_t::HEIGHT / 2, color_t::WHITE);
//         // 垂直平分线
//         draw3d.line(config_t::WIDTH / 2, 0, config_t::WIDTH / 2,
//                     config_t::HEIGHT - 1, color_t::WHITE);
//
//         _shader->shader_data.model_matrix
//           = get_model_matrix(vector4f_t(10, 10, 10),
//                              vector4f_t(0, 0, 1).normalize(), 180,
//                              vector4f_t(config_t::WIDTH / 2,
//                                         config_t::HEIGHT / 2, 0));
//         _shader->shader_data.view_matrix    = camera->look_at();
//         _shader->shader_data.project_matrix = matrix4f_t();
//         draw3d.model(model);
//
//         _shader->shader_data.model_matrix
//           = get_model_matrix(vector4f_t(1000, 1000, 1000),
//                              vector4f_t(0, 1, 1).normalize(), 45,
//                              vector4f_t(config_t::WIDTH / 2,
//                                         config_t::HEIGHT / 2, 0));
//         _shader->shader_data.view_matrix    = camera->look_at();
//         _shader->shader_data.project_matrix = matrix4f_t();
//         draw3d.model(model2);
//     }
//     return;
// }

int  main(int _argc, char** _argv) {
    // obj 路径
    std::vector<std::string> obj_path;
    // 如果没有指定那么使用默认值
    if (_argc == 1) {
        // obj_path = "../../obj/helmet.obj";
        //  obj_path = "../../obj/cube.obj";
        //  obj_path = "../../obj/cube2.obj";
        obj_path.emplace_back("../../obj/cube3.obj");
        // obj_path = "../../obj/cornell_box.obj";
        // obj_path.push_back("../../obj/helmet.obj");
        // obj_path.push_back("../../obj/african_head.obj");
        // obj_path.push_back("../../obj/utah-teapot/utah-teapot.obj");
    }
    // 否则使用指定的
    else {
        for (auto i = 1; i < _argc; i++) {
            obj_path.emplace_back(_argv[i]);
        }
    }

    // 读取模型与材质
    for (auto& i : obj_path) {
        model_t model(i);

        scene->add_model(model);
    }
    scene->add_light(light_t());

    render->loop();

    return 0;
}
