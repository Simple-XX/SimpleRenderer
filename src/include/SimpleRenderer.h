
/**
 * @file SimpleRenderer.h
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

#ifndef SIMPLERENDER_SIMPLERENDER_H
#define SIMPLERENDER_SIMPLERENDER_H

#include <array>
#include <cstdint>
#include <string_view>
#include <vector>

#include "framebuffer.h"
#include "light.h"
#include "render.h"
#include "scene.h"

namespace SimpleRenderer {
template <size_t _W, size_t _H> class SimpleRenderer {
public:
  SimpleRenderer()
      : render(std::ref(state), std::ref(scene), std::ref(framebuffers)) {
    // 初始化日志系统
    log_init();

    // 添加缓冲区
    framebuffers.emplace_back(std::make_shared<framebuffer_t>(_W, _H));
    framebuffers.emplace_back(std::make_shared<framebuffer_t>(_W, _H));
    framebuffers.emplace_back(std::make_shared<framebuffer_t>(_W, _H));
    // 设置光照
    scene->set_light(light_t());
    // 初始化渲染器
    // render = render_t(std::ref(state), std::ref(scene),
    // std::ref(framebuffers));
  }

  /// @name 默认构造/析构函数
  /// @{
  SimpleRenderer(const SimpleRenderer &_simplerenderer) = default;
  SimpleRenderer(SimpleRenderer &&_simplerenderer) = default;
  auto operator=(const SimpleRenderer &_simplerenderer)
      -> SimpleRenderer & = default;
  auto operator=(SimpleRenderer &&_simplerenderer)
      -> SimpleRenderer & = default;
  virtual ~SimpleRenderer() = default;
  /// @}

  void add_model(const std::string &_obj_path) {
    auto model = model_t(_obj_path);
    scene->add_model(model);
  }

  auto get_buffer() const -> const std::array<uint32_t, _W * _H> & {
    while (1) {
      for (const auto &i : framebuffers) {
        if (i->displayable == true) {
          return i;
        }
      }
    }
  }

  void loop() {
    auto render_ret = render.run();
    // return render_ret.get();
  }

private:
  std::shared_ptr<scene_t> state;
  std::shared_ptr<scene_t> scene;
  std::vector<std::shared_ptr<framebuffer_t>> framebuffers;
  render_t render;
};

} // namespace SimpleRenderer

#endif /* SIMPLERENDER_SIMPLERENDER_H */
