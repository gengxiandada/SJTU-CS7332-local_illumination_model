//
// Created by 程思浩 on 24-5-31.
//

#ifndef LOCAL_ILLUMINATION_MODEL_FRAMEBUFFER_H
#define LOCAL_ILLUMINATION_MODEL_FRAMEBUFFER_H


class FrameBuffer {
private:
    unsigned int m_renderer_ID;
public:
    FrameBuffer();

    ~FrameBuffer();

    void bind() const;

    void unbind() const;

    void addTexutre(unsigned int texture);
};


#endif //LOCAL_ILLUMINATION_MODEL_FRAMEBUFFER_H
