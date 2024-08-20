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
    io.cpp
)

# Headers
set(${PROJECT_NAME}_HEADERS
    compress.hpp
    glob.hpp
    io.hpp
)

set(${PROJECT_NAME}_LIBS
    loguru
    ZLIB::ZLIB
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
