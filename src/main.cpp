
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

#include "common.h"
#include "iostream"
#include "framebuffer.h"
#include "model.h"
#include "renderer.h"
#include "display.h"

using namespace std;

int main(int _argc, char **_argv) {
    // obj 路径
    string obj_path;
    // 材质路径
    string mtl_path;
    // 如果没有指定那么使用默认值
    if (_argc == 1) {
        obj_path = "../../src/obj/cube.obj";
        mtl_path = "../../src/obj/";
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
    model_t       model(obj_path, mtl_path);
    framebuffer_t framebuffer(WIDTH, HEIGHT);
    for (auto i = 0; i < 1920; i++) {
        for (auto j = 0; j < 1080; j++) {
            framebuffer.pixel(i, j, framebuffer_t::RGBA(255, 0, 0, 255));
        }
    }

    show_window(framebuffer);
    return 0;
}
