# CMakeLists.txt for Atom-Component
# This project is licensed under the terms of the GPL3 license.
#
# Project Name: Atom-Component
# Description: The core component library of Atom
# Author: Max Qian
# License: GPL3

cmake_minimum_required(VERSION 3.20)
project(atom-component C CXX)


list(APPEND ${PROJECT_NAME}_SOURCES
    component.cpp
    dispatch.cpp
)

# Headers
list(APPEND ${PROJECT_NAME}_HEADERS
    component.hpp
    dispatch.hpp
    dispatch.inl
    types.hpp
    var.hpp
    var.inl
)

list(APPEND ${PROJECT_NAME}_LIBS
    loguru
    atom-error
    atom-type
    atom-utils
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
target_link_libraries(${PROJECT_NAME} atom-utils)
target_link_libraries(${PROJECT_NAME} atom-type)
target_include_directories(${PROJECT_NAME} PUBLIC .)

set_target_properties(${PROJECT_NAME} PROPERTIES
    VERSION ${CMAKE_HYDROGEN_VERSION_STRING}
    SOVERSION ${HYDROGEN_SOVERSION}
    OUTPUT_NAME ${PROJECT_NAME}
)

install(TARGETS ${PROJECT_NAME}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
