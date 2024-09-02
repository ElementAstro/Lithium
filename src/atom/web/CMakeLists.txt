# CMakeLists.txt for Atom-Web
# This project is licensed under the terms of the GPL3 license.
#
# Project Name: Atom-Web
# Description: Web API
# Author: Max Qian
# License: GPL3

cmake_minimum_required(VERSION 3.20)
project(atom-web C CXX)

# Sources
set(${PROJECT_NAME}_SOURCES
    address.cpp
    downloader.cpp
    httplite.cpp
    httpparser.cpp
    utils.cpp
    time.cpp
)

# Headers
set(${PROJECT_NAME}_HEADERS
    address.hpp
    downloader.hpp
    httplite.hpp
    httpparser.hpp
    utils.hpp
    time.hpp
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

if(WIN32)
target_link_libraries(${PROJECT_NAME}_OBJECT wsock32 ws2_32)
endif()

target_link_libraries(${PROJECT_NAME} ${PROJECT_NAME}_OBJECT ${${PROJECT_NAME}_LIBS})
target_link_libraries(${PROJECT_NAME} ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(${PROJECT_NAME} cpp_httplib)
target_include_directories(${PROJECT_NAME} PUBLIC .)

set_target_properties(${PROJECT_NAME} PROPERTIES
    VERSION ${CMAKE_HYDROGEN_VERSION_STRING}
    SOVERSION ${HYDROGEN_SOVERSION}
    OUTPUT_NAME ${PROJECT_NAME}
)

install(TARGETS ${PROJECT_NAME}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
