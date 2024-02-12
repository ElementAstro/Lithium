# CMakeLists.txt for Atom-Server
# This project is licensed under the terms of the GPL3 license.
#
# Project Name: Atom-Server
# Description: Core Server Components for Element Astro Projects
# Author: Max Qian
# License: GPL3

cmake_minimum_required(VERSION 3.20)
project(atom-server C CXX)

# Sources
set(${PROJECT_NAME}_SOURCES
    daemon.cpp
    deserialize.cpp
    global_ptr.cpp
    json_checker.cpp
    serialize.cpp
    variables.cpp
)

# Headers
set(${PROJECT_NAME}_HEADERS
    commander_impl.hpp
    commander.hpp
    daemon.hpp
    deserialize.hpp
    global_ptr.hpp
    json_checker.hpp
    message_bus.hpp
    message_queue.hpp
    serialize.hpp
    variables.hpp
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
