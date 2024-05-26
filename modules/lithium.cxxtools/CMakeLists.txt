# CMakeLists.txt for lithium.cxxtools
# This project is licensed under the terms of the GPL3 license.
#
# Author: Max Qian
# License: GPL3

cmake_minimum_required(VERSION 3.20)
project(lithium.cxxtools)

set(CMAKE_LITHIUM_CXXTOOLS_VERSION_MAJOR 1)
set(CMAKE_LITHIUM_CXXTOOLS_VERSION_MINOR 0)
set(CMAKE_LITHIUM_CXXTOOLS_VERSION_RELEASE 0)

set(LITHIUM_CXXTOOLS_SOVERSION ${CMAKE_LITHIUM_CXXTOOLS_VERSION_MAJOR})
set(CMAKE_LITHIUM_CXXTOOLS_VERSION_STRING "${CMAKE_LITHIUM_CXXTOOLS_VERSION_MAJOR}.${CMAKE_LITHIUM_CXXTOOLS_VERSION_MINOR}.${CMAKE_LITHIUM_CXXTOOLS_VERSION_RELEASE}")
set(LITHIUM_CXXTOOLS_VERSION ${CMAKE_LITHIUM_CXXTOOLS_VERSION_MAJOR}.${CMAKE_LITHIUM_CXXTOOLS_VERSION_MINOR}.${CMAKE_LITHIUM_CXXTOOLS_VERSION_RELEASE})

# Set the C++ standard
set(CMAKE_CXX_STANDARD 20)

# Add source files
set(SOURCE_FILES
    src/csv2json.cpp
    src/ini2json.cpp
    src/json2ini.cpp
    src/json2xml.cpp
    src/xml2json.cpp

    _component.cpp
    _main.cpp
)

find_package(tomlplusplus)

if (tomlplusplus_FOUND)
    list(APPEND SOURCE_FILES src/toml2json.cpp src/json2toml.cpp)
endif()

set(${PROJECT_NAME}_LIBS
    atom-component
    atom-error
    ${CMAKE_THREAD_LIBS_INIT}
    loguru
    tinyxml2
)

# Create the module library
add_library(lithium.cxxtools SHARED ${SOURCE_FILES})
target_link_libraries(lithium.cxxtools ${${PROJECT_NAME}_LIBS})

# Include directories
target_include_directories(lithium.cxxtools PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
