#include "window.hpp"
#include <iostream>

Window::Window(int width, int height) : 
    width_(width), height_(height) {
    InitWindow(width, height, "Simple Renderer");
    SetTargetFPS(60);

    texture_ = LoadTextureFromImage(GenImageColor(width, height, BLANK));
}

Window::~Window() {
    UnloadTexture(texture_);
    CloseWindow();
}

void Window::Display(uint32_t *buffer) {
    while (!WindowShouldClose()) {
        UpdateTexture(texture_, buffer);
        BeginDrawing();
        ClearBackground(RAYWHITE);
        DrawTexture(texture_, 0, 0, WHITE);
        EndDrawing();
    }
}
