//
// Created by 程思浩 on 24-5-31.
//

#include "IndexBuffer.h"
#include "Renderer.h"

IndexBuffer::IndexBuffer(const unsigned int *data, unsigned int count)
        : m_count(count) {
    glGenBuffers(1, &m_renderer_ID);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_renderer_ID);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(unsigned int), data, GL_STATIC_DRAW);
}

IndexBuffer::~IndexBuffer() {
    glDeleteBuffers(1, &m_renderer_ID);
}

void IndexBuffer::bind() const {
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_renderer_ID);
}

void IndexBuffer::unbind() const {
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}
