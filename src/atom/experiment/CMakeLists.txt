# CMakeLists.txt for Atom-Experiment
# This project is licensed under the terms of the GPL3 license.
#
# Project Name: Atom-Experiment
# Description: A collection of experiments for the Atom project
# Author: Max Qian
# License: GPL3

cmake_minimum_required(VERSION 3.20)
project(atom-experiment C CXX)

# Sources
list(APPEND ${PROJECT_NAME}_SOURCES
    platform.cpp
    string.cpp
)

# Headers
list(APPEND ${PROJECT_NAME}_HEADERS
    any.hpp
    anyutils.hpp
    bind_first.hpp
    callable.hpp
    decorate.hpp
    flatmap.hpp
    func_traits.hpp
    invoke.hpp
    list.hpp
    memory.hpp
    noncopyable.hpp
    object.hpp
    optional.hpp
    platform.hpp
    ranges.hpp
    short_alloc.hpp
    stack_vector.hpp
    static_vector.hpp
    string.hpp
    to_string.hpp
    type_info.hpp
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