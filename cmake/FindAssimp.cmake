#
# Find Assimp
#
# Try to find Assimp: Open Asset Import Library.
#
# The module defines the following variables:
# - ASSIMP_FOUND - True if Assimp development files were found
# - ASSIMP_INCLUDE_DIR - Assimp include directories
# - ASSIMP_LIBRARIES
# - ASSIMP_LIBRARY_RELEASE - Assimp libraries to link in release configuration
# - ASSIMP_LIBRARY_DEBUG - Assimp libraries to link in debug configuration
# - ASSIMP_DLL_RELEASE - Assimp DLL library (Windows only)
# - ASSIMP_DLL_DEBUG - Assimp DLL library (Windows only)
#
# The following variables can be set as arguments for the module.
# - ASSIMP_ROOT_DIR: Root library directory of Assimp
#

# Additional modules
#include(FindPackageHandleStandardArgs)
include(ArchBitSize)

SET(_PF86 "PROGRAMFILES(x86)")
SET(ASSIMP_SEARCH_PATHS
    # Can be set as this module's argument
    ${ASSIMP_ROOT_DIR}
    # Top project's local external libraries
    ${CMAKE_SOURCE_DIR}/external/assimp
    # Current project's local external libraries
    ${PROJECT_SOURCE_DIR}/external/assimp
    # Unix like systems
    /usr/local /usr /opt
    # Windows specific
    $ENV{PROGRAMFILES}/ASSIMP $ENV{_PF86}/ASSIMP
    # MacOS specific
    ~/Library/Frameworks /Library/Frameworks
    # Other exotic options (Fink, DarwinPorts & Blastwave)
    /sw /opt/local /opt/csw
)

FIND_PATH(ASSIMP_ROOT_DIR include/assimp/version.h
    PATH_SUFFIXES
        assimp
    PATHS
        ${ASSIMP_SEARCH_PATHS}
)

FIND_PATH(ASSIMP_INCLUDE_DIR
    NAMES
        assimp/version.h
    PATH_SUFFIXES
        include
    PATHS
        ${ASSIMP_SEARCH_PATHS}
    DOC
        "The directory where assimp's development headers reside"
)

# Check if on decent OS
if (UNIX)
    SET(CMAKE_FIND_LIBRARY_PREFIXES "lib")
    SET(CMAKE_FIND_LIBRARY_SUFFIXES ".so" ".a")
# Deal with lesser OSes
elseif (WIN32)
    if (MSVC)
        SET(CMAKE_FIND_LIBRARY_PREFIXES "")
        SET(CMAKE_FIND_LIBRARY_SUFFIXES ".lib")
    else (MSVC)
        # Can accept lib<name>.a or <name>.lib.
        # More info:
        # http://www.mingw.org/wiki/specify_the_libraries_for_the_linker_to_use
        SET(CMAKE_FIND_LIBRARY_PREFIXES "" "lib")
        SET(CMAKE_FIND_LIBRARY_SUFFIXES ".lib" ".a" "dll.a")
    endif (WIN32)
endif (UNIX)


