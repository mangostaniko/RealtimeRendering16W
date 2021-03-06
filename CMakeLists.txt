project(SuzanneIsland)
cmake_minimum_required(VERSION 3.6)

# add cmake module dir to the cmake module path
set(CMAKE_MODULE_PATH 
    "${CMAKE_MODULE_PATH}"
    "${CMAKE_SOURCE_DIR}"
    "${CMAKE_SOURCE_DIR}/cmake"
)

# specify runtime dir to be the build dir for out-of-source build
# (aka shadow build)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

### COMPILER FLAGS ###
# note: add_definitions can be used as well, but is intended for preprocessor definitions

# Compiler flags
IF (CMAKE_COMPILER_IS_GNUCC)
    # setup debugging flags
    SET(CMAKE_CXX_FLAGS "-g")
    # Setup warning flags
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")
    # Specify language's standard version
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
ENDIF ()


### EXTERNAL LIBRARIES ###

find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(GLFW REQUIRED)
find_package(GLM REQUIRED)
find_package(FreeImagePlus REQUIRED)
find_package(Assimp REQUIRED)
find_package(Freetype REQUIRED)


### INCLUDE HEADER FILES ###
include_directories(
    ${OPENGL_INCLUDE_DIRS}
    ${GLEW_INCLUDE_DIRS}
    ${GLFW_INCLUDE_DIRS}
    ${FREEIMAGEPLUS_INCLUDE_DIRS}
    ${ASSIMP_INCLUDE_DIRS}
    ${FREETYPE_INCLUDE_DIRS}
)

# CPack configuration
SET(CPACK_PACKAGE_NAME "SuzanneIsland")
## Setting contact and vendor details
SET(CPACK_PACKAGE_VENDOR "Team SuzanneIsland")
SET(CPACK_PACKAGE_CONTACT ${CPACK_PACKAGE_VENDOR})

SET(CPACK_RESOURCE_FILE_LICENSE ${PROJECT_SOURCE_DIR}/LICENSE)
SET(CPACK_RESOURCE_FILE_README ${PROJECT_SOURCE_DIR}/README)

ADD_SUBDIRECTORY(docs)
ADD_SUBDIRECTORY(src)

