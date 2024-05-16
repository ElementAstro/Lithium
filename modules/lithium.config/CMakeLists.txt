# This project is licensed under the terms of the GPL3 license.
#
# Project Name: Lithium.Config
# Description: the official config module for lithium server
# Author: Max Qian
# License: GPL3

cmake_minimum_required(VERSION 3.20)
project(lithium.config C CXX)

add_library(${PROJECT_NAME} SHARED)

# Sources
set(${PROJECT_NAME}_SOURCES
    _main.cpp
    _component.cpp
)

# Headers
set(${PROJECT_NAME}_HEADERS
    _component.hpp
)

set(${PROJECT_NAME}_LIBS
    atom-component
    lithium-config
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

target_link_libraries(${PROJECT_NAME} ${PROJECT_NAME}_OBJECT ${${PROJECT_NAME}_LIBS})
target_link_libraries(${PROJECT_NAME} ${${PROJECT_NAME}_LIBS})
target_include_directories(${PROJECT_NAME} PUBLIC .)

set_target_properties(${PROJECT_NAME} PROPERTIES
    OUTPUT_NAME ${PROJECT_NAME}
)

install(TARGETS ${PROJECT_NAME}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

add_executable(${PROJECT_NAME}_TEST _test.cpp)
target_link_libraries(${PROJECT_NAME}_TEST ${PROJECT_NAME})
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(${PROJECT_NAME}_TEST PRIVATE _DEBUG)
endif()
set_target_properties(${PROJECT_NAME}_TEST PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
)


