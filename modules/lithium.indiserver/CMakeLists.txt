# CMakeLists.txt for lithium.indiserver
# This project is licensed under the terms of the GPL3 license.
#
# Author: Max Qian
# License: GPL3

cmake_minimum_required(VERSION 3.20)
project(lithium.indiserver)

# Set the C++ standard
set(CMAKE_CXX_STANDARD 20)

# Add source files
set(SOURCE_FILES
    src/indiserver.cpp
    src/collection.cpp
    src/iconnector.cpp

    _component.cpp
    _main.cpp
)

set(LIBS
    loguru
    atom-system
    atom-io
    tinyxml2
    atom-component
    atom-error
    atom-utils
)

if (WIN32)
    set(LIBS ${LIBS} ws2_32 version iphlpapi)
endif()

# Create the module library
add_library(lithium.indiserver SHARED ${SOURCE_FILES} ${HEADER_FILES})
target_link_libraries(lithium.indiserver PUBLIC ${LIBS})

# Include directories
target_include_directories(lithium.indiserver PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
