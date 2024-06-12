//
// Created by 程思浩 on 24-5-31.
//

#include "VertexArray.h"
#include "VertexBufferLayout.h"

VertexArray::VertexArray(unsigned int count)
:m_count(count){
    m_renderer_ID = new unsigned int(m_count);
    glGenVertexArrays(m_count, m_renderer_ID);
}

VertexArray::~VertexArray() {
    glDeleteVertexArrays(m_count, m_renderer_ID);
    delete m_renderer_ID;
}

void VertexArray::addBuffer(unsigned int index, const VertexBuffer &vb, const VertexBufferLayout &layout) {
    bind(index);
    vb.bind();
    const auto &elements = layout.getElements();
    unsigned int offset = 0;
    for (unsigned int i = 0; i < elements.size(); i++) {
        const auto &element = elements[i];
        glEnableVertexAttribArray(i);
        glVertexAttribPointer(i, element.count, element.type, element.normalized, layout.getStride(),
                              (const void *) offset);
        offset += element.count * vertexBufferElement::getSizeOfType(element.type);
    }
}

void VertexArray::bind(unsigned int index) const {
    glBindVertexArray(*(m_renderer_ID+index));
}

void VertexArray::unbind() const {
    glBindVertexArray(0);
}
