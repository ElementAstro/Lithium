# CMakeLists.txt for Lithium-Task
# This project is licensed under the terms of the GPL3 license.
#
# Project Name: Lithium-Task
# Description: The official config module for lithium server
# Author: Max Qian
# License: GPL3

cmake_minimum_required(VERSION 3.20)
project(lithium-task VERSION 1.0.0 LANGUAGES C CXX)

# Sources
set(PROJECT_SOURCES
    container.cpp
    generator.cpp
    loader.cpp
    manager.cpp
    singlepool.cpp
    task.cpp
)

# Headers
set(PROJECT_HEADERS
    container.hpp
    generator.hpp
    loader.hpp
    manager.hpp
    singlepool.hpp
    task.hpp
)

# Required libraries
set(PROJECT_LIBS
    atom-component
    atom-function
    atom-utils
    atom-error
    loguru
    ${CMAKE_THREAD_LIBS_INIT}
)

# Create Object Library
add_library(${PROJECT_NAME}_OBJECT OBJECT ${PROJECT_SOURCES} ${PROJECT_HEADERS})
set_property(TARGET ${PROJECT_NAME}_OBJECT PROPERTY POSITION_INDEPENDENT_CODE ON)

# Create Static Library
add_library(${PROJECT_NAME} STATIC $<TARGET_OBJECTS:${PROJECT_NAME}_OBJECT>)

# Include directories
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(${PROJECT_NAME} PRIVATE ${PROJECT_LIBS})

# Set version properties
set_target_properties(${PROJECT_NAME} PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION 1
    OUTPUT_NAME ${PROJECT_NAME}
)

# Install target
install(TARGETS ${PROJECT_NAME}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
