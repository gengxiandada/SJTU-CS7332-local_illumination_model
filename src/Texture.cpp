//
// Created by 程思浩 on 24-5-31.
//

#include "Texture.h"
#include "vendor/stb_image/stb_image.h"

Texture::Texture(const std::string &path, unsigned int count, textureType type, unsigned width, unsigned height)
        : m_file_path(path), m_local_buffer(nullptr), m_width(width), m_height(height), m_BPP(0), m_count(count) {
    stbi_set_flip_vertically_on_load(1);
    m_local_buffer = stbi_load(path.c_str(), &m_width, &m_height, &m_BPP, 4);

    m_renderer_ID = new unsigned int(m_count);
    glGenTextures(count, m_renderer_ID);

    for (size_t i = 0; i < count; i++) {
        glBindTexture(GL_TEXTURE_2D, m_renderer_ID[i]);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        if (type == textureType::RGB) {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, m_width, m_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, m_local_buffer);

            if (m_local_buffer) {
                stbi_image_free(m_local_buffer);
            }
        } else if (type == textureType::Depth) {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, m_width, m_height, 0, GL_DEPTH_COMPONENT, GL_FLOAT,
                         nullptr);
        }
    }

    glBindTexture(GL_TEXTURE_2D, 0);
}

Texture::~Texture() {
    glDeleteTextures(m_count, m_renderer_ID);
    delete m_renderer_ID;
}

void Texture::bind(unsigned int slot) const {
    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(GL_TEXTURE_2D, *(m_renderer_ID + slot));
}

void Texture::unBind() const {
    glBindTexture(GL_TEXTURE_2D, 0);
}
