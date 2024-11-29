# CMakeLists.txt for atom_iosysinfo
# This project is licensed under the terms of the GPL3 license.
#
# Author: Max Qian
# License: GPL3

cmake_minimum_required(VERSION 3.20)
project(atom_iosysinfo)

# Add source files
set(SOURCE_FILES
    component.cpp
)

set(${PROJECT_NAME}_LIBS
    loguru
    atom-component
    atom-sysinfo
    ${ZLIB_LIBRARIES}
    ${CMAKE_THREAD_LIBS_INIT}
)

# Create the module library
add_library(atom_iosysinfo SHARED ${SOURCE_FILES})

target_link_libraries(atom_iosysinfo ${${PROJECT_NAME}_LIBS})

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
