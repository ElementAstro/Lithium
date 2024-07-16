# CMakeLists.txt for Atom-Function
# This project is licensed under the terms of the GPL3 license.
#
# Project Name: Atom-Function
# Description: a library for meta programming in C++
# Author: Max Qian
# License: GPL3

cmake_minimum_required(VERSION 3.20)
project(atom-function C CXX)

list(APPEND ${PROJECT_NAME}_SOURCES
    global_ptr.cpp
)

# Headers
list(APPEND ${PROJECT_NAME}_HEADERS
    global_ptr.hpp
)

list(APPEND ${PROJECT_NAME}_LIBS
)

# Build Object Library
add_library(${PROJECT_NAME}_OBJECT OBJECT)
set_property(TARGET ${PROJECT_NAME}_OBJECT PROPERTY POSITION_INDEPENDENT_CODE 1)

target_link_libraries(${PROJECT_NAME}_OBJECT ${${PROJECT_NAME}_LIBS})

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

install(TARGETS ${PROJECT_NAME}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
