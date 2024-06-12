//
// Created by 程思浩 on 24-5-31.
//

#include "Shader.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <glm/gtc/type_ptr.hpp>
#include <vector>

Shader::Shader(const std::string &filePath)
        : m_file_path1(filePath), m_renderer_ID(0) {
    shaderProgramSource source = parseShader(filePath);
//    std::cout << "VERTEX" << std::endl;
//    std::cout << source.vertexShaderSource << std::endl;
//    std::cout << "FRAGMENT" << std::endl;
//    std::cout << source.fragmentShaderSource << std::endl;
    m_renderer_ID = createShader(source.vertexShaderSource, source.fragmentShaderSource);
}

Shader::Shader(const std::string &vertexShader, const std::string &fragmentShader)
        : m_file_path1(vertexShader), m_file_path2(fragmentShader), m_renderer_ID(0) {
    shaderProgramSource source = parseShader(vertexShader, fragmentShader);
//    std::cout << "VERTEX" << std::endl;
//    std::cout << source.vertexShaderSource << std::endl;
//    std::cout << "FRAGMENT" << std::endl;
//    std::cout << source.fragmentShaderSource << std::endl;
    m_renderer_ID = createShader(source.vertexShaderSource, source.fragmentShaderSource);
}

Shader::~Shader() {
    glDeleteProgram(m_renderer_ID);
}

void Shader::bind() const {
    glUseProgram(m_renderer_ID);
}

void Shader::unbind() const {
    glUseProgram(0);
}

void Shader::setUniform1i(const std::string &name, int value) {
    glUniform1i(getUniformLocation(name), value);
}

void Shader::setUniform1f(const std::string &name, float value) {
    glUniform1f(getUniformLocation(name), value);
}

void Shader::setUniform3f(const std::string &name, float f0, float f1, float f2) {
    glUniform3f(getUniformLocation(name), f0, f1, f2);
}

void Shader::setUniform4f(const std::string &name, float f0, float f1, float f2, float f3) {
    glUniform4f(getUniformLocation(name), f0, f1, f2, f3);
}

void Shader::setUniformMatrix4fv(const std::string &name, unsigned int count, bool transpose, glm::mat4 value) {
    glUniformMatrix4fv(getUniformLocation(name), count, transpose, glm::value_ptr(value));
}

int Shader::getUniformLocation(const std::string &name) {
    if (m_uniform_location_cache.find(name) != m_uniform_location_cache.end()) {
        return m_uniform_location_cache[name];
    }
    int location = glGetUniformLocation(m_renderer_ID, name.c_str());
    if (location == -1) {
        std::cout << "Warning: Uniform " << name << " doesn't exist!" << std::endl;
    }

    m_uniform_location_cache[name] = location;
    return location;
}

shaderProgramSource Shader::parseShader(const std::string &filePath) {
    std::ifstream stream(filePath);

    enum class shaderType {
        NONE = -1, VERTEX = 0, FRAGMENT = 1
    };

    std::string line;
    std::stringstream ss[2];
    shaderType type = shaderType::NONE;
    while (getline(stream, line)) {
        if (line.find("#shader") != std::string::npos) {
            if (line.find("vertex") != std::string::npos) {
                type = shaderType::VERTEX;
            } else if (line.find("fragment") != std::string::npos) {
                type = shaderType::FRAGMENT;
            }
        } else {
            ss[(int) type] << line << '\n';
        }
    }

    return {ss[0].str(), ss[1].str()};
}

shaderProgramSource Shader::parseShader(const std::string &vertexShader, const std::string &fragmentShader) {
    std::ifstream stream1(vertexShader), stream2(fragmentShader);

    std::string line;
    std::stringstream ss[2];

    while (getline(stream1, line)) {
        ss[(int) 0] << line << '\n';
    }

    while (getline(stream2, line)) {
        ss[(int) 1] << line << '\n';
    }

    return {ss[0].str(), ss[1].str()};
}

unsigned int Shader::compileShader(unsigned int type, const std::string &source) {
    unsigned int id = glCreateShader(type);
    const char *src = source.c_str();
    glShaderSource(id, 1, &src, nullptr);
    glCompileShader(id);

    // Error handling.
    int result;
    glGetShaderiv(id, GL_COMPILE_STATUS, &result);
    if (result == GL_FALSE) {
        int length;
        glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);
        char *message = (char *) alloca(length * sizeof(char));
        glGetShaderInfoLog(id, length, &length, message);
        std::cout << "Failed to compile " << (type == GL_VERTEX_SHADER ? "vertex" : "fragment") << " shader!"
                  << std::endl;
        std::cout << message << std::endl;
        glDeleteShader(id);
        return 0;
    }

    return id;
}

unsigned int Shader::createShader(const std::string &vertexShader, const std::string &fragmentShader) {
    unsigned int program = glCreateProgram();
    unsigned int vs = compileShader(GL_VERTEX_SHADER, vertexShader);
    unsigned int fs = compileShader(GL_FRAGMENT_SHADER, fragmentShader);

    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);
    glValidateProgram(program);

    int result;
    glGetProgramiv(program, GL_LINK_STATUS, &result);
    if (result == GL_FALSE) {
        int length;
        glGetProgramiv(program, GL_LINK_STATUS, &length);
        char *message = (char *) alloca(length * sizeof(char));
        glGetProgramInfoLog(program, length, &length, message);
        std::cerr << "ERROR::PROGRAM::LINKING_FAILED\n" << message << std::endl;
        return 0;
    }

    glDeleteShader(vs);
    glDeleteShader(fs);

    return program;
}
