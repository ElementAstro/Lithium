# CMakeLists.txt for Atom-Secret
# This project is licensed under the terms of the GPL3 license.
#
# Project Name: Atom-Secret
# Description: Secret Management Library for Atom
# Author: Max Qian
# License: GPL3

cmake_minimum_required(VERSION 3.20)
project(atom-secret C CXX)

# Version Management
set(ATOM_SECRET_VERSION_MAJOR 1)
set(ATOM_SECRET_VERSION_MINOR 0)
set(ATOM_SECRET_VERSION_PATCH 0)

set(ATOM_SECRET_SOVERSION ${ATOM_SECRET_VERSION_MAJOR})
set(ATOM_SECRET_VERSION_STRING "${ATOM_SECRET_VERSION_MAJOR}.${ATOM_SECRET_VERSION_MINOR}.${ATOM_SECRET_VERSION_PATCH}")

# Sources and Headers
set(ATOM_SECRET_SOURCES
    password.cpp
)

set(ATOM_SECRET_HEADERS
    password.hpp
)

set(ATOM_SECRET_LIBS
    loguru
    ${CMAKE_THREAD_LIBS_INIT}
)

# Build Object Library
add_library(atom-secret-object OBJECT)
set_property(TARGET atom-secret-object PROPERTY POSITION_INDEPENDENT_CODE 1)

target_sources(atom-secret-object
    PUBLIC
    ${ATOM_SECRET_HEADERS}
    PRIVATE
    ${ATOM_SECRET_SOURCES}
)

add_library(atom-secret STATIC)

target_link_libraries(atom-secret atom-secret-object ${ATOM_SECRET_LIBS})
if (LINUX)
    find_package(Glib REQUIRED)
    find_package(LibSecret REQUIRED)
    target_link_libraries(atom-secret ${GLIB_LIBRARIES} ${LIBSECRET_LIBRARIES})
    target_include_directories(atom-secret PUBLIC ${GLIB_INCLUDE_DIRS} ${LIBSECRET_INCLUDE_DIRS})
    include_directories(${GLIB_INCLUDE_DIRS} ${LIBSECRET_INCLUDE_DIRS})
endif()
target_include_directories(atom-secret PUBLIC .)

# Set library properties
set_target_properties(atom-secret PROPERTIES
    VERSION ${ATOM_SECRET_VERSION_STRING}
    SOVERSION ${ATOM_SECRET_SOVERSION}
    OUTPUT_NAME ${PROJECT_NAME}
)

# Installation
install(TARGETS atom-secret
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION include/atom-secret
)
