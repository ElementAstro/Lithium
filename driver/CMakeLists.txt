# CMakeLists.txt for Atom-Driver
# This project is licensed under the terms of the GPL3 license.
#
# Project Name: Atom-Driver
# Description: A collection of drivers for Atom
# Author: Max Qian
# License: GPL3

cmake_minimum_required(VERSION 3.20)
project(atom-driver-interface C CXX)

add_subdirectory(solver)
add_subdirectory(client)
