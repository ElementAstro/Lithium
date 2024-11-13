# CMakeLists.txt for Atom-IO
# This project is licensed under the terms of the GPL3 license.
#
# Project Name: Atom-IO
# Description: IO Components for Element Astro Project
# Author: Max Qian
# License: GPL3

cmake_minimum_required(VERSION 3.20)
project(atom-io C CXX)

# Sources
set(${PROJECT_NAME}_SOURCES
    compress.cpp
    file_permission.cpp
    io.cpp
    pushd.cpp
)

# Headers
set(${PROJECT_NAME}_HEADERS
    compress.hpp
    file_permission.hpp
    glob.hpp
    io.hpp
    pushd.hpp
)

set(${PROJECT_NAME}_LIBS
    loguru
    MINIZIP::minizip
    ZLIB::ZLIB
    ${CMAKE_THREAD_LIBS_INIT}
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
target_include_directories(${PROJECT_NAME} PUBLIC .)

set_target_properties(${PROJECT_NAME} PROPERTIES
    VERSION ${CMAKE_HYDROGEN_VERSION_STRING}
    SOVERSION ${HYDROGEN_SOVERSION}
    OUTPUT_NAME ${PROJECT_NAME}
)

install(TARGETS ${PROJECT_NAME}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

set(atom-async-io_SOURCES
    async_compress.cpp
    async_glob.cpp
    async_io.cpp
)

set(atom-async-io_HEADERS
    async_compress.hpp
    async_glob.hpp
    async_io.hpp
)

set(atom-async-io_LIBS
    atom-io
    atom-async
    loguru
    ${CMAKE_THREAD_LIBS_INIT}
)

add_library(atom-async-io_OBJECT OBJECT)
set_property(TARGET atom-async-io_OBJECT PROPERTY POSITION_INDEPENDENT_CODE 1)

target_sources(atom-async-io_OBJECT
    PUBLIC
    ${atom-async-io_HEADERS}
    PRIVATE
    ${atom-async-io_SOURCES}
)

target_link_libraries(atom-async-io_OBJECT ${atom-async-io_LIBS})

add_library(atom-async-io STATIC)

target_link_libraries(atom-async-io atom-async-io_OBJECT ${atom-async-io_LIBS})

target_include_directories(atom-async-io PUBLIC .)

set_target_properties(atom-async-io PROPERTIES
    VERSION ${CMAKE_HYDROGEN_VERSION_STRING}
    SOVERSION ${HYDROGEN_SOVERSION}
    OUTPUT_NAME atom-async-io
)
