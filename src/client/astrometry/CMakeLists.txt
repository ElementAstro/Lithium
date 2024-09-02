# CMakeLists.txt for lithium.client.astrometry
# This project is licensed under the terms of the GPL3 license.
#
# Author: Max Qian
# License: GPL3

cmake_minimum_required(VERSION 3.20)
project(lithium.client.astrometry)

# Add source files
set(SOURCE_FILES
    astrometry.cpp
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
add_library(lithium.client.astrometry SHARED ${SOURCE_FILES} ${HEADER_FILES})
target_link_libraries(lithium.client.astrometry PUBLIC ${LIBS})

# Include directories
target_include_directories(lithium.client.astrometry PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)

enable_testing()

find_package(GTest REQUIRED)

find_package(GMock REQUIRED)

add_executable(lithium.client.astrometry.test _test.cpp)
target_link_libraries(lithium.client.astrometry.test lithium.client.astrometry GTest::GTest ${GMOCK_LIBRARY})
