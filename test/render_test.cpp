
/**
 * @file draw3d_test.cpp
 * @brief draw3d_t 测试
 * @author Zone.N (Zone.Niuzh@hotmail.com)
 * @version 1.0
 * @date 2022-09-06
 * @copyright MIT LICENSE
 * https://github.com/Simple-XX/SimpleRenderer
 * @par change log:
 * <table>
 * <tr><th>Date<th>Author<th>Description
 * <tr><td>2022-09-06<td>Zone.N<td>创建文件
 * </table>
 */

#include "gtest/gtest.h"

#include "camera.h"
#include "default_shader.h"
#include "display.h"
#include "input.h"
#include "render.h"

TEST(render, line) {
    auto config  = std::make_shared<config_t>();
    auto display = std::make_shared<display_t>(config->WIDTH, config->HEIGHT);
    auto scene   = std::make_shared<scene_t>(config);
    auto input   = std::make_shared<input_t>();
    auto shader  = std::make_shared<default_shader_t>();
    auto render  = std::make_shared<render_t>(config, scene, display, input);

    scene->add_light(light_t());

    render->loop();

    return;
}

TEST(render, obj) {
    auto config  = std::make_shared<config_t>();
    auto display = std::make_shared<display_t>(config->WIDTH, config->HEIGHT);
    auto scene   = std::make_shared<scene_t>(config);
    auto input   = std::make_shared<input_t>();
    auto shader  = std::make_shared<default_shader_t>();
    auto render  = std::make_shared<render_t>(config, scene, display, input);

    // obj 路径
    std::vector<std::string> obj_path;

    obj_path.emplace_back("../../obj/cube3.obj");

    // 读取模型与材质
    for (auto& i : obj_path) {
        model_t model(i);

        scene->add_model(model);
    }
    scene->add_light(light_t());

    render->loop();

    return;
}
