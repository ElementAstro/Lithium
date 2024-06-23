cmake_minimum_required(VERSION 3.20)
project(pymodule.embed.test)

add_executable(embed embed.cpp)
target_link_libraries(embed PRIVATE pocketpy)
target_link_directories(embed PRIVATE ${CMAKE_BINARY_DIR}/libs/pocketpy/include)