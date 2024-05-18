# CMakeLists.txt for Atom-System
# This project is licensed under the terms of the GPL3 license.
#
# Project Name: Atom-System
# Description: A collection of useful system functions
# Author: Max Qian
# License: GPL3

cmake_minimum_required(VERSION 3.20)
project(atom-system C CXX)

set(CMAKE_ATOM_SYSTEM_VERSION_MAJOR 1)
set(CMAKE_ATOM_SYSTEM_VERSION_MINOR 0)
set(CMAKE_ATOM_SYSTEM_VERSION_RELEASE 0)

set(ATOM_SYSTEM_SOVERSION ${CMAKE_ATOM_SYSTEM_VERSION_MAJOR})
set(CMAKE_ATOM_SYSTEM_VERSION_STRING "${CMAKE_ATOM_SYSTEM_VERSION_MAJOR}.${CMAKE_ATOM_SYSTEM_VERSION_MINOR}.${CMAKE_ATOM_SYSTEM_VERSION_RELEASE}")
set(ATOM_SYSTEM_VERSION ${CMAKE_ATOM_SYSTEM_VERSION_MAJOR}.${CMAKE_ATOM_SYSTEM_VERSION_MINOR}.${CMAKE_ATOM_SYSTEM_VERSION_RELEASE})

# Sources
set(${PROJECT_NAME}_SOURCES
    command.cpp
    crash_quotes.cpp
    crash.cpp
    env.cpp
    lregistry.cpp
    os.cpp
    pidwatcher.cpp
    platform.cpp
    process.cpp
    register.cpp
    storage.cpp
    system.cpp
    user.cpp
)

# Headers
set(${PROJECT_NAME}_HEADERS
    command.hpp
    crash_quotes.hpp
    crash.hpp
    env.hpp
    os.hpp
    pidwatcher.hpp
    platform.hpp
    process.hpp
    register.hpp
    storage.hpp
    system.hpp
    user.hpp
)

# Private Headers
set(${PROJECT_NAME}_PRIVATE_HEADERS
)

# Build Object Library
add_library(${PROJECT_NAME}_OBJECT OBJECT)
set_property(TARGET ${PROJECT_NAME}_OBJECT PROPERTY POSITION_INDEPENDENT_CODE 1)

target_link_libraries(${PROJECT_NAME}_OBJECT loguru)

target_sources(${PROJECT_NAME}_OBJECT
    PUBLIC
    ${${PROJECT_NAME}_HEADERS}
    PRIVATE
    ${${PROJECT_NAME}_SOURCES}
    ${${PROJECT_NAME}_PRIVATE_HEADERS}
)

target_link_libraries(${PROJECT_NAME}_OBJECT ${${PROJECT_NAME}_LIBS})

add_library(${PROJECT_NAME} STATIC)

if(WIN32)
target_link_libraries(${PROJECT_NAME} pdh wlanapi)
endif()

target_link_libraries(${PROJECT_NAME} ${PROJECT_NAME}_OBJECT ${${PROJECT_NAME}_LIBS})
target_link_libraries(${PROJECT_NAME} ${CMAKE_THREAD_LIBS_INIT})
target_include_directories(${PROJECT_NAME} PUBLIC .)

set_target_properties(${PROJECT_NAME} PROPERTIES
    VERSION ${CMAKE_HYDROGEN_VERSION_STRING}
    SOVERSION ${HYDROGEN_SOVERSION}
    OUTPUT_NAME ${PROJECT_NAME}
)

install(TARGETS ${PROJECT_NAME}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
