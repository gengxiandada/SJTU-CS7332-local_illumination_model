//
// Created by 程思浩 on 24-5-31.
//

#include "Renderer.h"
#include <iostream>

void GLClearError() {
    while (glGetError() != GL_NO_ERROR);
}

// Output error message.
bool GLLogCall(const char *function, const char *file, int line) {
    while (GLenum error = glGetError()) {
        std::cout << "[OpenGL Error] (" << error << ")" << function <<
                  " " << file << ":" << line << std::endl;
        return false;
    }

    return true;
}

void Renderer::draw(const VertexArray &va, const IndexBuffer &ib, const Shader &shader) const {
    va.bind(0);
    ib.bind();

    shader.bind();

    GLCall(glDrawElements(GL_TRIANGLES, ib.getCount(), GL_UNSIGNED_INT, nullptr));
}

void Renderer::clear() const {
    glClear(GL_COLOR_BUFFER_BIT);
}
