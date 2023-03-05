
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

#include "condition_variable"
#include "thread"

#include "render.h"

render_t::render_t(const std::shared_ptr<config_t>&  _config,
                   const std::shared_ptr<scene_t>&   _scene,
                   const std::shared_ptr<display_t>& _display,
                   const std::shared_ptr<input_t>&   _input)
    : config(_config), scene(_scene), display(_display), input(_input) {
    // 初始化缓冲区
    auto f1 = std::make_shared<framebuffer_t>(1920, 1080);
    auto f2 = std::make_shared<framebuffer_t>(1920, 1080);
    auto f3 = std::make_shared<framebuffer_t>(1920, 1080);

    framebuffer.push_back(f1);
    framebuffer.push_back(f2);
    framebuffer.push_back(f3);

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

std::mutex              mutex;
std::condition_variable data_cond;

/// @todo 验证 std::condition_variable 的正确性
/// @todo 保证时序正确
void                    display_func(
                     const std::atomic_bool&                            _is_running,
                     const std::vector<std::shared_ptr<framebuffer_t>>& _framebuffer,
                     std::shared_ptr<display_t>                         _display) {
    while (1) {
        // 等待获取锁
        for (const auto& i : _framebuffer) {
            std::unique_lock<std::mutex> lk(mutex);
            data_cond.wait(lk, [&] {
                return (i->displayable == true);
            });

            // 填充窗口
            _display->fill(i);
            i->displayable = false;
        }
        if (_is_running == false) {
            break;
        }
    }
    return;
}

void render_t::loop(void) {
    std::thread display_thread = std::thread(display_func, std::ref(is_running),
                                             std::ref(framebuffer), display);
    display_thread.detach();

    uint64_t sec    = 0;
    uint32_t frames = 0;
    uint32_t fps    = 0;
    auto     start  = us();
    auto     end    = us();

    // 主循环
    while (is_running == true) {
        start      = us();
        /// @todo 移动速度在不同帧率下一致

        // 处理输入
        is_running = input->process(*scene, 1);

        // 更新场景
        scene->tick(1);

        // 绘制缓冲区
        for (auto& i : framebuffer) {
            // 加锁
            std::lock_guard<std::mutex> lk(mutex);
            // 如果当前缓冲区还没有填充
            if (i->displayable == false) {
                // 清空缓冲区
                i->clear();

                // 右对角线
                i->line(0, config->HEIGHT - 1, config->WIDTH - 1, 0,
                        color_t::WHITE);
                // 左对角线
                i->line(config->WIDTH - 1, config->HEIGHT - 1, 0, 0,
                        color_t::WHITE);
                // 水平平分线
                i->line(config->WIDTH - 1, config->HEIGHT / 2, 0,
                        config->HEIGHT / 2, color_t::WHITE);
                // 垂直平分线
                i->line(config->WIDTH / 2, 0, config->WIDTH / 2,
                        config->HEIGHT - 1, color_t::WHITE);

                // 设置 mvp 矩阵
                shader.shader_data.model_matrix
                  = get_model_matrix(vector4f_t(1000, 1000, 1000),
                                     vector4f_t(0, 1, 1).normalize(), 45,
                                     vector4f_t(config->WIDTH / 2,
                                                config->HEIGHT / 2, 0));
                shader.shader_data.view_matrix    = matrix4f_t();
                // shader.shader_data.view_matrix
                //   = scene->get_current_camera().look_at();
                shader.shader_data.project_matrix = matrix4f_t();

                // 绘制场景
                i->scene(shader, *scene);

                // 将可显示标记置位
                i->displayable = true;

                // 通知等待线程
                data_cond.notify_one();
            }
        }

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
