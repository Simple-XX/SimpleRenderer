
/**
 * @file render.cpp
 * @brief 渲染抽象
 * @author Zone.N (Zone.Niuzh@hotmail.com)
 * @version 1.0
 * @date 2022-12-15
 * @copyright MIT LICENSE
 * https://github.com/Simple-XX/SimpleRenderer
 * @par change log:
 * <table>
 * <tr><th>Date<th>Author<th>Description
 * <tr><td>2022-12-15<td>Zone.N<td>创建文件
 * </table>
 */

#include "render.h"

render_t::render_t(const std::shared_ptr<config_t>&      _config,
                   const std::shared_ptr<scene_t>&       _scene,
                   const std::shared_ptr<display_t>&     _display,
                   const std::shared_ptr<framebuffer_t>& _framebuffer,
                   const std::shared_ptr<input_t>&       _input)
    : config(_config),
      scene(_scene),
      display(_display),
      framebuffer(_framebuffer),
      input(_input) {
    return;
}

render_t::render_t(const render_t& _render) : scene(_render.scene) {
    return;
}

render_t::~render_t(void) {
    return;
}

render_t& render_t::operator=(const render_t& _render) {
    if (&_render == this) {
        return *this;
    }

    scene = _render.scene;
    return *this;
}

void render_t::loop(void) {
    uint64_t sec        = 0;
    uint32_t frames     = 0;
    uint32_t fps        = 0;
    auto     start      = us();
    auto     end        = us();
    auto     is_running = true;
    // 主循环
    while (is_running == true) {
        start      = us();
        /// @todo 移动速度在不同帧率下一致

        // 处理输入
        is_running = input->process(*scene, 1);

        // 更新场景
        scene->tick(1);

        // 清空缓冲区
        framebuffer->clear();

        draw3d_t draw3d(config, framebuffer, shader);
        auto     models = scene->get_visible_models();
        // 右对角线
        draw3d.line(0, config->HEIGHT - 1, config->WIDTH - 1, 0,
                    color_t::WHITE);
        // 左对角线
        draw3d.line(config->WIDTH - 1, config->HEIGHT - 1, 0, 0,
                    color_t::WHITE);
        // 水平平分线
        draw3d.line(config->WIDTH - 1, config->HEIGHT / 2, 0,
                    config->HEIGHT / 2, color_t::WHITE);
        // 垂直平分线
        draw3d.line(config->WIDTH / 2, 0, config->WIDTH / 2, config->HEIGHT - 1,
                    color_t::WHITE);

        shader.shader_data.model_matrix
          = get_model_matrix(vector4f_t(1000, 1000, 1000),
                             vector4f_t(0, 1, 1).normalize(), 45,
                             vector4f_t(config->WIDTH / 2, config->HEIGHT / 2,
                                        0));
        shader.shader_data.view_matrix = scene->get_current_camera().look_at();
        shader.shader_data.project_matrix = matrix4f_t();

        while (!models.empty()) {
            draw3d.model(models.front());
            models.pop();
        }

        // 填充窗口
        display->fill(framebuffer);
        frames++;
        end = us();
        sec += end - start;
        if (sec >= US2S) {
            std::cout << "fps_window: " << fps << std::endl;
            fps    = frames;
            frames = 0;
            sec    = 0;
        }
    }
    return;
}
