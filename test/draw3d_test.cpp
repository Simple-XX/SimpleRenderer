
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
#include "draw3d.h"

static constexpr const uint32_t WIDTH  = 1920;
static constexpr const uint32_t HEIGHT = 1080;

void line(std::shared_ptr<framebuffer_t> _framebuffer,
          std::shared_ptr<shader_base_t> _shader,
          std::shared_ptr<config_t>      _config) {
    draw3d_t draw3d(*_framebuffer, *_shader, *_config);

    auto     obj_path  = "../../obj/utah-teapot/utah-teapot.obj";
    auto     obj_path2 = "../../obj/cube3.obj";

    while (1) {
        // 右对角线
        draw3d.line(0, HEIGHT - 1, WIDTH - 1, 0, color_t::WHITE);
        // 左对角线
        draw3d.line(WIDTH - 1, HEIGHT - 1, 0, 0, color_t::WHITE);
        // 水平平分线
        draw3d.line(WIDTH - 1, HEIGHT / 2, 0, HEIGHT / 2, color_t::WHITE);
        // 垂直平分线
        draw3d.line(WIDTH / 2, 0, WIDTH / 2, HEIGHT - 1, color_t::WHITE);

        // 红色三角形
        vector4f_t v0(80, 80);
        vector4f_t v1(100, 100);
        vector4f_t v2(50, 110);
        draw3d.triangle(v0, v1, v2, color_t::RED);

        // 绿色三角形
        vector4f_t v3(80, 200);
        vector4f_t v4(100, 300);
        vector4f_t v5(50, 400);
        draw3d.triangle(v3, v4, v5, color_t::GREEN);

        // 红白绿线
        vector4f_t line1(100, 200, 300);
        auto       mmm = matrix4f_t().translate(1000, 500, 0);
        line1          = mmm * line1;
        draw3d.line(0, 0, line1.x, line1.y, color_t::RED);
        mmm = matrix4f_t()
                .translate(1000, 500, 0)
                .rotate(vector4f_t(0, 0, 1), -15);
        line1 = vector4f_t(100, 200, 300);
        line1 = mmm * line1;
        draw3d.line(0, 0, line1.x, line1.y, color_t::WHITE);
        mmm = matrix4f_t()
                .translate(1000, 500, 0)
                .rotate(vector4f_t(0, 0, 1), 15);
        line1 = vector4f_t(100, 200, 300);
        line1 = mmm * line1;
        draw3d.line(0, 0, line1.x, line1.y, color_t::GREEN);
    }
    return;
}

TEST(draw3d_t, line) {
    auto framebuffer    = std::make_shared<framebuffer_t>(WIDTH, HEIGHT);
    auto config         = std::make_shared<config_t>();
    auto shader         = std::make_shared<default_shader_t>();
    auto camera         = std::make_shared<camera_t>();
    auto event_callback = std::make_shared<event_callback_t>(*config, *camera);
    auto display
      = std::make_shared<display_t>(*framebuffer, *camera, *event_callback);

    std::thread draw_thread = std::thread(line, framebuffer, shader, config);
    draw_thread.detach();

    display->loop();

    return;
}

void obj(std::shared_ptr<framebuffer_t> _framebuffer,
         std::shared_ptr<shader_base_t> _shader,
         std::shared_ptr<config_t>      _config) {
    draw3d_t draw3d(*_framebuffer, *_shader, *_config);

    auto     obj_path  = "../../obj/utah-teapot/utah-teapot.obj";
    auto     obj_path2 = "../../obj/cube3.obj";

    auto     model     = model_t(obj_path);
    auto     model2    = model_t(obj_path2);
    while (1) {
        // 右对角线
        draw3d.line(0, HEIGHT - 1, WIDTH - 1, 0, color_t::WHITE);
        // 左对角线
        draw3d.line(WIDTH - 1, HEIGHT - 1, 0, 0, color_t::WHITE);
        // 水平平分线
        draw3d.line(WIDTH - 1, HEIGHT / 2, 0, HEIGHT / 2, color_t::WHITE);
        // 垂直平分线
        draw3d.line(WIDTH / 2, 0, WIDTH / 2, HEIGHT - 1, color_t::WHITE);

        _shader->shader_data.model_matrix
          = get_model_matrix(vector4f_t(10, 10, 10),
                             vector4f_t(0, 0, 1).normalize(), 180,
                             vector4f_t(WIDTH / 2, HEIGHT / 2, 0));
        _shader->shader_data.view_matrix    = matrix4f_t();
        _shader->shader_data.project_matrix = matrix4f_t();
        draw3d.model(model);

        _shader->shader_data.model_matrix
          = get_model_matrix(vector4f_t(1000, 1000, 1000),
                             vector4f_t(0, 1, 1).normalize(), 45,
                             vector4f_t(WIDTH / 2, HEIGHT / 2, 0));

        _shader->shader_data.view_matrix    = matrix4f_t();
        _shader->shader_data.project_matrix = matrix4f_t();
        draw3d.model(model2);
    }
    return;
}

TEST(draw3d_t, obj) {
    auto framebuffer    = std::make_shared<framebuffer_t>(WIDTH, HEIGHT);
    auto config         = std::make_shared<config_t>();
    auto shader         = std::make_shared<default_shader_t>();
    auto camera         = std::make_shared<camera_t>();
    auto event_callback = std::make_shared<event_callback_t>(*config, *camera);
    auto display
      = std::make_shared<display_t>(*framebuffer, *camera, *event_callback);

    std::thread draw_thread = std::thread(obj, framebuffer, shader, config);
    draw_thread.detach();

    display->loop();

    return;
}
