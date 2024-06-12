//
// Created by 程思浩 on 24-5-31.
//

#include "VertexBuffer.h"
#include "Renderer.h"

VertexBuffer::VertexBuffer(const void *data, unsigned int size) {
    glGenBuffers(1, &m_renderer_ID);
    glBindBuffer(GL_ARRAY_BUFFER, m_renderer_ID);
    glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);
}

VertexBuffer::VertexBuffer(const std::vector<glm::vec3> &data, unsigned int size) {
    glGenBuffers(1, &m_renderer_ID);
    glBindBuffer(GL_ARRAY_BUFFER, m_renderer_ID);
    glBufferData(GL_ARRAY_BUFFER, size, data.data(), GL_STATIC_DRAW);
}

VertexBuffer::~VertexBuffer() {
    glDeleteBuffers(1, &m_renderer_ID);
}

void VertexBuffer::bind() const {
    glBindBuffer(GL_ARRAY_BUFFER, m_renderer_ID);
}

void VertexBuffer::unbind() const {
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}
