#include <iostream>
#include <fstream>
#include <unordered_map>
#include "glm//glm.hpp"
#include <vector>

std::unordered_map<unsigned int, glm::vec3> axisOffs;

void getAxisOff(const std::string filePath, std::unordered_map<unsigned int, glm::vec3> &axisOffs){
    std::ifstream stream(filePath);

    std::string line;

    while (getline(stream, line)) {
        if (line.find("##") == std::string::npos && line != "") { // Skip if encounter comment lines or blank lines.
            if (line.find("# object") != std::string::npos) {
                line = line.assign(line, 8);
                unsigned int index = std::stoul(line);
                std::vector<float> offset;
                getline(stream, line);
                std::string::iterator it = line.begin();
                while (it != line.end()) {
                    std::string off;
                    while (it != line.end() && (*it) != ' ') {
                        off.push_back(*it);
                        it++;
                    }
                    offset.push_back(std::stof(off));
                    if ((*it) == ' ') {
                        it++;
                    }
                }
                axisOffs[index] = glm::vec3(offset[0], offset[1],offset[2]);
            } else {
                std::cout << "Failed to parse scene information from " << filePath << std::endl;
                return ;
            }
        }
    }
}

int main(){
    getAxisOff("res/objects/scene.txt", axisOffs);
    return 0;
}