#pragma once

#include "GLDebug.hpp"

#include <vector>
#include <GLFW/glfw3.h>

struct VertexBufferLayoutElement {
    unsigned int type;
    unsigned int count;
    unsigned char normalized;

    static unsigned int getSizeofType(unsigned int type) {
        switch (type) {
            case GL_FLOAT:          return 4; 
            case GL_UNSIGNED_INT:   return 4; 
            case GL_UNSIGNED_BYTE:  return 1; 
        }
        ASSERT(false);
        return 0;
    }
};

class VertexBufferLayout {
public:
    VertexBufferLayout()
        : m_Stride(0) {}

    template<typename T>
    void push(unsigned int count) {
        // static_assert(false);
    }

    template<>
    void push<float>(unsigned int count) {
        m_Elements.push_back({GL_FLOAT, count, GL_FALSE});
        m_Stride += count * VertexBufferLayoutElement::getSizeofType(GL_FLOAT);
    }

    template<>
    void push<unsigned int>(unsigned int count) {
        m_Elements.push_back({GL_UNSIGNED_INT, count, GL_FALSE});
        m_Stride += count * VertexBufferLayoutElement::getSizeofType(GL_UNSIGNED_INT);
    }

    template<>
    void push<unsigned char>(unsigned int count) {
        m_Elements.push_back({GL_UNSIGNED_BYTE, count, GL_TRUE});
        m_Stride += count * VertexBufferLayoutElement::getSizeofType(GL_UNSIGNED_BYTE);
    }

    inline const std::vector<VertexBufferLayoutElement>& getElements() const {return m_Elements;}
    inline unsigned int getStride() const {return m_Stride;}

private:
    std::vector<VertexBufferLayoutElement> m_Elements;
    unsigned int m_Stride;
};
