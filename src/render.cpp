
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

#include <future>

#include "render.h"
#include "status.h"

render_t::render_t(
    const std::shared_ptr<state_t> &_state,
    const std::shared_ptr<scene_t> &_scene,
    const std::shared_ptr<input_t> &_input,
    const std::vector<std::shared_ptr<framebuffer_t>> &_framebuffers)
    : state(_state), scene(_scene), input(_input), framebuffers(_framebuffers) {
}

/// @todo 验证 std::condition_variable 的正确性
/// @todo 保证时序正确
auto render_t::loop() -> state_t::status_t {
  uint64_t sec = 0;
  uint32_t frames = 0;
  uint32_t fps = 0;
  auto start = us();
  auto end = us();

  // 主循环
  while (state->status != state_t::STOP) {
    start = us();
    /// @todo 移动速度在不同帧率下一致

    // 处理输入
    auto is_running = input->process(*scene, 1);
    if (!is_running) {
      state->status.store(state_t::STOP);
    }

    // 更新场景
    scene->tick(1);

    // 绘制缓冲区
    for (auto &i : framebuffers) {
      // 加锁
      // 如果当前缓冲区还没有填充
      if (!i->displayable.load()) {
        // 清空缓冲区
        i->clear();

        // 右对角线
        i->line(0, HEIGHT - 1, WIDTH - 1, 0, color_t::WHITE);
        // 左对角线
        i->line(WIDTH - 1, HEIGHT - 1, 0, 0, color_t::WHITE);
        // 水平平分线
        i->line(WIDTH - 1, HEIGHT / 2, 0, HEIGHT / 2, color_t::WHITE);
        // 垂直平分线
        i->line(WIDTH / 2, 0, WIDTH / 2, HEIGHT - 1, color_t::WHITE);

        // 设置 mvp 矩阵
        shader->shader_data.model_matrix = get_model_matrix(
            vector3f_t(1000, 1000, 1000), vector3f_t(0, 1, 1).normalized(),
            M_PI_4, vector3f_t(WIDTH / 2, HEIGHT / 2, 0));
        shader->shader_data.view_matrix = matrix4f_t().setIdentity();
        // shader->shader_data.view_matrix
        //   = scene->get_current_camera().look_at();
        shader->shader_data.project_matrix = matrix4f_t().setIdentity();

        // 绘制场景
        i->scene(*shader, *scene);

        // 将可显示标记置位
        i->displayable.store(true);
      }
    }

    frames++;
    end = us();
    sec += end - start;
    if (sec >= US2S) {
      std::cout << "fps_window: " << fps << std::endl;
      fps = frames;
      frames = 0;
      sec = 0;
    }
  }
  return state_t::STOP;
}

auto render_t::run() -> std::future<state_t::status_t> {
  return std::async(std::launch::async, &render_t::loop, this);
}
