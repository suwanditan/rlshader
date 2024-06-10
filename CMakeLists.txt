# tilemapping simple
cmake_minimum_required(VERSION 3.28)

project(glfwBase VERSION 0.1 LANGUAGES C)
add_executable(glfwBase glfwbase.c ./deps/glad_gl.c)

find_package(Threads REQUIRED)
#find_package(raylib REQUIRED)
#link_libraries(raylib)

target_link_libraries(glfwBase PUBLIC    
    #raylib
    glfw3
    #glad
    GL
    GLEW
    m
    z
    #GL
    pthread
    #dl
    rt
    X11
    
)

# Copy the resources
#file(GLOB resources resources/*)
#file(COPY ${resources} DESTINATION "resources/")

