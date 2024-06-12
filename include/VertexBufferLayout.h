//
// Created by 程思浩 on 24-5-31.
//

#ifndef OPENGL_TEST_VERTEXBUFFERLAYOUT_H
#define OPENGL_TEST_VERTEXBUFFERLAYOUT_H


#include <vector>
#include "Renderer.h"

struct vertexBufferElement {
    unsigned int type;
    unsigned int count;
    unsigned char normalized;

    static unsigned int getSizeOfType(unsigned int type) {
        switch (type) {
            case GL_FLOAT:
                return 4;
            case GL_UNSIGNED_INT:
                return 4;
            case GL_UNSIGNED_BYTE:
                return 1;
        }
        ASSERT(false);
        return 0;
    }
};

class VertexBufferLayout {
private:
    std::vector<vertexBufferElement> m_elements; // Different attributes of vertices.
    unsigned int m_stride;
public:
    VertexBufferLayout()
            : m_stride(0) {};

    ~VertexBufferLayout() {}

    template<class T>
    void push(unsigned int count) {
        ASSERT(false);
    }

    template<>
    void push<float>(unsigned int count) {
        m_elements.push_back({GL_FLOAT, count, GL_FALSE});
        m_stride += count * vertexBufferElement::getSizeOfType(GL_FLOAT);
    }

    template<>
    void push<unsigned int>(unsigned int count) {
        m_elements.push_back({GL_UNSIGNED_INT, count, GL_FALSE});
        m_stride += count * vertexBufferElement::getSizeOfType(GL_UNSIGNED_INT);
    }

    template<>
    void push<unsigned char>(unsigned int count) {
        m_elements.push_back({GL_UNSIGNED_BYTE, count, GL_TRUE});
        m_stride += count * vertexBufferElement::getSizeOfType(GL_UNSIGNED_BYTE);
    }

    inline const std::vector<vertexBufferElement> &getElements() const { return m_elements; };

    inline unsigned int getStride() const { return m_stride; };
};


#endif //OPENGL_TEST_VERTEXBUFFERLAYOUT_H
