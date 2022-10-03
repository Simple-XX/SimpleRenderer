
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
#include "display.h"
#include "draw3d.h"
#include "matrix.hpp"
#include "model.h"
#include "thread"

static constexpr const uint32_t                  WIDTH  = 1920;
static constexpr const uint32_t                  HEIGHT = 1080;
[[maybe_unused]] static constexpr const uint32_t RED    = 0xFFFF0000;
[[maybe_unused]] static constexpr const uint32_t GREEN  = 0xFF00FF00;
[[maybe_unused]] static constexpr const uint32_t BLUE   = 0xFF0000FF;
[[maybe_unused]] static constexpr const uint32_t WHITE  = 0xFFFFFFFF;
[[maybe_unused]] static constexpr const uint32_t BLACK  = 0xFF000000;

void draw(std::shared_ptr<framebuffer_t> _framebuffer) {
    draw3d_t draw3d(_framebuffer);
    draw3d.line(0, HEIGHT - 1, WIDTH - 1, 0, WHITE);
    draw3d.line(WIDTH - 1, HEIGHT - 1, 0, 0, WHITE);
    draw3d.line(WIDTH - 1, HEIGHT / 2, 0, HEIGHT / 2, WHITE);
    draw3d.line(WIDTH / 2, 0, WIDTH / 2, HEIGHT - 1, WHITE);

    vector4f_t v0(80, 80);
    vector4f_t v1(800, 800);
    vector4f_t v2(50, 900);
    vector4f_t v3(830, 984);
    vector4f_t v4(400, 874);
    vector4f_t v5(505, 456);
    // draw3d.triangle2d(v5, v3, v4, GREEN);
    // draw3d.triangle2d(v0, v1, v2, RED);

    return;
}

int main(int _argc, char** _argv) {
    auto        start = us();
    // obj 路径
    std::string obj_path;
    // 材质路径
    std::string mtl_path;
    // 如果没有指定那么使用默认值
    if (_argc == 1) {
        // obj_path = "../../obj/helmet.obj";
        //  obj_path = "../../obj/cube.obj";
        //  obj_path = "../../obj/cube2.obj";
        // obj_path = "../../obj/cube3.obj";
        //  obj_path = "../../obj/cornell_box.obj";
        //  mtl_path = "../../obj/cube.mtl";
        // obj_path = "../../obj/african_head.obj";
        obj_path = "../../obj/utah-teapot/utah-teapot.obj";
    }
    // 否则使用指定的
    else {
        if (_argc == 2) {
            obj_path = _argv[1];
        }
        if (_argc == 3) {
            mtl_path = _argv[2];
        }
    }

    // 读取模型与材质
    model_t  model(obj_path, mtl_path);

    auto     framebuffer = std::make_shared<framebuffer_t>(WIDTH, HEIGHT);

    draw3d_t draw3d(framebuffer);

    draw3d.model(model);

    auto m = matrix4f_t();
    for (size_t i = 0; i < model.get_face().size(); i++) {
        auto v0 = model.get_face()[i].v0.coord * m;
        auto v1 = model.get_face()[i].v1.coord * m;
        auto v2 = model.get_face()[i].v2.coord * m;
        // draw3d.line(v0, v1, RED);
        // draw3d.line(v1, v2, BLUE);
        // draw3d.line(v2, v0, WHITE);
        draw3d.triangle2d(v0, v1, v2, RED);
    }

    std::thread draw_thread = std::thread(draw, framebuffer);
    draw_thread.join();

    display_t display(framebuffer);
    display.loop();

    auto end = us();
    std::cout << "time: " << end - start << std::endl;

    return 0;
}
