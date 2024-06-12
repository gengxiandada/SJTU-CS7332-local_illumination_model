//
// Created by 程思浩 on 24-5-31.
//

#ifndef OPENGL_TEST_SHADER_H
#define OPENGL_TEST_SHADER_H


#include "GL/glew.h"
#include <string>
#include <unordered_map>
#include "glm/matrix.hpp"

struct shaderProgramSource {
    std::string vertexShaderSource;
    std::string fragmentShaderSource;
};

class Shader {
private:
    std::string m_file_path1, m_file_path2;
    unsigned int m_renderer_ID;
    std::unordered_map<std::string, int> m_uniform_location_cache; // Caching for uniforms.
public:
    Shader(const std::string &filePath);

    Shader(const std::string &filePath1, const std::string &filePath2);

    ~Shader();

    void bind() const;

    void unbind() const;

    // Set uniforms.
    void setUniform1i(const std::string &name, int value);

    void setUniform1f(const std::string &name, float value);

    void setUniform3f(const std::string &name, float f0, float f1, float f2);

    void setUniform4f(const std::string &name, float f0, float f1, float f2, float f3);

    void setUniformMatrix4fv(const std::string &name, unsigned int count, bool transpose, glm::mat4 value);

private:
    int getUniformLocation(const std::string &name);

    shaderProgramSource parseShader(const std::string &filePath);

    shaderProgramSource parseShader(const std::string &vertexShader, const std::string &fragmentShader);

    unsigned int compileShader(unsigned int type, const std::string &source);

    unsigned int createShader(const std::string &vertexShader, const std::string &fragmentShader);
};


#endif //OPENGL_TEST_SHADER_H
