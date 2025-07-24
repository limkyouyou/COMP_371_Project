cmake_minimum_required(VERSION 3.10)
project(OpenGLProject)
set(CMAKE_CXX_STANDARD 17)

# Find OpenGL globally
find_package(OpenGL REQUIRED)

file(GLOB_RECURSE SOURCES "src/*.cpp")

# Assimp (build from source)
set(ASSIMP_BUILD_ASSIMP_TOOLS OFF)
set(ASSIMP_BUILD_TESTS OFF)
set(ASSIMP_INJECT_DEBUG_POSTFIX OFF)
set(ASSIMP_BUILD_ZLIB ON CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)

add_subdirectory(external/assimp)

# Force console window on Windows
if(WIN32)
    set(CMAKE_WIN32_EXECUTABLE OFF)
    message(STATUS "CMAKE_WIN32_EXECUTABLE is ${CMAKE_WIN32_EXECUTABLE}")
endif()

add_executable(OpenGLProject ${SOURCES})

if (WIN32)
    add_custom_command(TARGET OpenGLProject POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_BINARY_DIR}/external/assimp/bin/Debug/assimp-vc143-mt.dll"
            $<TARGET_FILE_DIR:OpenGLProject>)
endif()

# Shared headers for both platforms
target_include_directories(OpenGLProject PRIVATE
    ${CMAKE_SOURCE_DIR}/external/glm
    ${CMAKE_SOURCE_DIR}/external/stb
    ${CMAKE_SOURCE_DIR}/external/glew/include
    ${CMAKE_SOURCE_DIR}/external/glfw/include
    ${CMAKE_SOURCE_DIR}/external/assimp/include
    ${CMAKE_CURRENT_BINARY_DIR}/external/assimp/include
    ${CMAKE_SOURCE_DIR}/headers
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
        assimp
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
        assimp
    )
endif()
