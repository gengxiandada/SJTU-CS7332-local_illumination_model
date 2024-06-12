#define TINYOBJLOADER_IMPLEMENTATION

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <tiny_obj_loader.h>
#include <vector>
#include <iostream>
#include "Shader.h"
#include "Lights.h"
#include <sstream>
#include <string>
#include "VertexArray.h"
#include "VertexBuffer.h"
#include "VertexBufferLayout.h"
#include "Texture.h"
#include "FrameBuffer.h"
#include "src/utils.cpp"

// Camera settings
glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
float cameraSpeed = 0.05f;

int main() {
    // Initialize GLFW.
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    // Set OpenGL version to 3.3.
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // For MacOS
#endif

    // Create GLFW window.
    GLFWwindow *window = glfwCreateWindow(800, 600, "Load OBJ", nullptr, nullptr);
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

    // Enable depth test and blending.
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Load light settings.
    Lights lights;
    lights.loadLights("res/lightsPos.pos");
    unsigned lightNum = lights.getLightNum();

    // Dynamically generate Shaders based on the number of light sources.
    if (!genShaderSrc("res/shaders/fragment.glsl", lightNum)) {
        std::cout << "Shader generation failed!" << std::endl;
        return -1;
    }

    // Compile and link shaders.
    Shader shaderProgram("res/shaders/vertex.glsl", "res/shaders/fragment.glsl");
    Shader depthShaderProgram("res/shaders/depth_vertex.glsl", "res/shaders/depth_fragment.glsl");

    // Clear the shader cache.
    clean("res/shaders/fragment.glsl", "res/shaders/fragment.glsl.cache");

    // Load multiple OBJ files.
    std::vector<GLuint> VAOs;
    std::vector<GLuint> VBOs;
    std::vector<GLuint> NBOs;
    std::vector<size_t> vertexCounts;

    std::vector<std::string> objFiles = {
            "res/objects/object1-object5-酒杯.obj",
            "res/objects/object2-小凳子.obj",
            "res/objects/object3-台灯.obj"
    };

    for (const auto &objFile: objFiles) {
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
            // positions                        // normals
            40.0f, 0.0f, 40.0f, 0.0f, 1.0f, 0.0f,
            -40.0f, 0.0f, 40.0f, 0.0f, 1.0f, 0.0f,
            -40.0f, 0.0f, -40.0f, 0.0f, 1.0f, 0.0f,

            40.0f, 0.0f, 40.0f, 0.0f, 1.0f, 0.0f,
            -40.0f, 0.0f, -40.0f, 0.0f, 1.0f, 0.0f,
            40.0f, 0.0f, -40.0f, 0.0f, 1.0f, 0.0f
    };

//    unsigned int planeVertexIndices[] = {
//            0, 1, 2,
//            2, 3, 0
//    };

    // Create VBO and VAO for the plane.
    VertexArray planeVA(1);
    VertexBuffer planeVB(planeVertices, sizeof(planeVertices));

    VertexBufferLayout layout;
    layout.push<float>(3); // Position coordinates layout.
    layout.push<float>(3); // Normals coordinates layout.
    planeVA.addBuffer(0, planeVB, layout);
    planeVA.unbind();

//    // Indices buffer object.
//    IndexBuffer ib(planeVertexIndices, sizeof(planeVertexIndices) / sizeof(planeVertexIndices[0]));

    const unsigned int SHADOW_WIDTH = 4096, SHADOW_HEIGHT = 4096; // Increase resolution for finer shadows.

    // Shadow mapping setup.
    Texture depthMap("", lightNum, textureType::Depth, SHADOW_WIDTH, SHADOW_HEIGHT);
    std::vector<FrameBuffer> depthMapFB;
    for (size_t i = 0; i < lightNum; i++) {
        depthMapFB.push_back(FrameBuffer());
        depthMapFB[i].addTexutre(depthMap.getID(i));

        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);
    }

    // Render loop.
    while (!glfwWindowShouldClose(window)) {
        // Process input for keyboard events and camera movement.
        processInput(window, cameraPos, cameraFront, cameraUp, cameraSpeed);

        // 1. Render depth map.
        float near_plane = 1.0f, far_plane = 40.0f;
        // Initialize orthogonal projection matrix.
        glm::mat4 lightProjection = glm::ortho(-40.0f, 40.0f, -40.0f, 40.0f, near_plane, far_plane);
        glm::mat4 lightView; // Light source view matrix.
        std::vector<glm::mat4> lightSpaceMatrix(lightNum); // Light space matrices.

        for (size_t i = 0; i < lightNum; i++) {
            lightView = glm::lookAt(lights.getLightPos(i), glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
            lightSpaceMatrix[i] = lightProjection * lightView;

            depthShaderProgram.bind();
            depthShaderProgram.setUniformMatrix4fv("lightSpaceMatrix", 1, GL_FALSE, lightSpaceMatrix[i]);

            glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
            glClear(GL_DEPTH_BUFFER_BIT);

            // Bind the frame buffer object of the current light source.
            depthMapFB[i].bind();

            // Render scene to depth map.
            planeVA.bind(0);
            glm::mat4 model = glm::mat4(1.0f);
            depthShaderProgram.setUniformMatrix4fv("model", 1, GL_FALSE, model);
            glDrawArrays(GL_TRIANGLES, 0, 6);

            for (size_t j = 0; j < VAOs.size(); ++j) {
                glBindVertexArray(VAOs[j]);
                model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f));
                depthShaderProgram.setUniformMatrix4fv("model", 1, GL_FALSE, model);
                glDrawArrays(GL_TRIANGLES, 0, vertexCounts[j]);
            }

            depthMapFB[i].unbind();
        }

        // Reset viewport.
