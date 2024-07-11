#include "window.hpp"
#include <raylib.h>
#include <iostream>

Window::Window(int width, int height) : 
    m_width(width), m_height(height) {
    InitWindow(width, height, "Simple Renderer");
    SetTargetFPS(60);

    m_texture = LoadTextureFromImage(GenImageColor(width, height, BLANK));
}

Window::~Window() {
    UnloadTexture(m_texture);
    CloseWindow();
}

void Window::Display(uint32_t *buffer) {
    while (!WindowShouldClose()) {
        UpdateTexture(m_texture, buffer);
        BeginDrawing();
        ClearBackground(RAYWHITE);
        DrawTexture(m_texture, 0, 0, WHITE);
        EndDrawing();
    }
}
