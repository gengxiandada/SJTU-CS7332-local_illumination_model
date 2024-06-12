//
// Created by 程思浩 on 24-5-31.
//

#ifndef OPENGL_TEST_INDEXBUFFER_H
#define OPENGL_TEST_INDEXBUFFER_H


class IndexBuffer {
private:
    unsigned int m_renderer_ID;
    unsigned int m_count;
public:
    IndexBuffer(const unsigned int *data, unsigned int count);

    ~IndexBuffer();

    void bind() const;

    void unbind() const;

    inline unsigned int getCount() const { return m_count; };
};


#endif //OPENGL_TEST_INDEXBUFFER_H
