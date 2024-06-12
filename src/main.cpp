//
// Created by 程思浩 on 2024/5/30.
//

#include "GL/glew.h"
#include "GLFW/glfw3.h"
#include "Renderer.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "VertexArray.h"
#include <iostream>
#include "Shader.h"
#include "VertexBufferLayout.h"
#include "Texture.h"

#define WINDOW_WIDTH 640
#define WINDOW_HEIGHT 480

const char *window_title = "hello world";

int main() {
    GLFWwindow *window;

    /* Initialize the library */
    if (!glfwInit())
        return -1;

//    // Set OpenGL version to 3.3
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // For MacOS
#endif

    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, window_title, NULL, NULL);
    if (!window) {
        glfwTerminate();
        return -1;
    }

    /* Make the window's context current */
    glfwMakeContextCurrent(window);

//    glfwSwapInterval(1);

    if (glewInit() != GLEW_OK) {
        std::cout << "Error!" << std::endl;
    }

    std::cout << glGetString(GL_VERSION) << std::endl;

    float positions[] = {
            -0.5f, -0.5f, 0.0f, 0.0f,
            0.5f, -0.5f, 1.0f, 0.0f,

            0.5f, 0.5f, 1.0f, 1.0f,
            -0.5f, 0.5f, 0.0f, 1.0f
    };

    unsigned int indices[] = {
            0, 1, 2,
            2, 3, 0
    };

//    glEnable(GL_BLEND);
//    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Vertex array object.
    VertexArray va(1);

    // Vertex buffer object.
    VertexBuffer vb(positions, sizeof(positions));

    VertexBufferLayout layout;
    layout.push<float>(2); // Position coordinates layout.
    layout.push<float>(2); // Texture coordinates layout.
    va.addBuffer(0, vb, layout);

    // Indices buffer object.
    IndexBuffer ib(indices, sizeof(indices) / sizeof(indices[0]));

    Shader shader("res/shaders/basic.shader");
    shader.bind();
    shader.setUniform4f("u_color", 0.8f, 0.3f, 0.8f, 1.0f);

    Texture texture("res/textures/原木木板.png", 1);
    texture.bind();
    shader.setUniform1i("u_texture", 0);

    va.unbind();
    vb.unbind();
    ib.unbind();
    shader.unbind();

    Renderer renderer;

    float r = 0.0f;
    float increment = 0.05f;
    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window)) {
        // Input handling
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);

        /* Render here */
        renderer.clear();

        shader.setUniform4f("u_color", r, 0.3f, 0.8f, 1.0f);

        renderer.draw(va, ib, shader);

        if (r > 1.0f) {
            increment = -0.05f;
        } else if (r < 0.0f) {
            increment = 0.05f;
        }
        r += increment;

        /* Swap front and back buffers */
        glfwSwapBuffers(window);

        /* Poll for and process events */
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}