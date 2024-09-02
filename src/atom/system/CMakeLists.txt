# CMakeLists.txt for Atom-System
# This project is licensed under the terms of the GPL3 license.
#
# Project Name: Atom-System
# Description: A collection of useful system functions
# Author: Max Qian
# License: GPL3

cmake_minimum_required(VERSION 3.20)
project(atom-system C CXX)

# Version Management
set(ATOM_SYSTEM_VERSION_MAJOR 1)
set(ATOM_SYSTEM_VERSION_MINOR 0)
set(ATOM_SYSTEM_VERSION_PATCH 0)

set(ATOM_SYSTEM_SOVERSION ${ATOM_SYSTEM_VERSION_MAJOR})
set(ATOM_SYSTEM_VERSION_STRING "${ATOM_SYSTEM_VERSION_MAJOR}.${ATOM_SYSTEM_VERSION_MINOR}.${ATOM_SYSTEM_VERSION_PATCH}")

# Sources and Headers
set(ATOM_SYSTEM_SOURCES
    command.cpp
    crash_quotes.cpp
    crash.cpp
    env.cpp
    lregistry.cpp
    pidwatcher.cpp
    power.cpp
    process.cpp
    software.cpp
    storage.cpp
    user.cpp
    wregistry.cpp
)

set(ATOM_SYSTEM_HEADERS
    command.hpp
    crash_quotes.hpp
    crash.hpp
    env.hpp
    pidwatcher.hpp
    platform.hpp
    power.hpp
    process.hpp
    software.hpp
    storage.hpp
    user.hpp
    wregistry.hpp
)

set(ATOM_SYSTEM_LIBS
    loguru
    ${CMAKE_THREAD_LIBS_INIT}
    atom-sysinfo
)

# Build Object Library
add_library(atom-system-object OBJECT)
set_property(TARGET atom-system-object PROPERTY POSITION_INDEPENDENT_CODE 1)

target_sources(atom-system-object
    PUBLIC
    ${ATOM_SYSTEM_HEADERS}
    PRIVATE
    ${ATOM_SYSTEM_SOURCES}
)

add_library(atom-system STATIC)

# Platform-specific libraries
if(WIN32)
    target_link_libraries(atom-system pdh wlanapi userenv)
endif()

target_link_libraries(atom-system atom-system-object ${ATOM_SYSTEM_LIBS})
target_include_directories(atom-system PUBLIC .)

# Set library properties
set_target_properties(atom-system PROPERTIES
    VERSION ${ATOM_SYSTEM_VERSION_STRING}
    SOVERSION ${ATOM_SYSTEM_SOVERSION}
    OUTPUT_NAME ${PROJECT_NAME}
)

# Installation
install(TARGETS atom-system
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION include/atom-system
)
