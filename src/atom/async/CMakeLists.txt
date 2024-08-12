# CMakeLists.txt for Atom-Async
# This project is licensed under the terms of the GPL3 license.
#
# Project Name: Atom-Async
# Description: Async Implementation of Lithium Server and Driver
# Author: Max Qian
# License: GPL3

cmake_minimum_required(VERSION 3.20)
project(atom-async C CXX)

# Sources
set(${PROJECT_NAME}_SOURCES
    daemon.cpp
    limiter.cpp
    lock.cpp
    timer.cpp
)

# Headers
set(${PROJECT_NAME}_HEADERS
    async.hpp
    daemon.hpp
    eventstack.hpp
    limiter.hpp
    lock.hpp
    message_bus.hpp
    message_queue.hpp
    pool.hpp
    queue.hpp
    safetype.hpp
    thread_wrapper.hpp
    timer.hpp
    trigger.hpp
)

set(${PROJECT_NAME}_LIBS
    loguru
    ${CMAKE_THREAD_LIBS_INIT}
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
target_include_directories(${PROJECT_NAME} PUBLIC .)

set_target_properties(${PROJECT_NAME} PROPERTIES
    VERSION ${CMAKE_HYDROGEN_VERSION_STRING}
    SOVERSION ${HYDROGEN_SOVERSION}
    OUTPUT_NAME ${PROJECT_NAME}
)

install(TARGETS ${PROJECT_NAME}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

if (ATOM_BUILD_PYTHON)
pybind11_add_module(${PROJECT_NAME}-py _pybind.cpp)
target_link_libraries(${PROJECT_NAME}-py PRIVATE ${PROJECT_NAME})
endif()
