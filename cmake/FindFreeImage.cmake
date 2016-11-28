#.rst:
#
# ==============
# Find FreeImage
# ==============
#
# Try to find FreeImage: Open Asset Import Library.
#
# The module defines the following variables:
# - FREEIMAGE_FOUND - True if FreeImage development files were found
# - FREEIMAGE_INCLUDE_DIR - FreeImage include directories
# - FREEIMAGE_LIBRARIES - FreeImage libraries to link
# - FREEIMAGE_DLL - FreeImage DLL library (Windows only)
#
# The following variables can be set as arguments for the module.
# - FREEIMAGE_ROOT_DIR: Root library directory of Assimp
#

# Additional modules
#include(FindPackageHandleStandardArgs)
include(ArchBitSize)
include(XPlatform)


SET(_PF86 "PROGRAMFILES(x86)")
SET(FREEIMAGE_SEARCH_PATHS
    # Can be set as this module's argument
    ${FREEIMAGE_ROOT_DIR}
    # Top project's local external libraries
    ${CMAKE_SOURCE_DIR}/external/freeimage
    # Current project's local external libraries
    ${PROJECT_SOURCE_DIR}/external/freeimage
    # Unix like systems
    /usr/local /usr /opt
    # Windows specific
    "$ENV{PROGRAMFILES}/freeimage"
    "$ENV{_PF86}/freeimage"
    "$ENV{SYSTEMDRIVE}/freeimage"
    # MacOS specific
    ~/Library/Frameworks /Library/Frameworks
    # Other exotic options (Fink, DarwinPorts & Blastwave)
    /sw /opt/local /opt/csw
)

if (NOT DEFINED FREEIMAGE_ROOT_DIR)
    FIND_PATH(FREEIMAGE_ROOT_DIR Dist/x32/FreeImage.h
        PATH_SUFFIXES
            freeimage
            FreeImage
        PATHS
            ${FREEIMAGE_SEARCH_PATHS}
    )
endif (NOT DEFINED FREEIMAGE_ROOT_DIR)

SET(FREEIMAGE_SEARCH_PATHS
    ${FREEIMAGE_SEARCH_PATHS}
    # Can be set as this module's argument
    ${FREEIMAGE_ROOT_DIR}
)

# Specify 64 bit architectures search paths
if (${XXBITS} STREQUAL x86_64)
    SET(FREEIMAGE_XXBITS x64)
 
    SET(FREEIMAGE_SEARCH_LIB_PATH_SUFFIXES
        lib64
        lib/${FREEIMAGE_XXBITS}
        Dist/${XXBITS}
        Dist/${FREEIMAGE_XXBITS}
    )
# Specify 32 bit architectures search paths
else (${XXBITS} STREQUAL x86_64)
    SET(FREEIMAGE_XXBITS x32)
    SET(FREEIMAGE_SEARCH_LIB_PATH_SUFFIXES
        lib/${XXBITS}
        Dist/${XXBITS}
        Dist/${FREEIMAGE_XXBITS}
    )

    if (NOT MSVC)
        SET(FREEIMAGE_SEARCH_LIB_PATH_SUFFIXES
            ${FREEIMAGE_SEARCH_LIB_PATH_SUFFIXES}
            lib
            lib32
            lib/mingw
        )
    endif (NOT MSVC)
endif (${XXBITS} STREQUAL x86_64)


if (WIN32)
    SET(FREEIMAGE_SEARCH_DLL_PATH_SUFFIXES
        ${FREEIMAGE_SEARCH_LIB_PATH_SUFFIXES}
    )
endif (WIN32)


FIND_PATH(FREEIMAGE_INCLUDE_DIR
    NAMES
        FreeImage.h freeimage.h
    PATH_SUFFIXES
        include
        include/FreeImage
        Dist
        Dist/${XXBITS}
        Dist/${FREEIMAGE_XXBITS}
    PATHS
        ${FREEIMAGE_SEARCH_PATHS}
    DOC
        "The directory where FreeImage's development headers reside"
)

FIND_PATH(FREEIMAGE_LIBRARY_DIR
    NAMES FreeImage.lib freeimage.lib
    PATH_SUFFIXES
        ${FREEIMAGE_SEARCH_LIB_PATH_SUFFIXES}
    PATHS
        ${FREEIMAGE_SEARCH_PATHS}
)

FIND_LIBRARY(FREEIMAGE_LIBRARY
    NAMES
        FreeImage freeimage
    PATH_SUFFIXES
        ${FREEIMAGE_SEARCH_LIB_PATH_SUFFIXES}
    PATHS
        ${FREEIMAGE_LIBRARY_DIR}
        ${FREEIMAGE_SEARCH_PATHS}
	DOC "The FreeImage library"
)

SET(FREEIMAGE_INCLUDE_DIRS "${FREEIMAGE_INCLUDE_DIR}")
SET(FREEIMAGE_LIBRARY_DIRS "${FREEIMAGE_LIBRARY_DIR}")

if (FREEIMAGE_LIBRARY)
    SET(FREEIMAGE_LIBRARIES "${FREEIMAGE_LIBRARY}")
endif (FREEIMAGE_LIBRARY)

if (FREEIMAGE_INCLUDE_DIR)
    file(STRINGS ${FREEIMAGE_INCLUDE_DIR}/FreeImage.h FREEIMAGE_VERSION_STR
    REGEX "^#[\t ]*define[\t ]+FREEIMAGE_(MAJOR_VERSION|MINOR_VERSION|RELEASE_SERIAL)[\t ]+[0-9]+$")
    
    unset(FREEIMAGE_VERSION_STRING)
    foreach(VLINE ${FREEIMAGE_VERSION_STR})
        foreach(VPART MAJOR_VERSION MINOR_VERSION RELEASE_SERIAL)
            if (VLINE MATCHES "^#[\t ]*define[\t ]+FREEIMAGE_${VPART}[\t ]+([0-9]+)$")
                set(FREEIMAGE_VERSION_PART "${CMAKE_MATCH_1}")
                if (FREEIMAGE_VERSION_STRING)
                    set(FREEIMAGE_VERSION_STRING "${FREEIMAGE_VERSION_STRING}.${FREEIMAGE_VERSION_PART}")
                else (FREEIMAGE_VERSION_STRING)
                    set(FREEIMAGE_VERSION_STRING "${FREEIMAGE_VERSION_PART}")
                endif (FREEIMAGE_VERSION_STRING)
                unset(FREEIMAGE_VERSION_PART)
            endif ()
        endforeach()
    endforeach()
endif (FREEIMAGE_INCLUDE_DIR)

MARK_AS_ADVANCED(
    FREEIMAGE_FOUND
    FREEIMAGE_LIBRARY
    FREEIMAGE_LIBRARIES
    FREEIMAGE_LIBRARY_DIR
    FREEIMAGE_INCLUDE_DIRS
)

find_package_handle_standard_args(
    FreeImage
    REQUIRED_VARS
        FREEIMAGE_LIBRARIES
        FREEIMAGE_INCLUDE_DIRS
    VERSION_VAR
        FREEIMAGE_VERSION_STRING
)
