/**
 * @file window.hpp
 * @author ZzzhHe
 * @brief A simple display abstraction by Raylib
 * @version 0.1
 * @date 2024-07-11
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#ifndef SIMPLERENDER_TEST_SYSTEM_TEST_DISPLAY_H_
#define SIMPLERENDER_TEST_SYSTEM_TEST_DISPLAY_H_

#include "raylib.h"
#include <cstdint>

class Window {
public:
    Window(int width, int height);

    ~Window();

    Window() = delete;
    Window(const Window &display) = delete;
    Window(Window &&display) = delete;
    Window &operator=(const Window &display) = delete;
    Window &operator=(Window &&display) = delete;

    void Display(uint32_t *buffer);

private:
    static constexpr const char *kWindowTitle = "SimpleRenderer";

    int m_width, m_height;

    Texture2D m_texture;
};

#endif
