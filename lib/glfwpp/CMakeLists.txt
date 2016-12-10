project(GLFWPPLIB)
cmake_minimum_required(VERSION 3.6)

# add cmake module dir to the cmake module path
set(CMAKE_MODULE_PATH 
    "${CMAKE_MODULE_PATH}"
    "${CMAKE_SOURCE_DIR}"
    "${CMAKE_SOURCE_DIR}/cmake"
)

### COMPILER FLAGS ###
# note: add_definitions can be used as well, but is intended for preprocessor definitions

# Compiler flags
set(COMPILERS_LIST Clang GNU)
if (${CMAKE_CXX_COMPILER_ID} IN_LIST COMPILERS_LIST)
    # setup debugging flags
    set(CMAKE_CXX_FLAGS "-g")
    # Setup warning flags
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")
    # Specify language's standard version
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")
endif ()

# New way to set the required language standard, does not work on travis CI
#set(CMAKE_CXX_STANDARD 14)
#set(CMAKE_CXX_STANDARD_REQUIRED true)
#set(CMAKE_CXX_EXTENSIONS false)


### EXTERNAL LIBRARIES ###

find_package(GLFW REQUIRED)

### INCLUDE HEADER FILES ###
include_directories(
    ${PROJECT_SOURCE_DIR}/include
    ${GLFW_INCLUDE_DIRS}
)

# CPack configuration
SET(CPACK_PACKAGE_NAME "GLFWpp")
## Setting contact and vendor details
SET(CPACK_PACKAGE_VENDOR "Joe R. Nassimian")
SET(CPACK_PACKAGE_CONTACT "joe.nassimian@gmail.com")

SET(CPACK_RESOURCE_FILE_LICENSE ${PROJECT_SOURCE_DIR}/LICENSE)
SET(CPACK_RESOURCE_FILE_README ${PROJECT_SOURCE_DIR}/README)

## Setting package description
SET(
    CPACK_PACKAGE_DESCRIPTION
    "A C++ wrapper for GLFW"
)

ADD_SUBDIRECTORY(docs)
ADD_SUBDIRECTORY(src)
