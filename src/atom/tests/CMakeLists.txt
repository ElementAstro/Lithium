# CMakeLists.txt for Atom-Tests
# This project is licensed under the terms of the GPL3 license.
#
# Project Name: Atom-Tests
# Description: Test Suite for Atom
# Author: Max Qian
# License: GPL3

cmake_minimum_required(VERSION 3.20)
project(atom-tests C CXX)

# Version Management
set(ATOM_TESTS_VERSION_MAJOR 1)
set(ATOM_TESTS_VERSION_MINOR 0)
set(ATOM_TESTS_VERSION_PATCH 0)

set(ATOM_TESTS_SOVERSION ${ATOM_TESTS_VERSION_MAJOR})
set(ATOM_TESTS_VERSION_STRING "${ATOM_TESTS_VERSION_MAJOR}.${ATOM_TESTS_VERSION_MINOR}.${ATOM_TESTS_VERSION_PATCH}")

# Sources and Headers
set(ATOM_TESTS_SOURCES
    benchmark.cpp
    fuzz.cpp
)

set(ATOM_TESTS_HEADERS
    benchmark.hpp
    fuzz.hpp
    test.hpp
)

set(ATOM_TESTS_LIBS
    loguru
    ${CMAKE_THREAD_LIBS_INIT}
)

# Build Object Library
add_library(atom-tests-object OBJECT)
set_property(TARGET atom-tests-object PROPERTY POSITION_INDEPENDENT_CODE 1)

target_sources(atom-tests-object
    PUBLIC
    ${ATOM_TESTS_HEADERS}
    PRIVATE
    ${ATOM_TESTS_SOURCES}
)

add_library(atom-tests STATIC)

# Platform-specific libraries
if(WIN32)
    target_link_libraries(atom-tests pdh wlanapi)
endif()

target_link_libraries(atom-tests atom-tests-object ${ATOM_TESTS_LIBS})
target_include_directories(atom-tests PUBLIC .)

# Set library properties
set_target_properties(atom-tests PROPERTIES
    VERSION ${ATOM_TESTS_VERSION_STRING}
    SOVERSION ${ATOM_TESTS_SOVERSION}
    OUTPUT_NAME ${PROJECT_NAME}
)

# Installation
install(TARGETS atom-tests
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION include/atom-tests
)
