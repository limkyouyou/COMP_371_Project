cmake_minimum_required(VERSION 3.10)
project(OpenGLProject)
set(CMAKE_CXX_STANDARD 17)

# Find OpenGL globally
find_package(OpenGL REQUIRED)

file(GLOB_RECURSE SOURCES "src/*.cpp")
add_executable(OpenGLProject ${SOURCES})

# Shared headers for both platforms
target_include_directories(OpenGLProject PRIVATE
    ${CMAKE_SOURCE_DIR}/external/glm
    ${CMAKE_SOURCE_DIR}/external/stb
    ${CMAKE_SOURCE_DIR}/external/glew/include
    ${CMAKE_SOURCE_DIR}/external/glfw/include
)

# Platform-specific linking
if(WIN32)
    message(STATUS "Using local .lib files for GLEW and GLFW")
    message(STATUS "Glew lib path: C:/DevLib/glew-2.1.0/lib/Release/x64")

    set(GLEW_LIB_DIR "C:/DevLib/glew-2.1.0/lib/Release/x64")
    set(GLFW_LIB_DIR "C:/DevLib/GLFW/lib")

    target_link_libraries(OpenGLProject PRIVATE
        "${GLEW_LIB_DIR}/glew32s.lib"
        "${GLFW_LIB_DIR}/glfw3.lib"
        OpenGL::GL
    ) 

elseif(APPLE)
    find_package(GLEW REQUIRED)
    find_package(glfw3 REQUIRED)
    find_package(glm REQUIRED)

    target_link_libraries(OpenGLProject PRIVATE
        OpenGL::GL
        GLEW::GLEW
        glfw
        glm
    )
endif()
