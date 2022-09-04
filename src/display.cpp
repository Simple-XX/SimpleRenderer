
/**
 * @file display.cpp
 * @brief 显示
 * @author Zone.N (Zone.Niuzh@hotmail.com)
 * @version 1.0
 * @date 2022-09-04
 * @copyright MIT LICENSE
 * https://github.com/Simple-XX/SimpleRenderer
 * @par change log:
 * <table>
 * <tr><th>Date<th>Author<th>Description
 * <tr><td>2022-09-04<td>Zone.N<td>创建文件
 * </table>
 */

#include <iostream>
#ifdef __APPLE__
#define GL_SILENCE_DEPRECATION
#endif
#include <GLFW/glfw3.h>
#include "display.h"
#include "common.h"

/**
 * @brief 显示循环
 * @param  _window          未使用
 * @param  _current_time    未使用
 * @param  _pixels          要写入的像素
 */
static void display(GLFWwindow *, double, const uint32_t *_pixels) {
    glDrawPixels(WIDTH, HEIGHT, GL_RGBA, GL_UNSIGNED_INT, _pixels);
}

/**
 * @brief 设置视窗大小回调函数
 * @param  _window          未使用
 * @param  _width           新的宽度
 * @param  _height          新的高度
 */
static void framebuffer_size_callback(GLFWwindow *, int _width, int _height) {
    glViewport(0, 0, _width, _height);
}

/**
 * @brief 处理输入回调函数
 * @param  _window          窗口
 */
static void processInput(GLFWwindow *_window) {
    if (glfwGetKey(_window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(_window, true);
    }
}

void show_window(const framebuffer_t &_framebuffer) {
    if (glfwInit() != GLFW_TRUE) {
        std::cerr << "glfwInit failed." << std::endl;
        return;
    }
    // 设置主版本号
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    // 设置次版本号
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
    // 创建窗口
    auto window = glfwCreateWindow(_framebuffer.WIDTH, _framebuffer.HEIGHT,
                                   "SimpleRenderer", nullptr, nullptr);
    if (window == nullptr) {
        std::cerr << "fail to create window" << std::endl;
        glfwTerminate();
        return;
    }

    // 创建上下文
    glfwMakeContextCurrent(window);

    // 设置视窗
    glViewport(0, 0, _framebuffer.WIDTH, _framebuffer.HEIGHT);
    // 设置视窗大小回调函数
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // 进入主循环
    while (glfwWindowShouldClose(window) == false) {
        // 处理输入
        processInput(window);
        // 刷新显示缓存
        display(window, glfwGetTime(), _framebuffer.get_color_buffer());
        // 交换缓冲区
        glfwSwapBuffers(window);
        // 处理事件
        glfwPollEvents();
    }

    glfwTerminate();
    return;
}
