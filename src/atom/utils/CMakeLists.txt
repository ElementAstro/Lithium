# CMakeLists.txt for Atom-Utils
# This project is licensed under the terms of the GPL3 license.
#
# Project Name: Atom-Utils
# Description: A collection of useful functions
# Author: Max Qian
# License: GPL3

cmake_minimum_required(VERSION 3.20)
project(atom-utils C CXX)

# Sources
set(${PROJECT_NAME}_SOURCES
    aes.cpp
    argsview.cpp
    qdatetime.cpp
    qtimer.cpp
    qtimezone.cpp
    random.cpp
    string.cpp
    stopwatcher.cpp
    time.cpp
    uuid.cpp
    xml.cpp
)

# Headers
set(${PROJECT_NAME}_HEADERS
    aes.hpp
    argsview.hpp
    qdatetime.hpp
    qtimer.hpp
    qtimezone.hpp
    random.hpp
    string.hpp
    stopwatcher.hpp
    switch.hpp
    time.hpp
    uuid.hpp
    xml.hpp
)

set(${PROJECT_NAME}_LIBS
    atom-io
    loguru
    tinyxml2
    OpenSSL::SSL
    OpenSSL::Crypto
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
