
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
#include "draw3d.h"

render_t::render_t(const std::shared_ptr<scene_t>       _scene,
                   const std::shared_ptr<display_t>     _display,
                   const std::shared_ptr<framebuffer_t> _framebuffer,
                   const std::shared_ptr<input_t>       _input)
    : scene(_scene),
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
    scene = _render.scene;
    return *this;
}

void render_t::loop(void) {
    uint64_t sec            = 0;
    uint32_t frames         = 0;
    uint32_t fps            = 0;
    auto     start          = us();
    auto     end            = us();
    auto     is_should_quit = true;
    // 主循环
    while (is_should_quit == true) {
        start          = us();
        // 处理输入
        /// @todo 移动速度在不同帧率下一致
        is_should_quit = input->process(1);
        // 填充窗口
        // framebuffer->clear();
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
