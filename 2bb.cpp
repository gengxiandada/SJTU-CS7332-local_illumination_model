#define TINYOBJLOADER_IMPLEMENTATION

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <tiny_obj_loader.h>
#include <vector>
#include <iostream>

// Shader source code
const char* vertexShaderSource = R"(
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;

out vec3 FragPos;
out vec3 Normal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
    FragPos = vec3(model * vec4(aPos, 1.0));
    Normal = mat3(transpose(inverse(model))) * aNormal;

    gl_Position = projection * view * vec4(FragPos, 1.0);
}
)";

const char* fragmentShaderSource = R"(
#version 330 core
out vec4 FragColor;

in vec3 FragPos; // 片元位置
in vec3 Normal; // 片元法向量

uniform vec3 lightPos[2]; // 两个光源的位置
uniform vec3 lightColor[2]; // 两个光源的颜色
uniform vec3 viewPos; // 观察者位置，即摄像机位置
uniform vec3 objectColor; // 物体的颜色

// 材质属性
uniform float ambientStrength; // Ambient light coefficient.
uniform float specularStrength; // Specular light coefficient.
uniform float diffuseStrength; // Diffuse coefficient.

// 光照衰减参数
uniform float att_a; // 衰减参数 a
uniform float att_b; // 衰减参数 b
uniform float att_c; // 衰减参数 c

uniform int n; // 幂次

uniform int lightNum; // Number of rays.

// 阴影相关
uniform sampler2D shadowMap; // 阴影贴图
uniform mat4 lightSpaceMatrix; // 光照空间变换矩阵

float ShadowCalculation(vec3 fragPos) {
    vec4 fragPosLightSpace = lightSpaceMatrix * vec4(fragPos, 1.0);
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;

    if(projCoords.z > 1.0) {
        return 0.0;
    }

    float shadow = 0.0;
    float bias = max(0.05 * (1.0 - dot(Normal, lightPos[0] - FragPos)), 0.005);
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
    for(int x = -1; x <= 1; ++x) {
        for(int y = -1; y <= 1; ++y) {
            float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
            shadow += (projCoords.z - bias > pcfDepth) ? 1.0 : 0.0;
        }
    }
    shadow /= 9.0;

    return shadow;
}

void main() {
    vec3 norm = normalize(Normal); // 归一化法向量
    vec3 viewDir = normalize(viewPos - FragPos); // 观察方向，即从片元指向观察者的方向

    vec3 totalDiffuse = vec3(0.0); // 总的漫反射光
    vec3 totalSpecular = vec3(0.0); // 总的镜面反射光
    vec3 totalAmbient = vec3(0.0); // 总的环境光

    for(int i = 0; i < lightNum; i++) {
        vec3 lightDir = normalize(lightPos[i] - FragPos); // 光源到片元的方向
        float diff = max(dot(norm, lightDir), 0.0); // 漫反射强度

        vec3 reflectDir = reflect(-lightDir, norm); // 反射方向
        float spec = max(pow(dot(viewDir, reflectDir), 2), 0.0); // 镜面反射强度

        float distance = length(lightPos[i] - FragPos); // 光源到片元的距离
        float attenuation = 1.0 / (att_a + att_b * distance + att_c * pow(distance, 2)); // 衰减因子

        vec3 diffuse = diffuseStrength * lightColor[i] * diff * attenuation; // 漫反射分量
        vec3 specular = specularStrength * lightColor[i] * spec * attenuation; // 镜面反射分量
        vec3 ambient = ambientStrength * lightColor[i]; // 环境光分量

        totalDiffuse += diffuse; // 累加漫反射光
        totalSpecular += specular; // 累加镜面反射光
        totalAmbient += ambient; // 累加环境光
    }

    float shadow = ShadowCalculation(FragPos); // 计算阴影
    vec3 result = (0.5 + (1.0 - shadow) * (totalDiffuse + totalSpecular)) * objectColor; // 计算最终颜色
    FragColor = vec4(result, 1.0); // 设置片元颜色
}
)";

// Shader source code for depth mapping
const char* depthVertexShaderSource = R"(
#version 330 core
layout (location = 0) in vec3 aPos;

uniform mat4 lightSpaceMatrix;
uniform mat4 model;

void main() {
    gl_Position = lightSpaceMatrix * model * vec4(aPos, 1.0);
}
)";

const char* depthFragmentShaderSource = R"(
#version 330 core
void main() {
    // Do nothing
}
)";

