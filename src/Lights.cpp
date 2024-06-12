//
// Created by 程思浩 on 24-5-31.
//

#include "Lights.h"
#include <fstream>
#include <iostream>

bool Lights::loadLights(const std::string &filePath) {
    m_lights_pos = parseLights(filePath);
    if (!m_lights_pos.empty()) {
        return true;
    } else {
        return false;
    }
}

std::vector<glm::vec3> Lights::parseLights(const std::string &filePath) {
    std::ifstream stream(filePath);

    std::string line;
    std::vector<glm::vec3> lights;

    while (getline(stream, line)) {
        if (line.find('#') == std::string::npos && line != "") { // Skip if encounter comment lines or blank lines.
            if (line.find("x/y/z: ") != std::string::npos) {
                std::vector<float> coordinate;
                std::string::iterator it = line.begin() + 7;
                while (it != line.end()) {
                    std::string pos;
                    while (it != line.end() && (*it) != '/') {
                        pos += *(it++);
                    }
                    coordinate.push_back(stof(pos));
                    if (it != line.end() && (*it) == '/') {
                        it++;
                    }
                }
                lights.push_back({coordinate[0], coordinate[1], coordinate[2]});
            } else {
                std::cout << "Failed to parse light coordinates from " << filePath << std::endl;
                return std::vector<glm::vec3>{};
            }
        }
    }

    m_count = lights.size();

    return lights;
}
