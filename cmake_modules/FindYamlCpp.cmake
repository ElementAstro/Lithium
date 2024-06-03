# - Try to find yaml-cpp
# Once done this will define
#
#  YAMLCPP_FOUND - system has yaml-cpp
#  YAMLCPP_INCLUDE_DIRS - the yaml-cpp include directory
#  YAMLCPP_LIBRARIES - Link these to use yaml-cpp
#  YAMLCPP_DEFINITIONS - Compiler switches required for using yaml-cpp
#
# Copyright (c) 2006, Alexander Neundorf, <neundorf@kde.org>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

find_package(PkgConfig)
pkg_check_modules(PC_YAMLCPP QUIET yaml-cpp)

set(YAMLCPP_DEFINITIONS ${PC_YAMLCPP_CFLAGS_OTHER})

find_path(YAMLCPP_INCLUDE_DIR
          NAMES yaml-cpp/yaml.h
          HINTS ${PC_YAMLCPP_INCLUDEDIR} ${PC_YAMLCPP_INCLUDE_DIRS}
          PATH_SUFFIXES yaml-cpp)

find_library(YAMLCPP_LIBRARY
             NAMES yaml-cpp
             HINTS ${PC_YAMLCPP_LIBDIR} ${PC_YAMLCPP_LIBRARY_DIRS})

set(YAMLCPP_LIBRARIES ${YAMLCPP_LIBRARY})
set(YAMLCPP_INCLUDE_DIRS ${YAMLCPP_INCLUDE_DIR})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(yaml-cpp
                                  REQUIRED_VARS YAMLCPP_LIBRARY YAMLCPP_INCLUDE_DIR
                                  VERSION_VAR YAMLCPP_VERSION)

mark_as_advanced(YAMLCPP_INCLUDE_DIR YAMLCPP_LIBRARY)