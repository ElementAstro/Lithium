cmake_minimum_required(VERSION 3.13)
project(lithium-driver C CXX)

find_package(Threads REQUIRED)

add_subdirectory(ascom)
if(NOT WIN32)
add_subdirectory(hydrogen)
endif()
add_subdirectory(phd2)
add_subdirectory(platesolve)