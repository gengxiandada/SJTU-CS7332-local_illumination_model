//
// Created by 程思浩 on 24-5-31.
//

#ifndef LOCAL_ILLUMINATION_MODEL_LIGHTS_H
#define LOCAL_ILLUMINATION_MODEL_LIGHTS_H


#include <vector>
#include "glm/glm.hpp"

class Lights {
private:
    std::vector<glm::vec3> m_lights_pos;
    unsigned int m_count = 0;
public:
    Lights() {};

    ~Lights() {};

    bool loadLights(const std::string &filePath);

    glm::vec3 getLightPos(int index) const { return m_lights_pos[index]; };

    unsigned int getLightNum() const { return m_count; };

private:
    std::vector<glm::vec3> parseLights(const std::string &filePath);
};


#endif //LOCAL_ILLUMINATION_MODEL_LIGHTS_H
