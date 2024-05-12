# CMakeLists.txt for Lithium Builtin Modules
# This project is licensed under the terms of the GPL3 license.
#
# Project Name: Lithium Builtin Modules
# Description:  A collection of useful system functions
# Author: Max Qian
# License: GPL3

cmake_minimum_required(VERSION 3.20)
project(lithium.builtin C CXX)

# Function to automatically add subdirectories that contain a CMakeLists.txt file
function(add_subdirectories_recursively start_dir)
  # Get all entries in the directory
  file(GLOB entries "${start_dir}/*")

  foreach(entry ${entries})
    # Check if the entry is a directory and that it contains a CMakeLists.txt file
    if(IS_DIRECTORY ${entry} AND EXISTS "${entry}/CMakeLists.txt" AND EXISTS "${entry}/package.json")
      message(STATUS "Adding module subdirectory: ${entry}")
      add_subdirectory(${entry})
    endif()
  endforeach()
endfunction()

# Use the function to add subdirectories
add_subdirectories_recursively(${CMAKE_CURRENT_SOURCE_DIR})