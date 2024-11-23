# FindLibGit2.cmake
# This module looks for the libgit2 library and sets the following variables:
#   LIBGIT2_FOUND        - True if libgit2 was found
#   LIBGIT2_INCLUDE_DIRS - Directories containing libgit2 headers
#   LIBGIT2_LIBRARIES    - Libraries to link against libgit2
#   LIBGIT2_VERSION      - Version of libgit2 found

# Use pkg-config if available (usually on Linux and macOS)
find_package(PkgConfig QUIET)
if(PkgConfig_FOUND)
    pkg_check_modules(PC_LIBGIT2 libgit2)
endif()

# Try to find libgit2 using the CMake find_path/find_library functions
find_path(LIBGIT2_INCLUDE_DIR
    NAMES git2.h
    PATHS ${PC_LIBGIT2_INCLUDEDIR} ${PC_LIBGIT2_INCLUDE_DIRS} ENV INCLUDE
    PATH_SUFFIXES libgit2
)

find_library(LIBGIT2_LIBRARY
    NAMES git2
    PATHS ${PC_LIBGIT2_LIBDIR} ${PC_LIBGIT2_LIBRARY_DIRS} ENV LIB
)

# Check if both the include path and library were found
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(LibGit2 REQUIRED_VARS LIBGIT2_LIBRARY LIBGIT2_INCLUDE_DIR
                                  VERSION_VAR PC_LIBGIT2_VERSION)

# Set output variables
if(LIBGIT2_FOUND)
    set(LIBGIT2_INCLUDE_DIRS ${LIBGIT2_INCLUDE_DIR})
    set(LIBGIT2_LIBRARIES ${LIBGIT2_LIBRARY})
    
    # Try to extract the version if not provided by pkg-config
    if(NOT LIBGIT2_VERSION AND EXISTS "${LIBGIT2_INCLUDE_DIR}/git2/version.h")
        file(READ "${LIBGIT2_INCLUDE_DIR}/git2/version.h" _version_header)
        string(REGEX MATCHALL "#define[ \t]+LIBGIT2_VER_[A-Z]+[ \t]+([0-9]+)" _version_matches ${_version_header})
        list(GET _version_matches 0 _major_match)
        list(GET _version_matches 1 _minor_match)
        list(GET _version_matches 2 _patch_match)
        string(REGEX REPLACE ".*([0-9]+).*" "\\1" LIBGIT2_VERSION_MAJOR ${_major_match})
        string(REGEX REPLACE ".*([0-9]+).*" "\\1" LIBGIT2_VERSION_MINOR ${_minor_match})
        string(REGEX REPLACE ".*([0-9]+).*" "\\1" LIBGIT2_VERSION_PATCH ${_patch_match})
        set(LIBGIT2_VERSION "${LIBGIT2_VERSION_MAJOR}.${LIBGIT2_VERSION_MINOR}.${LIBGIT2_VERSION_PATCH}")
    endif()

    message(STATUS "Found libgit2: ${LIBGIT2_LIBRARY} (include: ${LIBGIT2_INCLUDE_DIR}, version: ${LIBGIT2_VERSION})")
else()
    message(STATUS "Could not find libgit2")
endif()

# Provide variables for consumers of this module
mark_as_advanced(LIBGIT2_INCLUDE_DIR LIBGIT2_LIBRARY)
