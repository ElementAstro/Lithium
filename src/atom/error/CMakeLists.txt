# CMakeLists.txt for Atom-Error
# This project is licensed under the terms of the GPL3 license.
#
# Project Name: Atom-Error
# Description: Atom Error Library
# Author: Max Qian
# License: GPL3

cmake_minimum_required(VERSION 3.20)
project(atom-error C CXX)

list(APPEND ${PROJECT_NAME}_SOURCES
    error_stack.cpp
    exception.cpp
    stacktrace.cpp
)

# Headers
list(APPEND ${PROJECT_NAME}_HEADERS
    error_code.hpp
    error_stack.hpp
    stacktrace.hpp
)

list(APPEND ${PROJECT_NAME}_LIBS
    loguru
)

if (LINUX)
list (APPEND ${PROJECT_NAME}_LIBS
    dl
)
endif()

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