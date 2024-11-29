# FindFFI.cmake 

# Locate libffi
# This module defines
#  FFI_FOUND - True if libffi was found
#  FFI_INCLUDE_DIRS - Include directories for libffi
#  FFI_LIBRARIES - Linker flags for libffi

find_path(FFI_INCLUDE_DIRS
    NAMES ffi.h
    PATHS /usr/include /usr/local/include
)

find_library(FFI_LIBRARIES
    NAMES ffi
    PATHS /usr/lib /usr/local/lib
)

# Check if both the include directory and the library are found
if (FFI_INCLUDE_DIRS AND FFI_LIBRARIES)
    set(FFI_FOUND TRUE)
else()
    set(FFI_FOUND FALSE)
endif()

# Provide a message about the finding
if (FFI_FOUND)
    message(STATUS "Found libffi: ${FFI_LIBRARIES}")
else()
    message(WARNING "libffi not found")
endif()

# Export the results
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(FFI DEFAULT_MSG FFI_LIBRARIES FFI_INCLUDE_DIRS)

mark_as_advanced(FFI_INCLUDE_DIRS FFI_LIBRARIES)
