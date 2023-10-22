
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
    const std::vector<std::shared_ptr<framebuffer_t>> &_framebuffers)
    : state(_state), scene(_scene), framebuffers(_framebuffers) {}

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
        static uint32_t rotate = 0;
        // 旋转
        rotate = (rotate + 1) % 360;
        Eigen::AngleAxis<float> vec(rotate / M_PI, Eigen::Vector3f(0, 1, 0));
        auto mat = vec.matrix();
        auto rotation = matrix4f_t();
        rotation.setIdentity();
        rotation.block<3, 3>(0, 0) = mat;
        // 平移到屏幕中间
        auto translate_mat = matrix4f_t();
        translate_mat.setIdentity();
        translate_mat(0, 3) = WIDTH / 2;
        translate_mat(1, 3) = HEIGHT / 2;
        translate_mat(2, 3) = 0;
        // 缩放
        auto scale_mat = matrix4f_t();
        scale_mat.setIdentity();
        scale_mat.diagonal()[0] = 64;
        scale_mat.diagonal()[1] = 64;
        scale_mat.diagonal()[2] = 64;
        shader->shader_data.model_matrix = translate_mat * rotation * scale_mat;

        // 绘制场景
        if (state->obj_index >= scene->get_models().size()) {
          state->obj_index = 0;
        }
        i->scene(*shader, *scene, state->obj_index, state->draw_line,
                 state->draw_triangle);

        // 将可显示标记置位
        i->displayable.store(true);
      }
    }

    frames++;
    end = us();
    sec += end - start;
    if (sec >= US2S) {
      SPDLOG_LOGGER_INFO(SRLOG, "fps_window: {}", fps);
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
