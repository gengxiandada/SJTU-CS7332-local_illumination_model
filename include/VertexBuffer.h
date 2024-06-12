//
// Created by 程思浩 on 24-5-31.
//

#ifndef OPENGL_TEST_VERTEXBUFFER_H
#define OPENGL_TEST_VERTEXBUFFER_H


#include "glm/glm.hpp"
#include <vector>

class VertexBuffer {
private:
    unsigned int m_renderer_ID;
public:
    VertexBuffer(const void *data, unsigned int size);

    VertexBuffer(const std::vector<glm::vec3> &data, unsigned int size);

    ~VertexBuffer();

    void bind() const;

    void unbind() const;
};


#endif //OPENGL_TEST_VERTEXBUFFER_H
