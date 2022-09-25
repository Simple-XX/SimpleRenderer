
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
#include "model.h"
#include "display.h"
#include "draw2d.h"
#include "draw3d.h"
#include "matrix.hpp"
#include "camera.h"

static constexpr const uint32_t                  WIDTH  = 1920;
static constexpr const uint32_t                  HEIGHT = 1080;
[[maybe_unused]] static constexpr const uint32_t RED    = 0xFFFF0000;
[[maybe_unused]] static constexpr const uint32_t GREEN  = 0xFF00FF00;
[[maybe_unused]] static constexpr const uint32_t BLUE   = 0xFF0000FF;
[[maybe_unused]] static constexpr const uint32_t WHITE  = 0xFFFFFFFF;
[[maybe_unused]] static constexpr const uint32_t BLACK  = 0xFF000000;

void draw(framebuffer_t *_framebuffer) {
    draw2d_t draw2d(*_framebuffer);
    draw2d.line(0, HEIGHT - 1, WIDTH - 1, 0, WHITE);
    draw2d.line(WIDTH - 1, HEIGHT - 1, 0, 0, WHITE);
    draw2d.line(WIDTH - 1, HEIGHT / 2, 0, HEIGHT / 2, WHITE);
    draw2d.line(WIDTH / 2, 0, WIDTH / 2, HEIGHT - 1, WHITE);

    vector2i_t v0(80, 80);
    vector2i_t v1(800, 800);
    vector2i_t v2(50, 900);
    draw2d.line(v2.x, v2.y, v0.x, v0.y, GREEN);
    draw2d.line(v0.x, v0.y, v2.x, v2.y, GREEN);

    draw2d.line(WIDTH / 2, HEIGHT / 2, WIDTH / 2 + 100, HEIGHT / 2 + 60, GREEN);
    draw2d.line(WIDTH / 2, HEIGHT / 2, WIDTH / 2 + 50, HEIGHT / 2 + 100, GREEN);

    draw2d.line(WIDTH / 2, HEIGHT / 2, WIDTH / 2 - 80, HEIGHT / 2 - 100, GREEN);
    draw2d.line(WIDTH / 2, HEIGHT / 2, WIDTH / 2 - 100, HEIGHT / 2 - 50, GREEN);

    draw2d.line(WIDTH / 2, HEIGHT / 2, WIDTH / 2 - 50, HEIGHT / 2 + 100, GREEN);
    draw2d.line(WIDTH / 2, HEIGHT / 2, WIDTH / 2 - 100, HEIGHT / 2 + 90, GREEN);

    draw2d.line(WIDTH / 2, HEIGHT / 2, WIDTH / 2 + 90, HEIGHT / 2 - 100, GREEN);
    draw2d.line(WIDTH / 2, HEIGHT / 2, WIDTH / 2 + 100, HEIGHT / 2 - 50, GREEN);

    draw2d.line(10, 20, 100, 200, RED);

    vector2i_t v3(830, 984);
    vector2i_t v4(400, 874);
    vector2i_t v5(505, 456);
    draw2d.triangle(v5, v3, v4, GREEN);
    draw2d.triangle(v0, v1, v2, RED);

    return;
}

int main(int _argc, char **_argv) {
    // obj 路径
    std::string obj_path;
    // 材质路径
    std::string mtl_path;
    // 如果没有指定那么使用默认值
    if (_argc == 1) {
        //        obj_path = "../../obj/helmet.obj";
        obj_path = "../../obj/cube.obj";
        //        obj_path = "../../obj/cornell_box.obj";
        //        mtl_path = "../../obj/cube.mtl";
        //        obj_path = "../../obj/catmark_torus_creases0.obj";
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
    model_t model(obj_path, mtl_path);

    framebuffer_t framebuffer(WIDTH, HEIGHT);

    draw2d_t draw2d(framebuffer);
    draw3d_t draw3d(framebuffer);

    auto m = matrix4f_t();
    m      = m.rotate(1, 1, 0, matrix4f_t::RAD(-120));
    m      = m.scale(WIDTH / 16., HEIGHT / 9., 1);
    m      = m.translate(960, 540, 0);

    for (size_t i = 0; i < model.get_vertex().size(); i += 3) {
        auto v0 = m * model.get_vertex()[i];
        auto v1 = m * model.get_vertex()[i + 1];
        auto v2 = m * model.get_vertex()[i + 2];
        //                draw2d.line(v0, v1, RED);
        //                draw2d.line(v1, v2, BLUE);
        //                draw2d.line(v2, v0, WHITE);
        //        draw2d.triangle(v0, v1, v2, WHITE);
        draw3d.triangle(v0, v1, v2, WHITE);
    }

    std::thread draw_thread = std::thread(draw, &framebuffer);
    draw_thread.join();

    display_t display(framebuffer);
    display.loop();

    return 0;
}
