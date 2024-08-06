# CMakeLists.txt for Atom-Component
# This project adheres to the GPL3 license.
#
# Project Details:
#   Name: Atom-Component
#   Description: Central component library for the Atom framework
#   Author: Max Qian
#   License: GPL3

cmake_minimum_required(VERSION 3.20)
project(atom-component LANGUAGES C CXX)
# Source files with project-specific prefix
set(${PROJECT_NAME}_SOURCES
)

set(${PROJECT_NAME}_HEADERS
    component.hpp
    dispatch.hpp
    types.hpp
    var.hpp
)

# Dependencies
set(${PROJECT_NAME}_LIBS
    loguru
    atom-error
    atom-type
    atom-utils
)

# Include directories
include_directories(.)

# Object library for headers and sources with project prefix
add_library(${PROJECT_NAME}_OBJECT OBJECT ${${PROJECT_NAME}_HEADERS} ${${PROJECT_NAME}_SOURCES})
# set_target_properties(${PROJECT_NAME}_OBJECT PROPERTIES LINKER_LANGUAGE CXX)

# Static library target
add_library(${PROJECT_NAME} SHARED $<TARGET_OBJECTS:${PROJECT_NAME}_OBJECT>)

# Set project properties and definitions
# set_property(TARGET ${PROJECT_NAME} PROPERTY POSITION_INDEPENDENT_CODE ON)

# Link dependencies to the main target
target_link_libraries(${PROJECT_NAME} PRIVATE ${${PROJECT_NAME}_LIBS} ${CMAKE_THREAD_LIBS_INIT})

# Install rules
install(TARGETS ${PROJECT_NAME}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)