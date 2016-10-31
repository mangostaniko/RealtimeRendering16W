# Try to find FreeImage
# When found, this will define the following:
# - FreeImage_FOUND          system has FreeImage
# - FreeImage_INCLUDE_DIRS   the FreeImage include directories
# - FreeImage_LIBRARIES        link these to use FreeImage

# the include dir
find_path(FreeImage_INCLUDE_DIR
	NAMES FreeImage.h
	HINTS $ENV{FREEIMAGE_ROOT}/include
)

# the library itself
find_library(FreeImage_LIBRARY
	NAMES freeimageplus
	HINTS $ENV{FREEIMAGE_ROOT}/lib
)

if(FreeImage_INCLUDE_DIR)
    SET(FreeImage_INCLUDE_DIRS "${FreeImage_INCLUDE_DIR}")
    SET(FreeImage_LIBRARIES "${FreeImage_LIBRARY}")
    message(STATUS "Found FreeImage: ${FreeImage_INCLUDE_DIRS}")
    message(STATUS "Found FreeImage: ${FreeImage_LIBRARIES}")
endif(FreeImage_INCLUDE_DIR)