// Function to load the OBJ file
bool loadOBJ(const char* path, std::vector<glm::vec3>& vertices, std::vector<glm::vec3>& normals) {
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;

    if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, path)) {
        std::cerr << warn << err << std::endl;
        return false;
    }

    for (const auto& shape : shapes) {
        for (const auto& index : shape.mesh.indices) {
            glm::vec3 vertex = {
                    attrib.vertices[3 * index.vertex_index + 0],
                    attrib.vertices[3 * index.vertex_index + 1],
                    attrib.vertices[3 * index.vertex_index + 2]
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

// Function to compile shader
GLuint compileShader(GLenum type, const char* source) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);

    int success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, nullptr, infoLog);
        std::cerr << "ERROR::SHADER::COMPILATION_FAILED\n" << infoLog << std::endl;
        return 0;
    }
    return shader;
}

// Function to create shader program
GLuint createShaderProgram(const char* vertexSource, const char* fragmentSource) {
    GLuint vertexShader = compileShader(GL_VERTEX_SHADER, vertexSource);
    GLuint fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentSource);

    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    int success;
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(shaderProgram, 512, nullptr, infoLog);
        std::cerr << "ERROR::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
        return 0;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return shaderProgram;
}

// Camera settings
glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
float cameraSpeed = 0.05f;

// Process input for camera movement
void processInput(GLFWwindow *window) {
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
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
        glm::mat4 rotationMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(1.0f), glm::normalize(glm::cross(cameraFront, cameraUp)));
        cameraFront = glm::vec3(rotationMatrix * glm::vec4(cameraFront, 1.0f));
    }
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
        glm::mat4 rotationMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(-1.0f), glm::normalize(glm::cross(cameraFront, cameraUp)));
        cameraFront = glm::vec3(rotationMatrix * glm::vec4(cameraFront, 1.0f));
    }
}

// Function to create VAO and VBO for a model
void createModelVAO(const std::vector<glm::vec3>& vertices, const std::vector<glm::vec3>& normals, GLuint& VAO, GLuint& VBO, GLuint& NBO) {
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &NBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), vertices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, NBO);
    glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(glm::vec3), normals.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
}

