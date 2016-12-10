FILE(
    GLOB_RECURSE
    source_files
    *.cpp
)

FILE(
    GLOB_RECURSE
    header_files
    *.cpp
)

# adds a library target with given name to be built from the source files
add_library(glfwpp SHARED ${source_files} ${header_files})

# specifies libraries to link with during the build process for a given
# executable target
target_link_libraries(
    glfwpp
    ${GLFW_LIBRARIES}
)
