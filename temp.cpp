#define TINYOBJLOADER_IMPLEMENTATION

#include <glad/glad.h>
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

in vec3 FragPos;
in vec3 Normal;

uniform vec3 lightPos;
uniform vec3 viewPos;
uniform vec3 lightColor;
uniform vec3 objectColor;

// Material properties
uniform float ambientStrength;
uniform float specularStrength;
uniform int shininess;
uniform float refractionRatio;

void main() {
    // Ambient
    vec3 ambient = ambientStrength * lightColor;

    // Diffuse
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;

    // Specular
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
    vec3 specular = specularStrength * spec * lightColor;

    vec3 result = (ambient + diffuse + specular) * objectColor;
    FragColor = vec4(result, 1.0);
}
)";

// Function to load the OBJ file
bool loadOBJ(const char* path, std::vector<glm::vec3>& vertices, std::vector<glm::vec3>& normals, float& minY) {
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;

    if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, path)) {
        std::cerr << warn << err << std::endl;
        return false;
    }

    minY = std::numeric_limits<float>::max();
    for (const auto& shape : shapes) {
        for (const auto& index : shape.mesh.indices) {
            glm::vec3 vertex = {
                    attrib.vertices[3 * index.vertex_index + 0],
                    attrib.vertices[3 * index.vertex_index + 1],
                    attrib.vertices[3 * index.vertex_index + 2]
            };
            vertices.push_back(vertex);

            if (vertex.y < minY) {
                minY = vertex.y;
            }

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

    // Initialize GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // Enable depth test
    glEnable(GL_DEPTH_TEST);

    // Compile and link shaders
    GLuint shaderProgram = createShaderProgram(vertexShaderSource, fragmentShaderSource);
    if (shaderProgram == 0) {
        return -1;
    }

    // Load OBJ file
    std::vector<glm::vec3> vertices;
    std::vector<glm::vec3> normals;
    float minY = 0.0;
    if (!loadOBJ("res/objects/object1-object5-酒杯.obj", vertices, normals, minY)) {
        std::cerr << "Failed to load OBJ file" << std::endl;
        return -1;
    }

    // Define vertices for the plane
    float planeVertices[] = {
            // positions          // normals
            10.0f, 0.0f,  10.0f,  0.0f, 1.0f, 0.0f,
            -10.0f, 0.0f,  10.0f,  0.0f, 1.0f, 0.0f,
            -10.0f, 0.0f, -10.0f,  0.0f, 1.0f, 0.0f,

            10.0f, 0.0f,  10.0f,  0.0f, 1.0f, 0.0f,
            -10.0f, 0.0f, -10.0f,  0.0f, 1.0f, 0.0f,
            10.0f, 0.0f, -10.0f,  0.0f, 1.0f, 0.0f
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

    // Create VBO and VAO for the model
    GLuint VAO, VBO, NBO;
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

    // Render loop
    while (!glfwWindowShouldClose(window)) {
        processInput(window);

        // Render
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(shaderProgram);

        // Transformation matrices
        glm::mat4 model = glm::translate(
                glm::mat4(1.0f), glm::vec3(0.0f, -minY, 0.0f));  // Adjust model matrix
        glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
        glm::mat4 projection = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);

        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

        // Set light and view positions
        glUniform3f(glGetUniformLocation(shaderProgram, "lightPos"), 2.2f, 5.5f, 2.2f);
        glUniform3f(glGetUniformLocation(shaderProgram, "viewPos"), cameraPos.x, cameraPos.y, cameraPos.z);
        glUniform3f(glGetUniformLocation(shaderProgram, "lightColor"), 1.0f, 1.0f, 1.0f);
        glUniform3f(glGetUniformLocation(shaderProgram, "objectColor"), 1.0f, 0.5f, 0.31f);

        // Set material properties
        glUniform1f(glGetUniformLocation(shaderProgram, "ambientStrength"), 0.1f);
        glUniform1f(glGetUniformLocation(shaderProgram, "specularStrength"), 0.5f);
        glUniform1i(glGetUniformLocation(shaderProgram, "shininess"), 32);
        glUniform1f(glGetUniformLocation(shaderProgram, "refractionRatio"), 1.0f / 1.33f);  // Example for air to water

        // Draw plane
        glBindVertexArray(planeVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        // Draw model
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, vertices.size());
        glBindVertexArray(0);

        // Swap buffers and poll IO events
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Clean up
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &NBO);
    glDeleteVertexArrays(1, &planeVAO);
    glDeleteBuffers(1, &planeVBO);

    glfwTerminate();
    return 0;
}
