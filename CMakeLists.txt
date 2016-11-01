project(SuzanneIsland)
cmake_minimum_required(VERSION 3.6)

# add CMakeLists.txt source dir and cmake module dir to the cmake module path
# cmake modules are cmake files used e.g. to find library files
set(CMAKE_MODULE_PATH 
    ${CMAKE_MODULE_PATH} 
    "${CMAKE_SOURCE_DIR}"
    "${CMAKE_SOURCE_DIR}/CMakeModules"
    ) 

# specify build dir for out-of-source build (aka shadow build)
set(CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR}/build)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})


### COMPILER FLAGS ###
# note: add_definitions can be used as well, but is intended for preprocessor definitions

set(CMAKE_CXX_FLAGS "-g -Wall -std=c++0x")


### EXTERNAL LIBRARIES ###

find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(GLFW REQUIRED)
find_package(GLM REQUIRED)
find_package(FreeImage REQUIRED)
find_package(Assimp REQUIRED)
find_package(Freetype REQUIRED)


### SOURCE FILES ###

# relative path to every single h and cpp file
set(SRC_CLASSES 

    src/main.cpp

    src/shader.h
    src/shader.cpp
    src/texture.h
    src/texture.cpp
    src/textrenderer.h
    src/textrenderer.cpp

    src/sceneobject.h
    src/sceneobject.cpp
    src/camera.h
    src/camera.cpp
    src/light.h
    src/light.cpp
    src/geometry.h
    src/geometry.cpp
    src/surface.h
    src/surface.cpp

    src/eagle.h
    src/eagle.cpp

    src/effects/ssaopostprocessor.h
    src/effects/ssaopostprocessor.cpp
    src/effects/particlesystem.h
    src/effects/particlesystem.cpp

    )
    
# relative path to shader files
set(SRC_SHADERS
        src/shaders/textured_blinnphong.vert
        src/shaders/textured_blinnphong.frag
        src/shaders/normal_mapping.vert
        src/shaders/normal_mapping.frag

        src/shaders/text.vert
        src/shaders/text.frag
        src/shaders/particles.vert
        src/shaders/particles.frag

        src/shaders/depth_shader.vert
        src/shaders/depth_shader.frag
        src/shaders/depth_shader_vsm.vert
        src/shaders/depth_shader_vsm.frag
        src/shaders/quad_debug.vert
        src/shaders/quad_debug.frag

        src/shaders/ssao.vert
        src/shaders/ssao.frag
        src/shaders/blur.vert
        src/shaders/blur.frag
        src/shaders/blur_vsm.vert
        src/shaders/blur_vsm.frag

        )
        
# adds an executable target with given name to be built from the source files listed afterwards
add_executable(${PROJECT_NAME} ${SRC_CLASSES} ${SRC_SHADERS})


### INCLUDE HEADER FILES ###
include_directories(${OPENGL_INCLUDE_DIRS} 
                    ${GLEW_INCLUDE_DIRS} 
                    ${GLFW_INCLUDE_DIRS}
                    ${FreeImage_INCLUDE_DIR}
                    ${ASSIMP_INCLUDE_DIRS}
                    ${FREETYPE_INCLUDE_DIRS}
                    )
                    
### LINK LIBRARIES ###
target_link_libraries(${PROJECT_NAME}
                      ${OPENGL_LIBRARIES}
                      ${GLEW_LIBRARIES}
                      ${GLFW_LIBRARIES}
                      ${FreeImage_LIBRARIES}
                      ${ASSIMP_LIBRARIES}
                      ${FREETYPE_LIBRARIES}
                      )