# Specify 64 bit architectures search paths
if (${XXBITS} STREQUAL x86_64)
    set(ASSIMP_XXBITS x64)
    set(ASSIMP_SEARCH_LIB_PATH_SUFFIXES lib64)

    set(ASSIMP_BUILD_TYPE Release)
    set(ASSIMP_SEARCH_RELEASE_LIB_PATH_SUFFIXES
        lib/${ASSIMP_XXBITS}/${ASSIMP_BUILD_TYPE}
        lib/${ASSIMP_BUILD_TYPE}/${ASSIMP_XXBITS}
    )

    set(ASSIMP_BUILD_TYPE Debug)
    set(ASSIMP_SEARCH_DEBUG_LIB_PATH_SUFFIXES
        lib/${ASSIMP_XXBITS}/${ASSIMP_BUILD_TYPE}
        lib/${ASSIMP_BUILD_TYPE}/${ASSIMP_XXBITS}
    )

    if (WIN32)
        set(ASSIMP_BUILD_TYPE release)
        set(ASSIMP_SEARCH_RELEASE_DLL_PATH_SUFFIXES
            bin64
            bin/assimp_${ASSIMP_BUILD_TYPE}-dll_${ASSIMP_XXBITS}
        )
        set(ASSIMP_BUILD_TYPE debug)
        set(ASSIMP_SEARCH_DEBUG_DLL_PATH_SUFFIXES
            bin64
            bin/assimp_${ASSIMP_BUILD_TYPE}-dll_${ASSIMP_XXBITS}
        )
    endif (WIN32)
    if (MSVC)
        set(ASSIMP_SEARCH_LIB_PATH_SUFFIXES
            ${ASSIMP_SEARCH_LIB_PATH_SUFFIXES}
            lib/${ASSIMP_XXBITS}
        )

        set(ASSIMP_BUILD_TYPE release)
        set(ASSIMP_SEARCH_RELEASE_LIB_PATH_SUFFIXES
            ${ASSIMP_SEARCH_RELEASE_LIB_PATH_SUFFIXES}
            lib/assimp_${ASSIMP_BUILD_TYPE}-dll_${ASSIMP_XXBITS}
        )

        set(ASSIMP_BUILD_TYPE debug)
        set(ASSIMP_SEARCH_DEBUG_LIB_PATH_SUFFIXES
            ${ASSIMP_SEARCH_DEBUG_LIB_PATH_SUFFIXES}
            lib/assimp_${ASSIMP_BUILD_TYPE}-dll_${ASSIMP_XXBITS}
        )
    else (MSVC)
        set(ASSIMP_XXBITS win64)
        set(ASSIMP_BUILD_TYPE release)
        set(ASSIMP_SEARCH_RELEASE_LIB_PATH_SUFFIXES
            ${ASSIMP_SEARCH_RELEASE_LIB_PATH_SUFFIXES}
            lib/assimp_${ASSIMP_BUILD_TYPE}-dll_${ASSIMP_XXBITS}
        )

        set(ASSIMP_BUILD_TYPE debug)
        set(ASSIMP_SEARCH_DEBUG_LIB_PATH_SUFFIXES
            ${ASSIMP_SEARCH_DEBUG_LIB_PATH_SUFFIXES}
            lib/assimp_${ASSIMP_BUILD_TYPE}-dll_${ASSIMP_XXBITS}
        )
    endif (MSVC)
# Specify 32 bit architectures search paths
else (${XXBITS} STREQUAL x86_64)
    set(ASSIMP_SEARCH_LIB_PATH_SUFFIXES lib/${XXBITS})
    set(ASSIMP_XXBITS ${XXBITS})

    set(ASSIMP_BUILD_TYPE Release)
    set(ASSIMP_SEARCH_RELEASE_LIB_PATH_SUFFIXES
        lib/${ASSIMP_XXBITS}/${ASSIMP_BUILD_TYPE}
        lib/${ASSIMP_BUILD_TYPE}/${ASSIMP_XXBITS}
    )

    set(ASSIMP_BUILD_TYPE Debug)
    set(ASSIMP_SEARCH_DEBUG_LIB_PATH_SUFFIXES
        lib/${ASSIMP_XXBITS}/${ASSIMP_BUILD_TYPE}
        lib/${ASSIMP_BUILD_TYPE}/${ASSIMP_XXBITS}
    )

    set(ASSIMP_XXBITS win32)

    set(ASSIMP_BUILD_TYPE release)
    set(ASSIMP_SEARCH_RELEASE_LIB_PATH_SUFFIXES
        ${ASSIMP_SEARCH_RELEASE_LIB_PATH_SUFFIXES}
        lib/assimp_${ASSIMP_BUILD_TYPE}-dll_${ASSIMP_XXBITS}
    )
    set(ASSIMP_BUILD_TYPE debug)
    set(ASSIMP_SEARCH_DEBUG_LIB_PATH_SUFFIXES
        ${ASSIMP_SEARCH_DEBUG_LIB_PATH_SUFFIXES}
        lib/assimp_${ASSIMP_BUILD_TYPE}-dll_${ASSIMP_XXBITS}
    )

    if (NOT MSVC)
        set(ASSIMP_SEARCH_LIB_PATH_SUFFIXES
            ${ASSIMP_SEARCH_LIB_PATH_SUFFIXES}
            lib
            lib32
            lib/mingw
        )
    endif (NOT MSVC)
    if (WIN32)
        set(ASSIMP_BUILD_TYPE release)
        set(ASSIMP_SEARCH_RELEASE_DLL_PATH_SUFFIXES
            bin32
            bin/assimp_${ASSIMP_BUILD_TYPE}-dll_${ASSIMP_XXBITS}
        )
        set(ASSIMP_BUILD_TYPE debug)
        set(ASSIMP_SEARCH_DEBUG_DLL_PATH_SUFFIXES
            bin32
            bin/assimp_${ASSIMP_BUILD_TYPE}-dll_${ASSIMP_XXBITS}
        )
    endif (WIN32)
