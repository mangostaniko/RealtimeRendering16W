#.rst:
#
# ==================
# Find FreeImagePlus
# ==================
#
# Try to find FreeImage: Open Asset Import Library.
#
# The module defines the following variables:
# - FREEIMAGEPLUS_FOUND - True if FreeImage development files were found
# - FREEIMAGEPLUS_INCLUDE_DIRS - FreeImage include directories
# - FREEIMAGEPLUS_LIBRARIES - FreeImage libraries to link
# - FREEIMAGEPLUS_DLL - FreeImage DLL library (Windows only)
#
# The following variables can be set as arguments for the module.
# - FREEIMAGEPLUS_ROOT_DIR: Root library directory of Assimp
#

include(ArchBitSize)
include(XPlatform)

include(FindFreeImage)


SET(_PF86 "PROGRAMFILES(x86)")
SET(FREEIMAGEPLUS_SEARCH_PATHS
    # Can be set as this module's argument
    ${FREEIMAGEPLUS_ROOT_DIR}
    # Get FreeImage's root directory as FreeImagePlus is bundled with it.
    ${FREEIMAGE_ROOT_DIR}
    # Top project's local external libraries
    ${CMAKE_SOURCE_DIR}/external/freeimageplus
    # Current project's local external libraries
    ${PROJECT_SOURCE_DIR}/external/freeimageplus
    # Unix like systems
    /usr/local /usr /opt
    # Windows specific
    "$ENV{PROGRAMFILES}/freeimageplus"
    "$ENV{_PF86}/freeimageplus"
    "$ENV{SYSTEMDRIVE}/freeimageplus"
    # MacOS specific
    ~/Library/Frameworks /Library/Frameworks
    # Other exotic options (Fink, DarwinPorts & Blastwave)
    /sw /opt/local /opt/csw
)

if (NOT DEFINED FREEIMAGEPLUS_ROOT_DIR)
    FIND_PATH(FREEIMAGEPLUS_ROOT_DIR dist/x32/FreeImagePlus.h
        PATH_SUFFIXES
            FreeImage/Wrapper/FreeImagePlus
            Wrapper/FreeImagePlus
        PATHS
            ${FREEIMAGEPLUS_SEARCH_PATHS}
    )
endif (NOT DEFINED FREEIMAGEPLUS_ROOT_DIR)

SET(FREEIMAGEPLUS_SEARCH_PATHS
    ${FREEIMAGEPLUS_SEARCH_PATHS}
    # Can be set as this module's argument
    ${FREEIMAGEPLUS_ROOT_DIR}
)

# Specify 64 bit architectures search paths
if (${XXBITS} STREQUAL x86_64)
    SET(FREEIMAGEPLUS_XXBITS x64)
 
    SET(FREEIMAGEPLUS_SEARCH_LIB_PATH_SUFFIXES
        lib64
        lib/${FREEIMAGEPLUS_XXBITS}
        dist/${XXBITS}
        dist/${FREEIMAGEPLUS_XXBITS}
    )
# Specify 32 bit architectures search paths
else (${XXBITS} STREQUAL x86_64)
    SET(FREEIMAGEPLUS_XXBITS x32)
    SET(FREEIMAGEPLUS_SEARCH_LIB_PATH_SUFFIXES
        lib/${XXBITS}
        dist/${XXBITS}
        dist/${FREEIMAGEPLUS_XXBITS}
    )

    if (NOT MSVC)
        SET(FREEIMAGEPLUS_SEARCH_LIB_PATH_SUFFIXES
            ${FREEIMAGEPLUS_SEARCH_LIB_PATH_SUFFIXES}
            lib
            lib32
            lib/mingw
        )
    endif (NOT MSVC)
endif (${XXBITS} STREQUAL x86_64)


FIND_PATH(FREEIMAGEPLUS_INCLUDE_DIR
    NAMES
        FreeImagePlus.h freeimageplus.h
    PATH_SUFFIXES
        include
        include/FreeImage
        include/FreeImagePlus
        dist
        ${FREEIMAGEPLUS_SEARCH_LIB_PATH_SUFFIXES}
    PATHS
        ${FREEIMAGEPLUS_SEARCH_PATHS}
    DOC
        "The directory where FreeImagePlus's development headers reside"
)

FIND_PATH(FREEIMAGEPLUS_LIBRARY_DIR
    NAMES FreeImagePlus.lib freeimageplus.lib
    PATH_SUFFIXES
        ${FREEIMAGEPLUS_SEARCH_LIB_PATH_SUFFIXES}
    PATHS
        ${FREEIMAGEPLUS_SEARCH_PATHS}
)

FIND_LIBRARY(FREEIMAGEPLUS_LIBRARY
    NAMES
        FreeImagePlus freeimageplus
    PATH_SUFFIXES
        ${FREEIMAGEPLUS_SEARCH_LIB_PATH_SUFFIXES}
    PATHS
        ${FREEIMAGEPLUS_LIBRARY_DIR}
        ${FREEIMAGEPLUS_SEARCH_PATHS}
	DOC
	    "The FreeImagePlus library"
)


SET(FREEIMAGEPLUS_INCLUDE_DIRS "${FREEIMAGEPLUS_INCLUDE_DIR}")
SET(FREEIMAGEPLUS_LIBRARY_DIRS "${FREEIMAGEPLUS_LIBRARY_DIR}")

if (FREEIMAGEPLUS_LIBRARY)
    SET(FREEIMAGEPLUS_LIBRARIES "${FREEIMAGEPLUS_LIBRARY}")
endif (FREEIMAGEPLUS_LIBRARY)


MARK_AS_ADVANCED(
    FREEIMAGEPLUS_FOUND
    FREEIMAGEPLUS_LIBRARY
    FREEIMAGEPLUS_LIBRARIES
    FREEIMAGEPLUS_LIBRARY_DIR
    FREEIMAGEPLUS_INCLUDE_DIRS
)

find_package_handle_standard_args(
    FreeImagePlus
    REQUIRED_VARS
        FREEIMAGEPLUS_LIBRARIES
        FREEIMAGEPLUS_INCLUDE_DIRS
    VERSION_VAR
        FREEIMAGE_VERSION_STRING
)
