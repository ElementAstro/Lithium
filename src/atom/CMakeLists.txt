# CMakeLists.txt for Atom
# This project is licensed under the terms of the GPL3 license.
#
# Project Name: Atom
# Description: Atom Library for all of the Element Astro Project
# Author: Max Qian
# License: GPL3

cmake_minimum_required(VERSION 3.20)
project(atom C CXX)

# Versioning
set(ATOM_VERSION_MAJOR 1)
set(ATOM_VERSION_MINOR 0)
set(ATOM_VERSION_PATCH 0)
set(ATOM_SOVERSION ${ATOM_VERSION_MAJOR})
set(ATOM_VERSION_STRING "${ATOM_VERSION_MAJOR}.${ATOM_VERSION_MINOR}.${ATOM_VERSION_PATCH}")

# Python Support
option(ATOM_BUILD_PYTHON "Build Atom with Python support" OFF)
if(ATOM_BUILD_PYTHON)
    find_package(Python COMPONENTS Interpreter Development REQUIRED)
    if(PYTHON_FOUND)
        message("-- Found Python ${PYTHON_VERSION_STRING}: ${PYTHON_EXECUTABLE}")
        find_package(pybind11 QUIET)
        if(pybind11_FOUND)
            message(STATUS "Found pybind11: ${pybind11_INCLUDE_DIRS}")
        else()
            message(FATAL_ERROR "pybind11 not found")
        endif()
    else()
        message(FATAL_ERROR "Python not found")
    endif()
endif()

# Subdirectories
add_subdirectory(algorithm)
add_subdirectory(async)
add_subdirectory(components)
add_subdirectory(connection)
add_subdirectory(error)
add_subdirectory(function)
add_subdirectory(io)
add_subdirectory(log)
add_subdirectory(search)
add_subdirectory(sysinfo)
add_subdirectory(system)
add_subdirectory(type)
add_subdirectory(utils)
add_subdirectory(web)

# Sources and Headers
set(ATOM_SOURCES
    log/logger.cpp
    log/syslog.cpp
)

set(ATOM_HEADERS
    log/logger.hpp
    log/syslog.hpp
)

# Libraries
set(ATOM_LIBS
    loguru
    cpp_httplib
    atom-function
    atom-algorithm
    atom-async
    atom-io
    atom-component
    atom-type
    atom-utils
    atom-search
    atom-web
    atom-system
    atom-sysinfo
)

# Object Library
add_library(atom_object OBJECT ${ATOM_SOURCES} ${ATOM_HEADERS})

if(WIN32)
    target_link_libraries(atom_object setupapi wsock32 ws2_32 shlwapi iphlpapi)
endif()

target_link_libraries(atom_object ${ATOM_LIBS})

# Static Library
add_library(atom STATIC)
set_target_properties(atom PROPERTIES
    IMPORTED_LOCATION "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_STATIC_LIBRARY_PREFIX}atom${CMAKE_STATIC_LIBRARY_SUFFIX}"
    VERSION ${ATOM_VERSION_STRING}
    SOVERSION ${ATOM_SOVERSION}
)

target_link_libraries(atom atom_object ${CMAKE_THREAD_LIBS_INIT} ${ATOM_LIBS})

# Install
install(TARGETS atom
    DESTINATION ${CMAKE_INSTALL_LIBDIR}
    COMPONENT library
)