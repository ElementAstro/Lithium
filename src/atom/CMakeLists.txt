# CMakeLists.txt for Atom
# This project is licensed under the terms of the GPL3 license.
#
# Project Name: Atom
# Description: Atom Library for all of the Element Astro Project
# Author: Max Qian
# License: GPL3

cmake_minimum_required(VERSION 3.20)
project(atom C CXX)

CHECK_INCLUDE_FILE(format HAS_STD_FORMAT)

set(CMAKE_ATOM_VERSION_MAJOR 1)
set(CMAKE_ATOM_VERSION_MINOR 0)
set(CMAKE_ATOM_VERSION_RELEASE 0)

set(ATOM_SOVERSION ${CMAKE_ATOM_VERSION_MAJOR})
set(CMAKE_ATOM_VERSION_STRING "${CMAKE_ATOM_VERSION_MAJOR}.${CMAKE_ATOM_VERSION_MINOR}.${CMAKE_ATOM_VERSION_RELEASE}")
set(ATOM_VERSION ${CMAKE_ATOM_VERSION_MAJOR}.${CMAKE_ATOM_VERSION_MINOR}.${CMAKE_ATOM_VERSION_RELEASE})

option(ATOM_BUILD_PYTHON " Build Atom with Python support" OFF)
find_package(Python COMPONENTS Interpreter Development REQUIRED)
if(${PYTHON_FOUND})
    message("-- Found Python ${PYTHON_VERSION_STRING}: ${PYTHON_EXECUTABLE}")
    find_package(pybind11)
    if (pybind11_FOUND)
        message(STATUS "Found pybind11: ${pybind11_INCLUDE_DIRS}")
        set(ATOM_BUILD_PYTHON ON)
    endif()
endif()
add_subdirectory(algorithm)
add_subdirectory(async)
add_subdirectory(components)
add_subdirectory(connection)
add_subdirectory(error)
add_subdirectory(io)
add_subdirectory(log)
add_subdirectory(server)
add_subdirectory(search)
add_subdirectory(system)
add_subdirectory(task)
add_subdirectory(type)
add_subdirectory(utils)
add_subdirectory(web)

if(NOT HAS_STD_FORMAT)
    find_package(fmt REQUIRED)
endif()
# Sources
list(APPEND ${PROJECT_NAME}_SOURCES
    log/logger.cpp
    log/global_logger.cpp
    log/syslog.cpp
)

# Headers
list(APPEND ${PROJECT_NAME}_HEADERS
    log/logger.hpp
    log/global_logger.hpp
    log/syslog.hpp
)

# Private Headers
list(APPEND ${PROJECT_NAME}_PRIVATE_HEADERS

)

list(APPEND ${PROJECT_NAME}_LIBS
    loguru
    cpp_httplib
    libzippp
    atom-algorithm
    #atom-connection
    atom-async
    atom-task
    atom-io
    atom-component
    atom-type
    atom-utils
    atom-search
    atom-web
    atom-system
    atom-server
    )

# Build Object Library
add_library(${PROJECT_NAME}_OBJECT OBJECT)
set_property(TARGET ${PROJECT_NAME}_OBJECT PROPERTY POSITION_INDEPENDENT_CODE 1)

target_compile_definitions(${PROJECT_NAME}_OBJECT PRIVATE "-DHAVE_LIBNOVA")

if(WIN32)
target_link_libraries(${PROJECT_NAME}_OBJECT setupapi wsock32 ws2_32 shlwapi iphlpapi)
endif()
target_link_libraries(${PROJECT_NAME}_OBJECT ${${PROJECT_NAME}_LIBS})

target_sources(${PROJECT_NAME}_OBJECT
    PUBLIC
    ${${PROJECT_NAME}_HEADERS}
    PRIVATE
    ${${PROJECT_NAME}_SOURCES}
    ${${PROJECT_NAME}_PRIVATE_HEADERS}
)

target_link_libraries(${PROJECT_NAME}_OBJECT ${${PROJECT_NAME}_LIBS})

add_library(${PROJECT_NAME}static STATIC)

target_link_libraries(${PROJECT_NAME}static ${PROJECT_NAME}_OBJECT ${${PROJECT_NAME}_LIBS})
target_link_libraries(${PROJECT_NAME}static ${CMAKE_THREAD_LIBS_INIT})
target_include_directories(${PROJECT_NAME}static PUBLIC .)

set_target_properties(${PROJECT_NAME}static PROPERTIES
    VERSION ${CMAKE_HYDROGEN_VERSION_STRING}
    SOVERSION ${HYDROGEN_SOVERSION}
    OUTPUT_NAME ${PROJECT_NAME} # this same name like shared library - backwards compatibility
)

install(TARGETS ${PROJECT_NAME}static
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)