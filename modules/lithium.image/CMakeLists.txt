# CMakeLists.txt for Lithium-Image-Builtin
# This project is licensed under the terms of the GPL3 license.
#
# Project Name: Lithium-Image-Builtin
# Description: A builtin module for Lithium-Image
# Author: Max Qian
# License: GPL3

cmake_minimum_required(VERSION 3.20)
project(lithium.image C CXX)

set(CMAKE_LITHIUM_IMAGE_BUILTIN_VERSION_MAJOR 1)
set(CMAKE_LITHIUM_IMAGE_BUILTIN_VERSION_MINOR 0)
set(CMAKE_LITHIUM_IMAGE_BUILTIN_VERSION_RELEASE 0)

set(LITHIUM_IMAGE_BUILTIN_SOVERSION ${CMAKE_LITHIUM_IMAGE_BUILTIN_VERSION_MAJOR})
set(CMAKE_LITHIUM_IMAGE_BUILTIN_VERSION_STRING "${CMAKE_LITHIUM_IMAGE_BUILTIN_VERSION_MAJOR}.${CMAKE_LITHIUM_IMAGE_BUILTIN_VERSION_MINOR}.${CMAKE_LITHIUM_IMAGE_BUILTIN_VERSION_RELEASE}")
set(LITHIUM_IMAGE_BUILTIN_VERSION ${CMAKE_LITHIUM_IMAGE_BUILTIN_VERSION_MAJOR}.${CMAKE_LITHIUM_IMAGE_BUILTIN_VERSION_MINOR}.${CMAKE_LITHIUM_IMAGE_BUILTIN_VERSION_RELEASE})

# Sources
set(${PROJECT_NAME}_SOURCES
    _main.cpp
    _component.cpp

    src/debayer.cpp
    src/fitsio.cpp
    src/fitskeyword.cpp
    src/hfr.cpp
    src/hist.cpp
    src/stack.cpp
    src/stretch.cpp
    src/utils.cpp
)

# Headers
set(${PROJECT_NAME}_HEADERS
    include/debayer.hpp
    include/fitsio.hpp
    include/fitskeyword.hpp
    include/hfr.hpp
    include/hist.hpp
    include/stack.hpp
    include/stretch.hpp
    include/utils.hpp
)

# Private Headers
set(${PROJECT_NAME}_PRIVATE_HEADERS
    _component.hpp
)

set(${PROJECT_NAME}_LIBS
    loguru
    atom-component
    atom-error
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
    ${${PROJECT_NAME}_PRIVATE_HEADERS}
)

target_link_libraries(${PROJECT_NAME}_OBJECT ${${PROJECT_NAME}_LIBS})

add_library(${PROJECT_NAME} SHARED)

target_link_libraries(${PROJECT_NAME} ${PROJECT_NAME}_OBJECT ${${PROJECT_NAME}_LIBS})
target_include_directories(${PROJECT_NAME} PUBLIC .)
target_include_directories(${PROJECT_NAME} PUBLIC include)

set_target_properties(${PROJECT_NAME} PROPERTIES
    VERSION ${CMAKE_HYDROGEN_VERSION_STRING}
    SOVERSION ${HYDROGEN_SOVERSION}
    OUTPUT_NAME system
)

install(TARGETS ${PROJECT_NAME}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
