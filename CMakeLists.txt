cmake_minimum_required(VERSION 3.28)
project(local_illumination_model)

set(CMAKE_CXX_STANDARD 17)

include_directories(${PROJECT_SOURCE_DIR}/include)

link_directories(${PROJECT_SOURCE_DIR}/lib)

set(SRC_LIST
        src/Shader.cpp
        src/Lights.cpp
        src/VertexBuffer.cpp
        src/VertexBufferLayout.cpp
        src/VertexArray.cpp
        src/Texture.cpp
        src/FrameBuffer.cpp
        src/vendor/stb_image/stb_iamge.cpp
        src/Renderer.cpp
        src/IndexBuffer.cpp
        src/utils.cpp)

add_executable(App
        Application.cpp
        ${SRC_LIST}
        )
add_executable(Demo
        src/main.cpp
        src/Renderer.cpp
        src/VertexBuffer.cpp
        src/IndexBuffer.cpp
        src/VertexArray.cpp
        src/VertexBufferLayout.cpp
        src/Shader.cpp
        src/Texture.cpp
        src/vendor/stb_image/stb_iamge.cpp
)
add_executable(test
        test.cpp)

# dynamic linking
target_link_libraries(App glfw.3 glew.2.2 "-framework Cocoa" "-framework OpenGL" "-framework IOKit")
# static linking
target_link_libraries(Demo glfw3 glew.2.2 "-framework Cocoa" "-framework OpenGL" "-framework IOKit")
target_link_libraries(test glfw3 glew.2.2 "-framework Cocoa" "-framework OpenGL" "-framework IOKit")

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)