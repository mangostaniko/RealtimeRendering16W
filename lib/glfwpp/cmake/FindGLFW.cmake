#.rst:
#
# =========
# Find GLFW
# =========
#
# Try to find GLFW: Open Asset Import Library.
#
# The module defines the following variables:
# - GLFW_FOUND - True if Assimp development files were found
# - GLFW_INCLUDE_DIR - Assimp include directories
# - GLFW_LIBRARIES
# - GLFW_DLL - Assimp DLL library (Windows only)
#
# The following variables can be set as arguments for the module.
# - GLFW_ROOT_DIR: Root library directory of Assimp
# - GLFW_USE_STATIC_LIBS: Use static GLFW library (Windows only)
#

# Additional modules
#include(FindPackageHandleStandardArgs)
include(ArchBitSize)
include(XPlatform)


# Macro to print some message to stdout, useful for debugging purpose.
MACRO(DBG_MSG _MSG)
    MESSAGE(STATUS "${CMAKE_CURRENT_LIST_FILE}(${CMAKE_CURRENT_LIST_LINE}): ${_MSG}")
ENDMACRO(DBG_MSG)


Set(_PF86 "PROGRAMFILES(x86)")
SET(GLFW_SEARCH_PATHS
    # Top project's local external libraries
    ${CMAKE_SOURCE_DIR}/external/
    # Current project's local external libraries
    ${PROJECT_SOURCE_DIR}/external/
    # Unix like systems
    /usr/local /usr /opt
    # Windows specific
    "$ENV{PROGRAMFILES}"
    "$ENV{_PF86}"
    "$ENV{SYSTEMDRIVE}"
    # MacOS specific
    ~/Library/Frameworks /Library/Frameworks
    # Other exotic options (Fink, DarwinPorts & Blastwave)
    /sw /opt/local /opt/csw
)

if (NOT DEFINED GLFW_ROOT_DIR)
    FIND_PATH(GLFW_ROOT_DIR include/GLFW/glfw3.h
        PATH_SUFFIXES
            glfw GLFW
        PATHS
            ${GLFW_SEARCH_PATHS}
    )
endif (NOT DEFINED GLFW_ROOT_DIR)

SET(GLFW_SEARCH_PATHS
    ${GLFW_SEARCH_PATHS}
    # Can be set as this module's argument
    ${GLFW_ROOT_DIR}
)

# Specify 64 bit architectures search paths
if (${XXBITS} STREQUAL x86_64)
    SET(GLFW_XXBITS x64)
 
    SET(GLFW_SEARCH_LIB_PATH_SUFFIXES
        lib64
        lib/${GLFW_XXBITS}
        ${XXBITS}/lib-mingw-w64
    )
# Specify 32 bit architectures search paths
else (${XXBITS} STREQUAL x86_64)
    SET(GLFW_XXBITS ${XXBITS})
    SET(GLFW_SEARCH_LIB_PATH_SUFFIXES
        lib/${GLFW_XXBITS}
        ${XXBITS}/lib-mingw-w64
    )

    if (NOT MSVC)
        SET(GLFW_SEARCH_LIB_PATH_SUFFIXES
            ${GLFW_SEARCH_LIB_PATH_SUFFIXES}
            lib
            lib32
            lib/mingw
        )
    endif (NOT MSVC)
endif (${XXBITS} STREQUAL x86_64)


FIND_PATH(GLFW_INCLUDE_DIR
    NAMES
        GLFW/glfw3.h
    PATH_SUFFIXES
        # For finding the include file under the root of the glfw expanded archive, typically on Windows.
        include
    PATHS
        ${GLFW_SEARCH_PATHS}
    DOC
        "The directory where GLFW's development headers reside"
)
DBG_MSG("GLFW_INCLUDE_DIR = ${GLFW_INCLUDE_DIR}")


FIND_LIBRARY(GLFW_LIBRARY
    NAMES
        glfw glfw3 GLFW
    PATH_SUFFIXES
        # For finding the library file under the root of the glfw expanded archive, typically on Windows.
        ${GLFW_SEARCH_LIB_PATH_SUFFIXES}
        lib/win32 
    PATHS
        ${GLFW_SEARCH_PATHS}
    DOC
        "Absolute path to GLFW library."
)
DBG_MSG("GLFW_LIBRARY = ${GLFW_LIBRARY}")


SET(GLFW_INCLUDE_DIRS "${GLFW_INCLUDE_DIR}")
SET(GLFW_LIBRARY_DIRS "${GLFW_LIBRARY_DIR}")

if (GLFW_LIBRARY)
    SET(GLFW_LIBRARIES "${GLFW_LIBRARY}")
endif (GLFW_LIBRARY)


if (GLFW_INCLUDE_DIR AND EXISTS "${GLFW_INCLUDE_DIR}/GLFW/glfw3.h")
    file(STRINGS "${GLFW_INCLUDE_DIR}/GLFW/glfw3.h" glfw_version_str
         REGEX "^#[\t ]*define[\t ]+GLFW_VERSION_(MAJOR|MINOR|REVISION)[\t ]+[0-9]+$")

    UNSET(GLFW_VERSION_STRING)
    foreach(VLINE ${glfw_version_str})
        foreach(VPART MAJOR MINOR REVISION)
            if (VLINE MATCHES "^#[\t ]*define[\t ]+GLFW_VERSION_${VPART}[\t ]+([0-9]+)$")
                set(GLFW_VERSION_PART "${CMAKE_MATCH_1}")
                if (GLFW_VERSION_STRING)
                    SET(GLFW_VERSION_STRING "${GLFW_VERSION_STRING}.${GLFW_VERSION_PART}")
                else (GLFW_VERSION_STRING)
                    SET(GLFW_VERSION_STRING "${GLFW_VERSION_PART}")
                endif (GLFW_VERSION_STRING)
                UNSET(GLFW_VERSION_PART)
            endif ()
        ENDFOREACH(VPART)
    ENDFOREACH(VLINE)
ENDIF(GLFW_INCLUDE_DIR AND EXISTS "${GLFW_INCLUDE_DIR}/GLFW/glfw3.h")


find_package_handle_standard_args(
    GLFW
    REQUIRED_VARS
        GLFW_LIBRARIES
        GLFW_INCLUDE_DIRS
    VERSION_VAR
        GLFW_VERSION_STRING
)
