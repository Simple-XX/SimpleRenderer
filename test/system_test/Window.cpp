#include <glad/glad.h>

#include "Window.hpp"

#include <iostream>

Window::Window(int width, int height, const char* title) 
    : m_width(width), m_height(height), m_title(title) {
    
    std::cout << "Creating window..." << std::endl;

    initGLFW();

    setWindowHints();

    createWindow();

    glfwMakeContextCurrent(m_window);

    initGLAD();
}

Window::~Window() {
    if (m_texture) {
        destroyRecTexture();
    }
    if (m_window) {
        glfwDestroyWindow(m_window);
    }
    glfwTerminate();
}

void Window::initGLFW() {
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        exit(EXIT_FAILURE);
    }
}

void Window::createWindow() {
    m_window = glfwCreateWindow(m_width, m_height, m_title, nullptr, nullptr);
    if (!m_window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        exit(EXIT_FAILURE);
    }
}

bool Window::shouldClose() const {
    return glfwWindowShouldClose(m_window);
}

void Window::swapBuffers() {
    glfwSwapBuffers(m_window);
}

void Window::pollEvents() {
    glfwPollEvents();
}

GLFWwindow* Window::getGLFWWindow() const {
    return m_window;
}

void Window::setWindowHints() {
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

	glfwWindowHint(GLFW_SAMPLES, 4); // MSAA
}

void Window::initGLAD() {
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        exit(EXIT_FAILURE);
    }
}

void Window::framebufferSizeCallback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

void Window::setupCallbacks() {
	glfwSetFramebufferSizeCallback(m_window, Window::framebufferSizeCallback);
}

void Window::setInputMode() {
	glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
}

void Window::fillWindowWithBufferData(const u_int32_t* buffer) {
    initRecTexture();
    updateRecTexture(buffer);
    drawRecTextureOnWindow();
    destroyRecTexture();
}

void Window::initRecTexture() {
    glGenTextures(1, &m_texture);
    glBindTexture(GL_TEXTURE_2D, m_texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

void Window::updateRecTexture(const u_int32_t* buffer) {
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_width, m_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, buffer);
}

void Window::drawRecTextureOnWindow() {

    // glEnable(GL_TEXTURE_2D);
    // glBegin(GL_QUADS);
    // glTexCoord2f(0.0f, 0.0f); glVertex2f(-1.0f, -1.0f);
    // glTexCoord2f(1.0f, 0.0f); glVertex2f( 1.0f, -1.0f);
    // glTexCoord2f(1.0f, 1.0f); glVertex2f( 1.0f,  1.0f);
    // glTexCoord2f(0.0f, 1.0f); glVertex2f(-1.0f,  1.0f);
    // glEnd();
    // glDisable(GL_TEXTURE_2D);
}

void Window::destroyRecTexture() {
    glDeleteTextures(1, &m_texture);
}

