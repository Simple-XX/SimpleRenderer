#pragma once

#include <glad/glad.h>

class VertexBuffer {
public:
    VertexBuffer(const void* data, GLuint size);
    ~VertexBuffer();

    void bind() const;
    void unBind() const;
    
private:
    GLuint m_RendererID;
};
