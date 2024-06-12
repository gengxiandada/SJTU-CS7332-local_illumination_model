//
// Created by 程思浩 on 24-5-31.
//

#ifndef OPENGL_TEST_VERTEXARRAY_H
#define OPENGL_TEST_VERTEXARRAY_H


#include "VertexBuffer.h"

class VertexBufferLayout;

class VertexArray {
private:
    unsigned int *m_renderer_ID;
    unsigned int m_count;
public:
    VertexArray(unsigned int count);

    ~VertexArray();

    void addBuffer(unsigned int index,const VertexBuffer &vb, const VertexBufferLayout &layout);

    void addBuffers(unsigned int index,const std::vector<VertexBuffer> &buffers, const std::vector<VertexBufferLayout> &layout, unsigned num);

    void bind(unsigned int index) const;

    void unbind() const;
};


#endif //OPENGL_TEST_VERTEXARRAY_H