// Main function
int main() {
    // Initialize GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    // Set OpenGL version to 3.3
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // For MacOS
#endif

    // Create GLFW window
    GLFWwindow* window = glfwCreateWindow(800, 600, "Load OBJ", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // Initialize GLEW.
    if (glewInit() != GLEW_OK) {
        std::cout << "Error!" << std::endl;
    }

    std::cout << glGetString(GL_VERSION) << std::endl;

    // Enable depth test
    glEnable(GL_DEPTH_TEST);

    // Compile and link shaders
    GLuint shaderProgram = createShaderProgram(vertexShaderSource, fragmentShaderSource);
    if (shaderProgram == 0) {
        return -1;
    }

    GLuint depthShaderProgram = createShaderProgram(depthVertexShaderSource, depthFragmentShaderSource);
    if (depthShaderProgram == 0) {
        return -1;
    }

    // Load multiple OBJ files
    std::vector<GLuint> VAOs;
    std::vector<GLuint> VBOs;
    std::vector<GLuint> NBOs;
    std::vector<size_t> vertexCounts;

    std::vector<std::string> objFiles = {
            "res/objects/object1-object5-酒杯.obj",
            "res/objects/object2-小凳子.obj",
            "res/objects/object3-台灯.obj"
    };

    for (const auto& objFile : objFiles) {
        std::vector<glm::vec3> vertices;
        std::vector<glm::vec3> normals;
        if (!loadOBJ(objFile.c_str(), vertices, normals)) {
            std::cerr << "Failed to load OBJ file: " << objFile << std::endl;
            return -1;
        }

        GLuint VAO, VBO, NBO;
        createModelVAO(vertices, normals, VAO, VBO, NBO);

        VAOs.push_back(VAO);
        VBOs.push_back(VBO);
        NBOs.push_back(NBO);
        vertexCounts.push_back(vertices.size());
    }

    // Define vertices for the plane
    float planeVertices[] = {
            // positions          // normals
            20.0f, 0.0f,  20.0f,  0.0f, 1.0f, 0.0f,
            -20.0f, 0.0f,  20.0f,  0.0f, 1.0f, 0.0f,
            -20.0f, 0.0f, -20.0f,  0.0f, 1.0f, 0.0f,

            20.0f, 0.0f,  20.0f,  0.0f, 1.0f, 0.0f,
            -20.0f, 0.0f, -20.0f,  0.0f, 1.0f, 0.0f,
            20.0f, 0.0f, -20.0f,  0.0f, 1.0f, 0.0f
    };

    // Create VBO and VAO for the plane
    GLuint planeVAO, planeVBO;
    glGenVertexArrays(1, &planeVAO);
    glGenBuffers(1, &planeVBO);

    glBindVertexArray(planeVAO);

    glBindBuffer(GL_ARRAY_BUFFER, planeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(planeVertices), planeVertices, GL_STATIC_DRAW);

    // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Normal attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);

    // Shadow mapping setup
    GLuint depthMapFBO;
    glGenFramebuffers(1, &depthMapFBO);

    const unsigned int SHADOW_WIDTH = 4096, SHADOW_HEIGHT = 4096; // Increase resolution for finer shadows
    GLuint depthMap;
    glGenTextures(1, &depthMap);
    glBindTexture(GL_TEXTURE_2D, depthMap);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Light settings
    glm::vec3 lightPos[2] = { glm::vec3(-10.2f, 5.5f, -10.2f), glm::vec3(2.2f, 6.5f, 2.2f) };

    // Render loop
    while (!glfwWindowShouldClose(window)) {
        processInput(window);

        // 1. Render depth map
        glm::mat4 lightProjection, lightView;
        glm::mat4 lightSpaceMatrix;
        float near_plane = 1.0f, far_plane = 30.0f; // Increase far plane to ensure the light covers the entire scene
        lightProjection = glm::ortho(-30.0f, 30.0f, -30.0f, 30.0f, near_plane, far_plane);
        lightView = glm::lookAt(lightPos[0], glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        lightSpaceMatrix = lightProjection * lightView;

        glUseProgram(depthShaderProgram);
        glUniformMatrix4fv(glGetUniformLocation(depthShaderProgram, "lightSpaceMatrix"), 1, GL_FALSE, glm::value_ptr(lightSpaceMatrix));

        glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
        glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
        glClear(GL_DEPTH_BUFFER_BIT);
        glActiveTexture(GL_TEXTURE0);

        // Render scene to depth map
        glBindVertexArray(planeVAO);
        glm::mat4 model = glm::mat4(1.0f);
        glUniformMatrix4fv(glGetUniformLocation(depthShaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
        glDrawArrays(GL_TRIANGLES, 0, 6);

        for (size_t i = 0; i < VAOs.size(); ++i) {
            glBindVertexArray(VAOs[i]);
            model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f));
            glUniformMatrix4fv(glGetUniformLocation(depthShaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
            glDrawArrays(GL_TRIANGLES, 0, vertexCounts[i]);
        }
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // Reset viewport
        glViewport(0, 0, 1600, 1200);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // 2. Render scene with shadows
        glUseProgram(shaderProgram);

        glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
        glm::mat4 projection = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);

        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "lightSpaceMatrix"), 1, GL_FALSE, glm::value_ptr(lightSpaceMatrix));

        // Set light and view positions
        for (int i = 0; i < 1; ++i) {
            glUniform3f(glGetUniformLocation(shaderProgram, ("lightPos[" + std::to_string(i) + "]").c_str()), lightPos[i].x, lightPos[i].y, lightPos[i].z);
            glUniform3f(glGetUniformLocation(shaderProgram, ("lightColor[" + std::to_string(i) + "]").c_str()), 1.0f, 1.0f, 1.0f);
        }
        glUniform3f(glGetUniformLocation(shaderProgram, "viewPos"), cameraPos.x, cameraPos.y, cameraPos.z);
        glUniform3f(glGetUniformLocation(shaderProgram, "objectColor"), 1.0f, 0.5f, 0.31f);

        // Set material properties
        glUniform1f(glGetUniformLocation(shaderProgram, "ambientStrength"), 0.5f);
        glUniform1f(glGetUniformLocation(shaderProgram, "specularStrength"), 0.5f);
        glUniform1f(glGetUniformLocation(shaderProgram, "diffuseStrength"), 1.0f);

        // Set attenuation parameters
        glUniform1f(glGetUniformLocation(shaderProgram, "att_a"), 0.0f);
        glUniform1f(glGetUniformLocation(shaderProgram, "att_b"), 0.0f);
        glUniform1f(glGetUniformLocation(shaderProgram, "att_c"), 0.02f);
        glUniform1i(glGetUniformLocation(shaderProgram, "n"), 2);
        glUniform1i(glGetUniformLocation(shaderProgram, "lightNum"), 2);

        // Bind depth map
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, depthMap);
        glUniform1i(glGetUniformLocation(shaderProgram, "shadowMap"), 1);

        // Draw plane
        glBindVertexArray(planeVAO);
        model = glm::mat4(1.0f);
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
        glDrawArrays(GL_TRIANGLES, 0, 6);

        // Draw models
        for (size_t i = 0; i < VAOs.size(); ++i) {
            glBindVertexArray(VAOs[i]);
            model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f));
            glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
            glDrawArrays(GL_TRIANGLES, 0, vertexCounts[i]);
        }
        glBindVertexArray(0);

        // Swap buffers and poll IO events
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Clean up
    for (GLuint VAO : VAOs) {
        glDeleteVertexArrays(1, &VAO);
    }
    for (GLuint VBO : VBOs) {
        glDeleteBuffers(1, &VBO);
    }
    for (GLuint NBO : NBOs) {
        glDeleteBuffers(1, &NBO);
    }
    glDeleteVertexArrays(1, &planeVAO);
    glDeleteBuffers(1, &planeVBO);

    glfwTerminate();
    return 0;
}
