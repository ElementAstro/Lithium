# CMakeLists.txt for Atom-Type
# This project is licensed under the terms of the GPL3 license.
#
# Project Name: Atom-Type
# Description: All of the self-implement types
# Author: Max Qian
# License: GPL3

cmake_minimum_required(VERSION 3.20)
project(atom-type C CXX)

# Sources
set(${PROJECT_NAME}_SOURCES
    args.cpp
    ini.cpp
    message.cpp
)

# Headers
set(${PROJECT_NAME}_HEADERS
    args.hpp
    autotype.hpp
    ini_impl.hpp
    ini.hpp
    json.hpp
    message.hpp
    small_vector.hpp
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
target_link_libraries(${PROJECT_NAME} atom-utils)
target_include_directories(${PROJECT_NAME} PUBLIC .)

set_target_properties(${PROJECT_NAME} PROPERTIES
    VERSION ${CMAKE_HYDROGEN_VERSION_STRING}
    SOVERSION ${HYDROGEN_SOVERSION}
    OUTPUT_NAME ${PROJECT_NAME}
    TARGET ${CMAKE_BINARY_DIR}
)

install(TARGETS ${PROJECT_NAME}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
