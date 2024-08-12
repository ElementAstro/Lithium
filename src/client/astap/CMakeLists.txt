# CMakeLists.txt for lithium.client.astap
# This project is licensed under the terms of the GPL3 license.
#
# Author: Max Qian
# License: GPL3

cmake_minimum_required(VERSION 3.20)
project(lithium.client.astap)

# Add source files
set(SOURCE_FILES
    astap.cpp

    _component.cpp
    _main.cpp
)

set(LIBS
    loguru
    atom-system
    atom-io
    atom-utils
    atom-component
    atom-error
)

# Create the module library
add_library(lithium.client.astap SHARED ${SOURCE_FILES} ${HEADER_FILES})
target_link_libraries(lithium.client.astap PUBLIC ${LIBS})

# Include directories
target_include_directories(lithium.client.astap PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)

enable_testing()

find_package(GTest REQUIRED)

find_package(GMock REQUIRED)

add_executable(lithium.client.astap.test _test.cpp)
target_link_libraries(lithium.client.astap.test lithium.client.astap GTest::GTest ${GMOCK_LIBRARY})
