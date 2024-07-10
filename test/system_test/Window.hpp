#pragma once

#include <GLFW/glfw3.h>
#include <memory>

class Window {
public:
    Window(int width, int height, const char *title);
    ~Window();

    void swapBuffers();
    void pollEvents();

    bool shouldClose() const;

    GLFWwindow *getGLFWWindow() const;

    static void framebufferSizeCallback(GLFWwindow *window, int width, int height);

	void setupCallbacks();
	void setInputMode();

    void fillWindowWithBufferData(const u_int32_t* buffer);

private:
    void initGLFW();
    void createWindow();
    void initGLAD();
    void setWindowHints();

    void initRecTexture();
    void updateRecTexture(const u_int32_t* buffer);
    void drawRecTextureOnWindow();
    void destroyRecTexture();

private:
    int m_width, m_height;
    const char *m_title;
    GLFWwindow *m_window;
    GLuint m_texture;
};
