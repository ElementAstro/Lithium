# CMakeLists.txt for Lithium Builtin Test
# This project is licensed under the terms of the GPL3 license.
#
# Project Name: Lithium Test
# Description:  All tests for Lithium
# Author: Max Qian
# License: GPL3

cmake_minimum_required(VERSION 3.20)
project(lithium.test C CXX)

function(add_subdirectories_recursively start_dir)
  file(GLOB entries "${start_dir}/*")
  foreach(entry ${entries})
    if(IS_DIRECTORY ${entry} AND EXISTS "${entry}/CMakeLists.txt")
      message(STATUS "Adding test subdirectory: ${entry}")
      add_subdirectory(${entry})
    endif()
  endforeach()
endfunction()
add_subdirectories_recursively(${CMAKE_CURRENT_SOURCE_DIR})
