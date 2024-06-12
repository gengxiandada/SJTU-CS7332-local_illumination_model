#include <vector>
#include <iostream>
#include <string>

#include "GL/glew.h"
#include "GLFW/glfw3.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

#include "VertexArray.h"
#include "VertexBuffer.h"
#include "VertexBufferLayout.h"
#include "Texture.h"
#include "FrameBuffer.h"
#include "IndexBuffer.h"
#include "Shader.h"
#include "Lights.h"

#define OP_OBJ_NUM 6 // The number of opaque objects.
#define TRANS_OBJ_NUM 3 // Number of translucent objects.
#define ALPHA 0.3 // The transparency of a translucent object.

#define A 0.0f
#define B 0.0f
#define C 0.02f
#define N 2

#define AMBIENT_STRENGTH 0.1f
#define SPECULAR_STRENGTH 0.5f
#define DIFFUSE_STRENGTH 0.8f

#define OBJECT_COLOR 1.0f, 0.5f, 0.31f
#define LIGHT_COLOR 1.0f, 1.0f, 1.0f

// Program window size.
#define WIDTH 1280
#define HEIGHT 720

// Camera settings
glm::vec3 cameraPos = glm::vec3(0.0f, 6.0f, 15.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
float cameraSpeed = 0.05f;

std::unordered_map<unsigned int, glm::vec3> axisOffs; // The offsets of the object in the scene.

bool genShaderSrc(const std::string &filePath, unsigned int num);
void processInput(GLFWwindow *window, glm::vec3 &cameraPos, glm::vec3 &cameraFront, glm::vec3 &cameraUp,
                  float &cameraSpeed);
bool loadOBJ(const char *path, std::vector<glm::vec3> &vertices, std::vector<glm::vec3> &normals, const glm::vec3 &offset);
void getAxisOff(const std::string filePath, std::unordered_map<unsigned int, glm::vec3> &axisOffs);

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
    GLFWwindow *window = glfwCreateWindow(WIDTH, HEIGHT, "Local Illumination Model", nullptr, nullptr);
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
    lights.loadLights("../res/lightsPos.pos");
    unsigned int lightNum = lights.getLightNum();

    // Dynamically generate Shaders based on the number of light sources.
    if (!genShaderSrc("../res/shaders/fragment.glsl", lightNum)) {
        std::cout << "Shader generation failed!" << std::endl;
        return -1;
    }

    // Compile and link shaders.
    Shader shaderProgram("../res/shaders/vertex.glsl", "../res/shaders/fragment.glsl.cache");
    Shader depthShaderProgram("../res/shaders/depth_vertex.glsl", "../res/shaders/depth_fragment.glsl");

    // Clear the shader cache.
    remove("../res/shaders/fragment.glsl.cache");

    // Load multiple OBJ files.
    std::vector<std::string> objFiles = {
            "../res/objects/object1-酒杯.obj",
            "../res/objects/object2-小凳子.obj",
            "../res/objects/object3-台灯.obj",
            "../res/objects/object4-正方体.obj",
            "../res/objects/object5-圆柱.obj",
            "../res/objects/object6-圆球.obj",
            "../res/objects/object7-锥体.obj",
            "../res/objects/object8-六边形柱体.obj",
            "../res/objects/object9-环.obj"
    };
    getAxisOff("../res/objects/scene.txt", axisOffs);

    // Create VAOs for the objects.
    VertexArray opVA(OP_OBJ_NUM);
    VertexArray transVA(TRANS_OBJ_NUM);
    std::vector<size_t> vertexCounts;

    for (size_t i = 0; i < OP_OBJ_NUM; i++) {
        std::vector<glm::vec3> vertices;
        std::vector<glm::vec3> normals;
        if (!loadOBJ(objFiles[i].c_str(), vertices, normals, axisOffs[i + 1])) {
            std::cerr << "Failed to load OBJ file: " << objFiles[i] << std::endl;
            return -1;
        }

        opVA.bind(i);
        VertexBuffer VB(vertices, vertices.size() * sizeof(glm::vec3));
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GL_FLOAT), (void *) 0);
        glEnableVertexAttribArray(0);

        VertexBuffer NB(normals, normals.size() * sizeof(glm::vec3));
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GL_FLOAT), (void *) 0);
        glEnableVertexAttribArray(1);
        opVA.unbind();

        vertexCounts.push_back(vertices.size());
    }

    for (size_t i = OP_OBJ_NUM; i < OP_OBJ_NUM + TRANS_OBJ_NUM; i++) {
        std::vector<glm::vec3> vertices;
        std::vector<glm::vec3> normals;
        if (!loadOBJ(objFiles[i].c_str(), vertices, normals, axisOffs[i + 1])) {
            std::cerr << "Failed to load OBJ file: " << objFiles[i] << std::endl;
            return -1;
        }

        transVA.bind(i - OP_OBJ_NUM);
        VertexBuffer VB(vertices, vertices.size() * sizeof(glm::vec3));
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GL_FLOAT), (void *) 0);
        glEnableVertexAttribArray(0);

        VertexBuffer NB(normals, normals.size() * sizeof(glm::vec3));
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GL_FLOAT), (void *) 0);
        glEnableVertexAttribArray(1);
        transVA.unbind();

        vertexCounts.push_back(vertices.size());
    }

    // Define vertices for the plane
    float planeVertices[] = {
            // positions                        // normals
            100.0f, 0.0f, 100.0f, 0.0f, 1.0f, 0.0f,
            -100.0f, 0.0f, 100.0f, 0.0f, 1.0f, 0.0f,
            -100.0f, 0.0f, -100.0f, 0.0f, 1.0f, 0.0f,
            100.0f, 0.0f, -100.0f, 0.0f, 1.0f, 0.0f
    };

    unsigned int planeVertexIndices[] = {
            0, 1, 2,
            2, 3, 0
    };

    // Create VBO and VAO for the plane.
    VertexArray planeVA(1);
    VertexBuffer planeVB(planeVertices, sizeof(planeVertices));

    VertexBufferLayout layout;
    layout.push<float>(3); // Position coordinates layout.
    layout.push<float>(3); // Normals coordinates layout.
    planeVA.addBuffer(0, planeVB, layout);
    planeVA.unbind();

    // Indices buffer object.
    IndexBuffer ib(planeVertexIndices, sizeof(planeVertexIndices) / sizeof(planeVertexIndices[0]));

    const unsigned int SHADOW_WIDTH = 4096, SHADOW_HEIGHT = 4096; // Increase resolution for finer shadows.

    // Shadow mapping setup.
    // Depth mapping for opaque objects.
    Texture opDepthMap("", lightNum, textureType::Depth, SHADOW_WIDTH, SHADOW_HEIGHT);
    std::vector<FrameBuffer> opDepthMapFB;
    // Depth mapping for translucent objects.
    Texture transDepthMap("", lightNum, textureType::Depth, SHADOW_WIDTH, SHADOW_HEIGHT);
    std::vector<FrameBuffer> transDepthMapFB;
    for (size_t i = 0; i < lightNum; i++) {
        opDepthMapFB.push_back(FrameBuffer());
        opDepthMapFB[i].addTexutre(opDepthMap.getID(i));
        transDepthMapFB.push_back(FrameBuffer());
        transDepthMapFB[i].addTexutre(transDepthMap.getID(i));

        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);
    }

    // For performance measurement.
    double lastTime = glfwGetTime();
    double currentTime;
    int nbFrames = 0;

    // Render loop.
    while (!glfwWindowShouldClose(window)) {
        // Process input for keyboard events and camera movement.
        processInput(window, cameraPos, cameraFront, cameraUp, cameraSpeed);

        // 1. Render depth map.
        glm::mat4 lightProjection = glm::perspective(glm::radians(90.0f), (GLfloat)WIDTH / HEIGHT, 5.0f, 100.0f);
        glm::mat4 lightView; // Light source view matrix.
        std::vector<glm::mat4> lightSpaceMatrix(lightNum); // Light space matrices.

        for (size_t i = 0; i < lightNum; i++) {
            lightView = glm::lookAt(lights.getLightPos(i), glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
            lightSpaceMatrix[i] = lightProjection * lightView;

            depthShaderProgram.bind();
            depthShaderProgram.setUniformMatrix4fv("lightSpaceMatrix", 1, GL_FALSE, lightSpaceMatrix[i]);
            depthShaderProgram.setUniformMatrix4fv("model", 1, GL_FALSE, glm::mat4(1.0f));

            glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
            glClear(GL_DEPTH_BUFFER_BIT);

            // Render scene to opaque objects' depth map.
            opDepthMapFB[i].bind();
            planeVA.bind(0);
            glDrawArrays(GL_TRIANGLES, 0, 6);

            for (size_t j = 0; j < OP_OBJ_NUM; ++j) {
                opVA.bind(j);
                glDrawArrays(GL_TRIANGLES, 0, vertexCounts[j]);
            }
            opDepthMapFB[i].unbind();


            // Render scene to translucent objects' depth map.
            transDepthMapFB[i].bind();
            for (size_t j = OP_OBJ_NUM; j < OP_OBJ_NUM + TRANS_OBJ_NUM; ++j) {
                transVA.bind(j - OP_OBJ_NUM);
                glDrawArrays(GL_TRIANGLES, 0, vertexCounts[j]);
            }
            transDepthMapFB[i].unbind();
        }

        depthShaderProgram.unbind();

        // Reset viewport. Optimized for Retina screens.
#ifdef __APPLE__
        glViewport(0, 0, 2 * WIDTH, 2 * HEIGHT); // For MacOS.
#else
        glViewport(0, 0, WIDTH, HEIGHT);
#endif
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // 2. Render scene with shadows.
        glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
        glm::mat4 projection = glm::perspective(glm::radians(45.0f), (GLfloat)WIDTH / HEIGHT, 0.1f, 200.0f);

        shaderProgram.bind();
        shaderProgram.setUniformMatrix4fv("view", 1, GL_FALSE, view);
        shaderProgram.setUniformMatrix4fv("projection", 1, GL_FALSE, projection);

        // Set light and view positions.
        for (size_t i = 0; i < lightNum; ++i) {
            glm::vec3 light = lights.getLightPos(i);
            shaderProgram.setUniformMatrix4fv("lightSpaceMatrix[" + std::to_string(i) + "]", 1, GL_FALSE,
                                              lightSpaceMatrix[i]);
            shaderProgram.setUniform3f("lightPos[" + std::to_string(i) + "]", light.x, light.y, light.z);
            shaderProgram.setUniform3f("lightColor[" + std::to_string(i) + "]", LIGHT_COLOR);
        }
        shaderProgram.setUniform3f("viewPos", cameraPos.x, cameraPos.y, cameraPos.z);
        shaderProgram.setUniform3f("objectColor", OBJECT_COLOR);

        // Set material properties
        shaderProgram.setUniform1f("ambientStrength", AMBIENT_STRENGTH);
        shaderProgram.setUniform1f("specularStrength", SPECULAR_STRENGTH);
        shaderProgram.setUniform1f("diffuseStrength", DIFFUSE_STRENGTH);
        shaderProgram.setUniform1f("alpha", ALPHA);
//        shaderProgram.setUniform1f("refractionRatio", 1.0f / 1.33f);

        // Set attenuation parameters.
        shaderProgram.setUniform1f("att_a", A);
        shaderProgram.setUniform1f("att_b", B);
        shaderProgram.setUniform1f("att_c", C);
        shaderProgram.setUniform1i("n", N);

        // 3. Bind depth maps.
        size_t slot = 0;
        for (size_t i = 0; i < lightNum; i++) {
            glActiveTexture(GL_TEXTURE0 + slot);
            glBindTexture(GL_TEXTURE_2D, opDepthMap.getID(i));
            shaderProgram.setUniform1i("opShadowMap[" + std::to_string(i) + "]", slot++);

            glActiveTexture(GL_TEXTURE0 + slot);
            glBindTexture(GL_TEXTURE_2D, transDepthMap.getID(i));
            shaderProgram.setUniform1i("transShadowMap[" + std::to_string(i) + "]", slot++);
        }

        // 4. Draw plane.
        shaderProgram.setUniformMatrix4fv("model", 1, GL_FALSE, glm::mat4(1.0f));
        shaderProgram.setUniform1f("flag", false);
        Renderer renderer;
        renderer.draw(planeVA, ib, shaderProgram);

        // 5. Draw opaque models.
        for (size_t i = 0; i < OP_OBJ_NUM; ++i) {
            opVA.bind(i);
            glDrawArrays(GL_TRIANGLES, 0, vertexCounts[i]);
        }

        // 6. Draw translucent models (after opaque ones).
        shaderProgram.setUniform1f("flag", true);
        for (size_t i = OP_OBJ_NUM; i < OP_OBJ_NUM + TRANS_OBJ_NUM; ++i) {
            transVA.bind(i - OP_OBJ_NUM);
            glDrawArrays(GL_TRIANGLES, 0, vertexCounts[i]);
        }

        shaderProgram.unbind();

        // Swap buffers and poll IO events.
        glfwSwapBuffers(window);
        glfwPollEvents();

        // Performance measurement.
        nbFrames++;
        currentTime = glfwGetTime();
        if (currentTime - lastTime >= 1.0) {
            printf("%lf ms/frame; %.1lf frames/sec\n", 1000.0 * (currentTime - lastTime) / double(nbFrames), \
                    double(nbFrames) / (currentTime - lastTime));
            nbFrames = 0;
            lastTime = glfwGetTime();
        }
    }

    glfwTerminate();
    return 0;
}
