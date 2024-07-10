#pragma once

#include <glad/glad.h>

class IndexBuffer {
public:
    IndexBuffer(const GLuint* data, GLuint count);
    ~IndexBuffer();

    inline GLuint getCount() const { return m_Count; }

    void bind() const;
    void unBind() const;

private:
    GLuint m_RendererID;
    GLuint m_Count;
};