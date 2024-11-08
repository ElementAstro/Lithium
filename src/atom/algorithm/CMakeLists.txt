# CMakeLists.txt for Atom-Algorithm
# This project is licensed under the terms of the GPL3 license.
#
# Project Name: Atom-Algorithm
# Description: A collection of algorithms
# Author: Max Qian
# License: GPL3

cmake_minimum_required(VERSION 3.20)
project(atom-algorithm C CXX)

# Sources
set(${PROJECT_NAME}_SOURCES
    algorithm.cpp
    base.cpp
    bignumber.cpp
    convolve.cpp
    fnmatch.cpp
    fraction.cpp
    huffman.cpp
    math.cpp
    matrix_compress.cpp
    md5.cpp
    mhash.cpp
    tea.cpp
)

# Headers
set(${PROJECT_NAME}_HEADERS
    algorithm.hpp
    base.hpp
    bignumber.hpp
    convolve.hpp
    fnmatch.hpp
    fraction.hpp
    hash.hpp
    huffman.hpp
    math.hpp
    matrix_compress.hpp
    md5.hpp
    mhash.hpp
    tea.hpp
)

# Build Object Library
add_library(${PROJECT_NAME}_OBJECT OBJECT)
set_property(TARGET ${PROJECT_NAME}_OBJECT PROPERTY POSITION_INDEPENDENT_CODE 1)

target_sources(${PROJECT_NAME}_OBJECT
    PUBLIC
    ${${PROJECT_NAME}_HEADERS}
    PRIVATE
    ${${PROJECT_NAME}_SOURCES}
)

target_link_libraries(${PROJECT_NAME}_OBJECT ${${PROJECT_NAME}_LIBS})

add_library(${PROJECT_NAME} STATIC)

target_link_libraries(${PROJECT_NAME} ${PROJECT_NAME}_OBJECT ${${PROJECT_NAME}_LIBS})
target_link_libraries(${PROJECT_NAME} ${CMAKE_THREAD_LIBS_INIT})
target_include_directories(${PROJECT_NAME} PUBLIC .)

set_target_properties(${PROJECT_NAME} PROPERTIES
    VERSION ${CMAKE_HYDROGEN_VERSION_STRING}
    SOVERSION ${HYDROGEN_SOVERSION}
    OUTPUT_NAME ${PROJECT_NAME}
)

install(TARGETS ${PROJECT_NAME}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

if (ATOM_BUILD_PYTHON)
pybind11_add_module(${PROJECT_NAME}-py _pybind.cpp)
target_link_libraries(${PROJECT_NAME}-py PRIVATE ${PROJECT_NAME})
endif()
