cmake_minimum_required(VERSION 3.20)

project(atom_iosearch.test)

file(GLOB_RECURSE TEST_SOURCES ${PROJECT_SOURCE_DIR}/*.cpp)

add_executable(${PROJECT_NAME} ${TEST_SOURCES})

target_link_libraries(${PROJECT_NAME} gtest gtest_main atom-search atom-error loguru)
