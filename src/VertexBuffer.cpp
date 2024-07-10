#include "VertexBuffer.hpp"
#include "GLDebug.hpp"

VertexBuffer::VertexBuffer(const void* data, GLuint size) {
    GLCall(glGenBuffers(1, &m_RendererID));
    GLCall(glBindBuffer(GL_ARRAY_BUFFER, m_RendererID));
    GLCall(glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW));
}

VertexBuffer::~VertexBuffer() {
    GLCall(glDeleteBuffers(1, &m_RendererID));
}

void VertexBuffer::bind() const {
    GLCall(glBindBuffer(GL_ARRAY_BUFFER, m_RendererID));
}

void VertexBuffer::unBind() const {
    GLCall(glBindBuffer(GL_ARRAY_BUFFER, 0));
}