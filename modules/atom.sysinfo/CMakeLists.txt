# CMakeLists.txt for atom.sysinfo
# This project is licensed under the terms of the GPL3 license.
#
# Author: Max Qian
# License: GPL3

cmake_minimum_required(VERSION 3.20)
project(atom.sysinfo)

# Set the C++ standard
set(CMAKE_CXX_STANDARD 20)

# Add source files
set(SOURCE_FILES
    src/battery.cpp
    src/cpu.cpp
    src/disk.cpp
    src/gpu.cpp
    src/memory.cpp
    src/os.cpp
    src/wifi.cpp

    _component.cpp
    _main.cpp
)

# Create the module library
add_library(atom.sysinfo SHARED ${SOURCE_FILES})

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
