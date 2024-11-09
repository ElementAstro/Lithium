# - Find JPEG
# Find the native JPEG includes and library
#
# This module defines the following variables:
#  JPEG_FOUND - True if JPEG was found
#  JPEG_INCLUDE_DIRS - The directory containing jpeglib.h
#  JPEG_LIBRARIES - The libraries needed to use JPEG

find_path(JPEG_INCLUDE_DIR
    NAMES jpeglib.h
    PATHS ${JPEG_ROOT_DIR}/include
    DOC "Path to the JPEG include directory"
)

find_library(JPEG_LIBRARY
    NAMES ${JPEG_NAMES} jpeg
    PATHS ${JPEG_ROOT_DIR}/lib
    DOC "Path to the JPEG library"
)

# Handle the QUIETLY and REQUIRED arguments and set JPEG_FOUND
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(JPEG
    REQUIRED_VARS JPEG_LIBRARY JPEG_INCLUDE_DIR
)

if(JPEG_FOUND)
    set(JPEG_INCLUDE_DIRS ${JPEG_INCLUDE_DIR})
    set(JPEG_LIBRARIES ${JPEG_LIBRARY})
    # Set additional compile definitions if needed
    if(UNIX AND NOT APPLE)
        add_definitions(-DJPEG_STATIC)
    endif()
endif()

# Deprecated declarations (for backward compatibility)
set(NATIVE_JPEG_INCLUDE_PATH ${JPEG_INCLUDE_DIR})
get_filename_component(NATIVE_JPEG_LIB_PATH ${JPEG_LIBRARY} PATH)

mark_as_advanced(JPEG_LIBRARY JPEG_INCLUDE_DIR)
