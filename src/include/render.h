
/**
 * @file render.h
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

#ifndef SIMPLERENDER_RENDER_H
#define SIMPLERENDER_RENDER_H

#include "cstdint"
#include "memory"

#include "config.h"
#include "default_shader.h"
#include "display.h"
#include "draw3d.h"
#include "framebuffer.h"
#include "input.h"
#include "scene.h"
#include "shader.h"

/**
 * @brief 渲染抽象
 */
class render_t {
private:
    /// @brief 配置信息
    std::shared_ptr<config_t>      config;
    /// @brief 场景
    std::shared_ptr<scene_t>       scene;
    /// @brief 显示
    std::shared_ptr<display_t>     display;
    /// @brief 缓冲
    std::shared_ptr<framebuffer_t> framebuffer;
    /// @brief 输入
    std::shared_ptr<input_t>       input;

    default_shader_t               shader;

public:
    /**
     * @brief 空构造函数
     */
    render_t(void) = delete;

    /**
     * @brief 构造函数
     * @param _config           配置信息
     * @param _scene            场景
     * @param _display          显示
     * @param _framebuffer      缓冲区
     * @param _input            输入
     */
    explicit render_t(const std::shared_ptr<config_t>&      _config,
                      const std::shared_ptr<scene_t>&       _scene,
                      const std::shared_ptr<display_t>&     _display,
                      const std::shared_ptr<framebuffer_t>& _framebuffer,
                      const std::shared_ptr<input_t>&       _input);

    /**
     * @brief 构造函数
     * @param  _render          另一个 render
     */
    render_t(const render_t& _render);

    /**
     * @brief 析构函数
     */
    ~render_t(void);

    /**
     * @brief 赋值
     * @param  _render          另一个 render
     * @return render_t&        结果
     */
    render_t& operator=(const render_t& _render);

    /**
     * @brief 渲染循环
     */
    void      loop(void);
};

#endif /* SIMPLERENDER_RENDER_H */
