# CMakeLists.txt for Lithium Builtin Modules
# This project is licensed under the terms of the GPL3 license.
#
# Project Name: Lithium Builtin Modules
# Description:  A collection of useful system functions
# Author: Max Qian
# License: GPL3

cmake_minimum_required(VERSION 3.20)
project(lithium.builtin C CXX)

add_subdirectory(atom.io)
add_subdirectory(atom.system)