endif (${XXBITS} STREQUAL x86_64)


FIND_PATH(ASSIMP_LIBRARY_RELEASE_DIR assimp.lib
    PATH_SUFFIXES
        ${ASSIMP_SEARCH_LIB_PATH_SUFFIXES}
        ${ASSIMP_SEARCH_RELEASE_LIB_PATH_SUFFIXES}
    PATHS
        ${ASSIMP_SEARCH_PATHS}
)

FIND_PATH(ASSIMP_LIBRARY_DEBUG_DIR assimpD.lib
    PATH_SUFFIXES
        ${ASSIMP_SEARCH_LIB_PATH_SUFFIXES}
        ${ASSIMP_SEARCH_DEBUG_LIB_PATH_SUFFIXES}
    PATHS
        ${ASSIMP_SEARCH_PATHS}
)

FIND_LIBRARY(ASSIMP_LIBRARY_RELEASE
    NAMES
        assimp
    PATHS
        ${ASSIMP_LIBRARY_RELEASE_DIR}
)

FIND_LIBRARY(ASSIMP_LIBRARY_DEBUG
    NAMES
        assimpD
    PATHS
        ${ASSIMP_LIBRARY_DEBUG_DIR}
)


# Find dll paths using path suffixes computed above
FIND_PATH(ASSIMP_DLL_RELEASE_DIR
    NAMES assimp.dll assimp.exe Assimp32.dll Assimp64.dll
    PATH_SUFFIXES
        ${ASSIMP_SEARCH_RELEASE_DLL_PATH_SUFFIXES}
    PATHS
        ${ASSIMP_SEARCH_PATHS}
)


FIND_PATH(ASSIMP_DLL_DEBUG_DIR
    NAMES assimp.dll assimp.exe Assimp32d.dll Assimp64d.dll
    PATH_SUFFIXES
        ${ASSIMP_SEARCH_RELEASE_DLL_PATH_SUFFIXES}
    PATHS
        ${ASSIMP_SEARCH_PATHS}
)

# Release runtime dlls
FILE(GLOB ASSIMP_DLL_RELEASE
    "${ASSIMP_DLL_RELEASE_DIR}/*.dll"
    "${ASSIMP_DLL_RELEASE_DIR}/*.exe"
)

# Debug runtime dlls
FILE(GLOB ASSIMP_DLL_RELEASE
    "${ASSIMP_DLL_DEBUG_DIR}/*.dll"
    "${ASSIMP_DLL_DEBUG_DIR}/*.exe"
)


MARK_AS_ADVANCED(ASSIMP_LIBRARY_RELEASE_DIR)
MARK_AS_ADVANCED(ASSIMP_LIBRARY_DEBUG_DIR)
MARK_AS_ADVANCED(ASSIMP_LIBRARY_RELEASE)
MARK_AS_ADVANCED(ASSIMP_LIBRARY_DEBUG_DIR)

find_package_handle_standard_args(
    ASSIMP
    DEFAULT_MSG
    ASSIMP_LIBRARY_RELEASE
    ASSIMP_INCLUDE_DIR
)

if (ASSIMP_FOUND)
    if (ASSIMP_LIBRARY_DEBUG)
        set(ASSIMP_LIBRARIES optimized ${ASSIMP_LIBRARY_RELEASE} debug ${ASSIMP_LIBRARY_DEBUG} CACHE STRING "Assimp libraries")
        set(ASSIMP_LIBRARY_DIRS ${ASSIMP_LIBRARY_RELEASE_DIR} ${ASSIMP_LIBRARY_DEBUG_DIR})
    else()
        set(ASSIMP_LIBRARIES ${ASSIMP_LIBRARY_RELEASE} CACHE STRING "Assimp libraries")
        set(ASSIMP_LIBRARY_DIRS ${ASSIMP_LIBRARY_RELEASE_DIR})
    endif()
endif (ASSIMP_FOUND)
