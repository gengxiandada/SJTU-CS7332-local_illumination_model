//
// Created by 程思浩 on 24-5-31.
//

#ifndef OPENGL_TEST_RENDERER_H
#define OPENGL_TEST_RENDERER_H


#include "GL/glew.h"
#include "cassert"
#include "VertexArray.h"
#include "IndexBuffer.h"
#include "Shader.h"

// Dealing with errors in OpenGL.
#define ASSERT(x) assert(x);
#define GLCall(x) GLClearError();\
        x;\
        ASSERT(GLLogCall(#x, __FILE__, __LINE__))

void GLClearError();

bool GLLogCall(const char *function, const char *file, int line);

class Renderer {
public:
    void draw(const VertexArray &va, const IndexBuffer &ib, const Shader &shader) const;

    void clear() const;
};


#endif //OPENGL_TEST_RENDERER_H
