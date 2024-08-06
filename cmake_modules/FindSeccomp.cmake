# - Try to find libseccomp
# Once done, this will define
#
#  Seccomp_FOUND - system has libseccomp
#  Seccomp_INCLUDE_DIRS - the libseccomp include directories
#  Seccomp_LIBRARIES - link these to use libseccomp

find_path(Seccomp_INCLUDE_DIR
          NAMES seccomp.h
          PATH_SUFFIXES seccomp
          PATHS /usr/local/include /usr/include
)

find_library(Seccomp_LIBRARY
             NAMES seccomp
             PATHS /usr/local/lib /usr/lib
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Seccomp DEFAULT_MSG Seccomp_LIBRARY Seccomp_INCLUDE_DIR)

if(Seccomp_FOUND)
    set(Seccomp_LIBRARIES ${Seccomp_LIBRARY})
    set(Seccomp_INCLUDE_DIRS ${Seccomp_INCLUDE_DIR})
else()
    set(Seccomp_LIBRARIES "")
    set(Seccomp_INCLUDE_DIRS "")
endif()

mark_as_advanced(Seccomp_INCLUDE_DIR Seccomp_LIBRARY)
