# CMakeLists.txt for Atom-System
# This project is licensed under the terms of the GPL3 license.
#
# Project Name: Atom-System
# Description: A collection of useful system functions
# Author: Max Qian
# License: GPL3

cmake_minimum_required(VERSION 3.20)
project(lithium-device-template C CXX)

# Version Management
set(LITHIUM_DEVICE_TEMPLATE_VERSION_MAJOR 1)
set(LITHIUM_DEVICE_TEMPLATE_VERSION_MINOR 0)
set(LITHIUM_DEVICE_TEMPLATE_VERSION_PATCH 0)

set(LITHIUM_DEVICE_TEMPLATE_SOVERSION ${LITHIUM_DEVICE_TEMPLATE_VERSION_MAJOR})
set(LITHIUM_DEVICE_TEMPLATE_VERSION_STRING "${LITHIUM_DEVICE_TEMPLATE_VERSION_MAJOR}.${LITHIUM_DEVICE_TEMPLATE_VERSION_MINOR}.${LITHIUM_DEVICE_TEMPLATE_VERSION_PATCH}")

# Sources and Headers
set(LITHIUM_DEVICE_TEMPLATE_SOURCES
    camera.cpp
    device.cpp
)

set(LITHIUM_DEVICE_TEMPLATE_HEADERS
    camera.hpp
    device.hpp
)

set(LITHIUM_DEVICE_TEMPLATE_LIBS
    loguru
    ${CMAKE_THREAD_LIBS_INIT}
)

# Build Object Library
add_library(lithium-device-template-object OBJECT)
set_property(TARGET lithium-device-template-object PROPERTY POSITION_INDEPENDENT_CODE 1)

target_sources(lithium-device-template-object
    PUBLIC
    ${LITHIUM_DEVICE_TEMPLATE_HEADERS}
    PRIVATE
    ${LITHIUM_DEVICE_TEMPLATE_SOURCES}
)

add_library(lithium-device-template STATIC)

target_link_libraries(lithium-device-template lithium-device-template-object ${LITHIUM_DEVICE_TEMPLATE_LIBS})
target_include_directories(lithium-device-template PUBLIC .)

# Set library properties
set_target_properties(lithium-device-template PROPERTIES
    VERSION ${LITHIUM_DEVICE_TEMPLATE_VERSION_STRING}
    SOVERSION ${LITHIUM_DEVICE_TEMPLATE_SOVERSION}
    OUTPUT_NAME ${PROJECT_NAME}
)

# Installation
install(TARGETS lithium-device-template
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION include/lithium-device-template
)
