
/**
 * @file SimpleRenderer.cpp
 * @brief SimpleRenderer 接口头文件
 * @author Zone.N (Zone.Niuzh@hotmail.com)
 * @version 1.0
 * @date 2023-10-23
 * @copyright MIT LICENSE
 * https://github.com/Simple-XX/SimpleRenderer
 * @par change log:
 * <table>
 * <tr><th>Date<th>Author<th>Description
 * <tr><td>2023-10-23<td>Zone.N<td>创建文件
 * </table>
 */

#include <cstdint>
#include <string_view>
#include <vector>

#include "framebuffer.h"
#include "light.h"
#include "render.h"
#include "scene.h"

namespace SimpleRenderer {

static auto state = std::make_shared<state_t>();
static auto scene = std::make_shared<scene_t>();
static std::vector<std::shared_ptr<framebuffer_t>> framebuffers;
static auto render = render_t();

template <size_t _W, size_t _H> SimpleRenderer::SimpleRenderer() {
  // 初始化日志系统
  log_init();
  // 添加缓冲区
  framebuffers.emplace_back(std::make_shared<framebuffer_t>(_W, _H));
  framebuffers.emplace_back(std::make_shared<framebuffer_t>(_W, _H));
  framebuffers.emplace_back(std::make_shared<framebuffer_t>(_W, _H));
  // 设置光照
  scene->set_light(light_t());
  // 初始化渲染器
  render = render_t(std::ref(state), std::ref(scene), std::ref(framebuffers));
}

template <size_t _W, size_t _H>
void SimpleRenderer::add_model(const std::string_view &_obj_path) {
  auto model = model_t(_obj_path);
  scene->add_model(model);
}

template <size_t _W, size_t _H>
auto SimpleRenderer::get_buffer() const
    -> const std::array<uint32_t, _W * _H> & {
  while (1) {
    for (const auto &i : framebuffers) {
      if (i->displayable == true) {
        return i;
      }
    }
  }
}

template <size_t _W, size_t _H> void SimpleRenderer::loop() {
  auto render_ret = render.run();
  return render_ret.get();
}

} // namespace SimpleRenderer
