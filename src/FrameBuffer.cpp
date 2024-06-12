//
// Created by 程思浩 on 24-5-31.
//

#include "FrameBuffer.h"
#include "GL/glew.h"

FrameBuffer::FrameBuffer() {
    glGenFramebuffers(1, &m_renderer_ID);
}

FrameBuffer::~FrameBuffer() {

}

void FrameBuffer::bind() const {
    glBindFramebuffer(GL_FRAMEBUFFER, m_renderer_ID);
}

void FrameBuffer::unbind() const {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void FrameBuffer::addTexutre(unsigned int texture) {
    bind();
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, texture, 0);
}
