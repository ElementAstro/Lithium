cmake_minimum_required(VERSION 3.13)
project(lithium-driver C CXX)

find_package(Threads REQUIRED)

add_subdirectory(ascom)
add_subdirectory(hydrogen)