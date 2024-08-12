# CMakeLists.txt for Atom-Sysinfo
# This project is licensed under the terms of the GPL3 license.
#
# Project Name: Atom-Sysinfo
# Description: System Information Library for Atom
# Author: Max Qian
# License: GPL3

cmake_minimum_required(VERSION 3.20)
project(atom-sysinfo C CXX)

# Version Management
set(ATOM_SYSINFO_VERSION_MAJOR 1)
set(ATOM_SYSINFO_VERSION_MINOR 0)
set(ATOM_SYSINFO_VERSION_PATCH 0)

set(ATOM_SYSINFO_SOVERSION ${ATOM_SYSINFO_VERSION_MAJOR})
set(ATOM_SYSINFO_VERSION_STRING "${ATOM_SYSINFO_VERSION_MAJOR}.${ATOM_SYSINFO_VERSION_MINOR}.${ATOM_SYSINFO_VERSION_PATCH}")

# Sources and Headers
set(ATOM_SYSINFO_SOURCES
    battery.cpp
    cpu.cpp
    disk.cpp
    gpu.cpp
    memory.cpp
    os.cpp
    wifi.cpp
)

set(ATOM_SYSINFO_HEADERS
    battery.hpp
    cpu.hpp
    disk.hpp
    gpu.hpp
    memory.hpp
    os.hpp
    wifi.hpp
)

set(ATOM_SYSINFO_LIBS
    loguru
    ${CMAKE_THREAD_LIBS_INIT}
)

# Build Object Library
add_library(atom-sysinfo-object OBJECT)
set_property(TARGET atom-sysinfo-object PROPERTY POSITION_INDEPENDENT_CODE 1)

target_sources(atom-sysinfo-object
    PUBLIC
    ${ATOM_SYSINFO_HEADERS}
    PRIVATE
    ${ATOM_SYSINFO_SOURCES}
)

add_library(atom-sysinfo STATIC)

# Platform-specific libraries
if(WIN32)
    target_link_libraries(atom-sysinfo pdh wlanapi)
endif()

target_link_libraries(atom-sysinfo atom-sysinfo-object ${ATOM_SYSINFO_LIBS})
target_include_directories(atom-sysinfo PUBLIC .)

# Set library properties
set_target_properties(atom-sysinfo PROPERTIES
    VERSION ${ATOM_SYSINFO_VERSION_STRING}
    SOVERSION ${ATOM_SYSINFO_SOVERSION}
    OUTPUT_NAME ${PROJECT_NAME}
)

# Installation
install(TARGETS atom-sysinfo
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION include/atom-sysinfo
)
