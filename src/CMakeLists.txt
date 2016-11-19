#FILE(
#	GLOB_RECURSE
#	source_files
#	*.[hc]pp
#)

# relative path to every single h and cpp file
set(SRC_CLASSES 
    main.cpp

    shader.h
    shader.cpp
    texture.hpp
    textrenderer.h
    textrenderer.cpp

    sceneobject.hpp
    camera.h
    camera.cpp
    light.h
    light.cpp
    geometry.h
    geometry.cpp
    surface.h
    surface.cpp

    eagle.h
    eagle.cpp

    effects/ssaopostprocessor.h
    effects/ssaopostprocessor.cpp
    effects/particlesystem.h
    effects/particlesystem.cpp
)

# relative path to shader files
set(SRC_SHADERS
    shaders/textured_blinnphong.vert
    shaders/textured_blinnphong.frag
    shaders/normal_mapping.vert
    shaders/normal_mapping.frag

    shaders/text.vert
    shaders/text.frag
    shaders/particles.vert
    shaders/particles.frag

    shaders/depth_shader.vert
    shaders/depth_shader.frag
    shaders/depth_shader_vsm.vert
    shaders/depth_shader_vsm.frag
    shaders/quad_debug.vert
    shaders/quad_debug.frag

    shaders/ssao.vert
    shaders/ssao.frag
    shaders/blur.vert
    shaders/blur.frag
    shaders/blur_vsm.vert
    shaders/blur_vsm.frag
)

# adds an executable target with given name to be built from the source files
add_executable(${PROJECT_NAME} ${SRC_CLASSES} ${SRC_SHADERS})

# specifies libraries to link with during the build process for a given
# executable target
target_link_libraries(
    ${PROJECT_NAME}
    ${OPENGL_LIBRARIES}
    ${GLEW_LIBRARIES}
    ${GLFW_LIBRARIES}
    ${FreeImage_LIBRARIES}
    ${ASSIMP_LIBRARIES}
    ${FREETYPE_LIBRARIES}
)

target_compile_definitions(${PROJECT_NAME} PRIVATE "GLM_FORCE_RADIANS;GLM_FORCE_SWIZZLE")

IF (MSVC)
    # add options from visual studio project
    target_compile_definitions(${PROJECT_NAME} PRIVATE "GLEW_STATIC")
ENDIF ()

add_custom_target(shaders)

add_custom_command(TARGET shaders PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/shaders
        ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/shaders
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/data
        ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/data
)

add_dependencies(${PROJECT_NAME} shaders)
