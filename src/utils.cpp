//
// Created by 程思浩 on 24-5-31.
//

#define TINYOBJLOADER_IMPLEMENTATION

#include "GL/glew.h"
#include "fstream"
#include "sstream"
#include <iostream>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "tiny_obj_loader.h"

bool genShaderSrc(const std::string &filePath, unsigned int num) {
    std::ifstream inFile(filePath);
    if (!inFile.is_open()) {
        std::cerr << "Could not read the shader!" << std::endl;
        return false;
    }

    std::stringstream buffer;
    buffer << inFile.rdbuf();
    std::string temp = buffer.str();
    inFile.close();

    size_t pos = 0;
    while ((pos = temp.find("LIGHT_NUM", pos)) != std::string::npos) {
        std::string replace = std::to_string(num);
        temp.replace(pos, 9, replace);
        pos += replace.length();
    }

    std::ofstream outFile(filePath + ".cache");
    if (!outFile.is_open()) {
        std::cerr << "Could not generate the shader!" << std::endl;
        return false;
    }
    outFile << temp;
    outFile.close();

    return true;
}

void processInput(GLFWwindow *window, glm::vec3 &cameraPos, glm::vec3 &cameraFront, glm::vec3 &cameraUp,
                  float &cameraSpeed) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        cameraPos += cameraSpeed * cameraFront;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        cameraPos -= cameraSpeed * cameraFront;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;

    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
        glm::mat4 rotationMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(1.0f), cameraUp);
        cameraFront = glm::vec3(rotationMatrix * glm::vec4(cameraFront, 1.0f));
    }
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
        glm::mat4 rotationMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(-1.0f), cameraUp);
        cameraFront = glm::vec3(rotationMatrix * glm::vec4(cameraFront, 1.0f));
    }
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
        glm::mat4 rotationMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(1.0f),
                                               glm::normalize(glm::cross(cameraFront, cameraUp)));
        cameraFront = glm::vec3(rotationMatrix * glm::vec4(cameraFront, 1.0f));
    }
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
        glm::mat4 rotationMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(-1.0f),
                                               glm::normalize(glm::cross(cameraFront, cameraUp)));
        cameraFront = glm::vec3(rotationMatrix * glm::vec4(cameraFront, 1.0f));
    }
}

// Function to load the OBJ file.
bool loadOBJ(const char *path, std::vector<glm::vec3> &vertices, std::vector<glm::vec3> &normals, const glm::vec3 &offset) {
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;

    if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, path)) {
        std::cerr << warn << err << std::endl;
        return false;
    }

    for (const auto &shape: shapes) {
        for (const auto &index: shape.mesh.indices) {
            glm::vec3 vertex = {
                    attrib.vertices[3 * index.vertex_index + 0] + offset.x,
                    attrib.vertices[3 * index.vertex_index + 1] + offset.y,
                    attrib.vertices[3 * index.vertex_index + 2] + offset.z
            };
            vertices.push_back(vertex);

            if (!attrib.normals.empty()) {
                glm::vec3 normal = {
                        attrib.normals[3 * index.normal_index + 0],
                        attrib.normals[3 * index.normal_index + 1],
                        attrib.normals[3 * index.normal_index + 2]
                };
                normals.push_back(normal);
            }
        }
    }

    return true;
}

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
                    if (it != line.end() && (*it) == ' ') {
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