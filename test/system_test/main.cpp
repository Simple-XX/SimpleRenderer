
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

#include "simple_renderer.h"

#include <cstdint>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "window.hpp"

static constexpr const int kWidth = 800;
static constexpr const int kHeight = 600;

static void pixel(int x, int y, uint32_t color, uint32_t *buffer) {
    buffer[x + y * kWidth] = color;
}

int main(int argc, char **argv) {
	// TODO: need to fix the way to use argc and argv
    (void) argc;
    (void) argv;
    // if (argc < 2) {
    //     std::cerr << "Usage: " << argv[0] << " <obj path>\n";
    // }
    // std::string obj_path = argv[1];
    std::string obj_path = "obj";

    auto framebuffer = std::make_shared<Framebuffer>(kWidth, kHeight);

    auto simple_renderer = simple_renderer::SimpleRenderer(framebuffer, pixel);

    std::vector<std::string> objs = {obj_path + "/utah-teapot/utah-teapot.obj"};

    auto matrix =
        simple_renderer::Matrix4f(simple_renderer::Matrix4f::Identity());
    matrix.diagonal() << 500, 500, 500, 1;
    matrix.col(matrix.cols() - 1) << kWidth / 2, kHeight / 2, 0, 1;

    for (auto &obj : objs) {
        auto model = simple_renderer::Model(obj);
        model = model * matrix;
        simple_renderer.render(model);
    }

    Window window(kWidth, kHeight);

    window.Display(framebuffer->data());

    return 0;
}