#ifdef __APPLE__
        glViewport(0, 0, 1600, 1200); // For MacOS
#endif
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // 2. Render scene with shadows.
        glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
        glm::mat4 projection = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);

        shaderProgram.bind();
        shaderProgram.setUniformMatrix4fv("view", 1, GL_FALSE, view);
        shaderProgram.setUniformMatrix4fv("projection", 1, GL_FALSE, projection);

        // Set light and view positions.
        for (size_t i = 0; i < lightNum; ++i) {
            glm::vec3 light = lights.getLightPos(i);
            shaderProgram.setUniformMatrix4fv("lightSpaceMatrix[" + std::to_string(i) + "]", 1, GL_FALSE,
                                              lightSpaceMatrix[i]);
            shaderProgram.setUniform3f("lightPos[" + std::to_string(i) + "]", light.x, light.y, light.z);
            shaderProgram.setUniform3f("lightColor[" + std::to_string(i) + "]", 1.0f, 1.0f, 1.0f);
        }
        shaderProgram.setUniform3f("viewPos", cameraPos.x, cameraPos.y, cameraPos.z);
        shaderProgram.setUniform3f("objectColor", 1.0f, 0.5f, 0.31f);

        // Set material properties
        shaderProgram.setUniform1f("ambientStrength", 0.1f);
        shaderProgram.setUniform1f("specularStrength", 0.5f);
        shaderProgram.setUniform1f("diffuseStrength", 1.0f);
//        shaderProgram.setUniform1f("refractionRatio", 1.0f / 1.33f);

        // Set attenuation parameters.
        shaderProgram.setUniform1f("att_a", 0.0f);
        shaderProgram.setUniform1f("att_b", 0.0f);
        shaderProgram.setUniform1f("att_c", 0.02f);
        shaderProgram.setUniform1i("n", 2);

//        // Bind depth maps.
//        for (size_t i = 0; i < 2; i++) {
//            depthMap.bind(i);
//
//            shaderProgram.setUniform1i("shadowMap[" + std::to_string(i) + "]", i);
//        }
//
//        for (size_t i = 2; i < 3; i++) {
//            depthMap.bind(i);
//
//            shaderProgram.setUniform1i("shadowMap[" + std::to_string(i) + "]", i);
//        }



        // Bind depth maps.
        for (size_t i = 0; i < lightNum; i++) {
            depthMap.bind(i);

            shaderProgram.setUniform1i("opShadowMap[" + std::to_string(i) + "]", i);
        }



        shaderProgram.setUniform1f("alpha", 1.0f); // Set alpha to 1.0 for opaque models.

        // Draw plane.
        planeVA.bind(0);

        glm::mat4 model = glm::mat4(1.0f);
        shaderProgram.setUniformMatrix4fv("model", 1, GL_FALSE, model);
        glDrawArrays(GL_TRIANGLES, 0, 6);

//        shaderProgram.setUniform1f("alpha", 1.0f);

        // Draw models.
        for (size_t i = 0; i < 2; ++i) {
            glBindVertexArray(VAOs[i]);
            model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f));
            shaderProgram.setUniformMatrix4fv("model", 1, GL_FALSE, model);
            glDrawArrays(GL_TRIANGLES, 0, vertexCounts[i]);
        }

        // Draw translucent models (after opaque ones).
        shaderProgram.setUniform1f("alpha", 0.1f); // Set alpha to 0.5 for translucent models
        for (size_t i = 2; i < 3; ++i) {
            glBindVertexArray(VAOs[i]);
            model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f));
            shaderProgram.setUniformMatrix4fv("model", 1, GL_FALSE, model);
            glDrawArrays(GL_TRIANGLES, 0, vertexCounts[i]);
        }

        glBindVertexArray(0);

        // Swap buffers and poll IO events.
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Clean up
    for (GLuint VAO: VAOs) {
        glDeleteVertexArrays(1, &VAO);
    }
    for (GLuint VBO: VBOs) {
        glDeleteBuffers(1, &VBO);
    }
    for (GLuint NBO: NBOs) {
        glDeleteBuffers(1, &NBO);
    }

    glfwTerminate();
    return 0;
}
