//
// Created by 程思浩 on 24-5-31.
//

#ifndef OPENGL_TEST_TEXTURE_H
#define OPENGL_TEST_TEXTURE_H


#include "Renderer.h"
#include <iostream>

enum class textureType {
    RGB = 0, Depth = 1
};

class Texture {
private:
    unsigned int *m_renderer_ID;
    std::string m_file_path;
    unsigned char *m_local_buffer;
    int m_width, m_height, m_BPP;
    unsigned int m_count;
public:
    Texture(const std::string &path, unsigned int count, textureType type = textureType::RGB, unsigned width = 0,
            unsigned height = 0);

    ~Texture();

    void bind(unsigned int slot = 0) const;

    void unBind() const;

    inline int getWidth() const { return m_width; }

    inline int getHeight() const { return m_height; }

    inline unsigned int getID(unsigned int slot) const {
//        std::cout<<*(m_renderer_ID + slot)<<std::endl;
        return *(m_renderer_ID + slot);
    }
};


#endif //OPENGL_TEST_TEXTURE_H
