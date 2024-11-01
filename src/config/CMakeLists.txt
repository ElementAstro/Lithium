# Minimum required CMake version
cmake_minimum_required(VERSION 3.20)

# Project name and version, using C and C++ languages
project(lithium-config VERSION 1.0.0 LANGUAGES C CXX)

# Project description and information
# This project is the official configuration module for the Lithium server.
# Author: Max Qian
# License: GPL3
# Project Name: Lithium-Config
# Description: The official config module for lithium server
# Author: Max Qian
# License: GPL3

# Project sources
set(PROJECT_SOURCES
    configor.cpp
)

# Project headers
set(PROJECT_HEADERS
    configor.hpp
)

# Required libraries for the project
set(PROJECT_LIBS
    loguru
    ${CMAKE_THREAD_LIBS_INIT}
)

if (WIN32)
    list(APPEND PROJECT_LIBS ws2_32)
endif()

# Create object library
add_library(${PROJECT_NAME}_OBJECT OBJECT ${PROJECT_SOURCES} ${PROJECT_HEADERS})

# Set object library property to be position independent code
set_property(TARGET ${PROJECT_NAME}_OBJECT PROPERTY POSITION_INDEPENDENT_CODE ON)

# Create static library
add_library(${PROJECT_NAME} STATIC $<TARGET_OBJECTS:${PROJECT_NAME}_OBJECT>)

# Set static library properties
set_target_properties(${PROJECT_NAME} PROPERTIES
    VERSION ${PROJECT_VERSION}  # Version number
    SOVERSION 1                 # Compatibility version
    OUTPUT_NAME ${PROJECT_NAME} # Output name
)

# Include directories so that project headers can be included
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# Link libraries required by the project
target_link_libraries(${PROJECT_NAME} PRIVATE ${PROJECT_LIBS})

# Install target to install the static library to a specified location
install(TARGETS ${PROJECT_NAME}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
