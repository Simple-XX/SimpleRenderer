
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
#include "display.h"
#include "draw3d.h"
#include "matrix.hpp"
#include "model.h"

/// @bug 在坐标系上可能有问题，设计的部分：法向量计算，光照方向，屏幕原点

static constexpr const uint32_t                  WIDTH  = 1920;
static constexpr const uint32_t                  HEIGHT = 1080;
[[maybe_unused]] static constexpr const uint32_t RED    = 0xFFFF0000;
[[maybe_unused]] static constexpr const uint32_t GREEN  = 0xFF00FF00;
[[maybe_unused]] static constexpr const uint32_t BLUE   = 0xFF0000FF;
[[maybe_unused]] static constexpr const uint32_t WHITE  = 0xFFFFFFFF;
[[maybe_unused]] static constexpr const uint32_t BLACK  = 0xFF000000;
std::vector<model_t>                             models;

void draw(std::shared_ptr<framebuffer_t> _framebuffer) {
    draw3d_t draw3d(_framebuffer);

    auto     obj_path = "../../obj/cube3.obj";
    auto     model    = model_t(obj_path);
    camera.pos        = vector4f_t(500, 500, 0);
    uint64_t frames   = 0;
    uint64_t sec      = 0;
    while (1) {
        auto start = us();
        draw3d.line(0, HEIGHT - 1, WIDTH - 1, 0, WHITE);
        draw3d.line(WIDTH - 1, HEIGHT - 1, 0, 0, WHITE);
        draw3d.line(WIDTH - 1, HEIGHT / 2, 0, HEIGHT / 2, WHITE);
        draw3d.line(WIDTH / 2, 0, WIDTH / 2, HEIGHT - 1, WHITE);

        // auto x_offset = 0;
        // auto y_offset = 0;
        // for (auto& i : models) {
        //     draw3d.model(i,
        //                  matrix4f_t()
        //                    .rotate(vector4f_t(0, 0, 1).normalize(),
        //                            matrix4f_t::RAD(90))
        //                    .translate(1000, 500, 0),
        //                  matrix4f_t(), matrix4f_t());
        //     x_offset += WIDTH / 2;
        // }

        auto model_mat = get_model_matrix(vector4f_t(1000, 1000, 1),
                                          vector4f_t(1, 1, 1).normalize(), 190,
                                          vector4f_t(1000, 500, 0));
        camera.pos     = vector4f_t(0, 0, 0);
        camera.up      = vector4f_t(0, 0, -1);
        camera.target  = vector4f_t(1, 1, 0);
        // auto view_mat  = get_view_matrix(camera);
        auto view_mat  = matrix4f_t();
        draw3d.model(model, model_mat, view_mat, matrix4f_t());

        frames++;
        auto end = us();
        sec      += end - start;
        if (sec >= 1000000) {
            // std::cout << "fps_draw: " << frames << std::endl;
            frames = 0;
            sec    = 0;
        }
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

    auto        framebuffer = std::make_shared<framebuffer_t>(WIDTH, HEIGHT);

    std::thread draw_thread = std::thread(draw, framebuffer);
    draw_thread.detach();

    display_t display(framebuffer);
    display.loop();

    return 0;
}
