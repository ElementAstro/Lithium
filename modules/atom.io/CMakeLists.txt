# CMakeLists.txt for Atom-IO-Builtin
# This project is licensed under the terms of the GPL3 license.
#
# Project Name: Atom-IO-Builtin
# Description: A builtin module for Atom-IO
# Author: Max Qian
# License: GPL3

cmake_minimum_required(VERSION 3.20)
project(atom.io C CXX)

set(CMAKE_ATOM_IO_BUILTIN_VERSION_MAJOR 1)
set(CMAKE_ATOM_IO_BUILTIN_VERSION_MINOR 0)
set(CMAKE_ATOM_IO_BUILTIN_VERSION_RELEASE 0)

set(ATOM_IO_BUILTIN_SOVERSION ${CMAKE_ATOM_IO_BUILTIN_VERSION_MAJOR})
set(CMAKE_ATOM_IO_BUILTIN_VERSION_STRING "${CMAKE_ATOM_IO_BUILTIN_VERSION_MAJOR}.${CMAKE_ATOM_IO_BUILTIN_VERSION_MINOR}.${CMAKE_ATOM_IO_BUILTIN_VERSION_RELEASE}")
set(ATOM_IO_BUILTIN_VERSION ${CMAKE_ATOM_IO_BUILTIN_VERSION_MAJOR}.${CMAKE_ATOM_IO_BUILTIN_VERSION_MINOR}.${CMAKE_ATOM_IO_BUILTIN_VERSION_RELEASE})

# Sources
set(${PROJECT_NAME}_SOURCES
    component.cpp
)

set(${PROJECT_NAME}_LIBS
    loguru
    atom-component
    atom-error
    atom-io
    ${ZLIB_LIBRARIES}
    ${CMAKE_THREAD_LIBS_INIT}
)

# Build Object Library
add_library(${PROJECT_NAME}_OBJECT OBJECT)
set_property(TARGET ${PROJECT_NAME}_OBJECT PROPERTY POSITION_INDEPENDENT_CODE 1)

target_sources(${PROJECT_NAME}_OBJECT
    PRIVATE
    ${${PROJECT_NAME}_SOURCES}
)

target_link_libraries(${PROJECT_NAME}_OBJECT ${${PROJECT_NAME}_LIBS})

add_library(${PROJECT_NAME} SHARED)

target_link_libraries(${PROJECT_NAME} ${PROJECT_NAME}_OBJECT ${${PROJECT_NAME}_LIBS})
target_include_directories(${PROJECT_NAME} PUBLIC .)

set_target_properties(${PROJECT_NAME} PROPERTIES
    VERSION ${CMAKE_ATOM_IO_BUILTIN_VERSION_STRING}
    SOVERSION ${ATOM_IO_BUILTIN_SOVERSION}
    OUTPUT_NAME atom.io
)

install(TARGETS ${PROJECT_NAME}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
