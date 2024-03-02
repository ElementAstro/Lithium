# CMakeLists.txt for Atom-Search
# This project is licensed under the terms of the GPL3 license.
#
# Project Name: Atom-Utils
# Description: Search Engine for Element Astro Project
# Author: Max Qian
# License: GPL3

cmake_minimum_required(VERSION 3.20)
project(atom-search C CXX)

# Sources
set(${PROJECT_NAME}_SOURCES
    search.cpp
    sqlite.cpp
)

# Headers
set(${PROJECT_NAME}_HEADERS
    cache_impl.hpp
    cache.hpp
    search.hpp
    sqlite.hpp
)

# Private Headers
set(${PROJECT_NAME}_PRIVATE_HEADERS
)

# Build Object Library
add_library(${PROJECT_NAME}_OBJECT OBJECT)
set_property(TARGET ${PROJECT_NAME}_OBJECT PROPERTY POSITION_INDEPENDENT_CODE 1)

target_link_libraries(${PROJECT_NAME}_OBJECT loguru)

target_sources(${PROJECT_NAME}_OBJECT
    PUBLIC
    ${${PROJECT_NAME}_HEADERS}
    PRIVATE
    ${${PROJECT_NAME}_SOURCES}
    ${${PROJECT_NAME}_PRIVATE_HEADERS}
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