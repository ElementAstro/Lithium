# 指定CMake的最低版本要求
cmake_minimum_required(VERSION 3.10)

# 设置项目名称
project(atom.component.test)

# 在本地查找Google Test库
find_package(GTest QUIET)

if(NOT GTEST_FOUND)
  # 如果在本地找不到Google Test库,则从GitHub下载
  include(FetchContent)
  FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG release-1.11.0
  )
  FetchContent_MakeAvailable(googletest)
  include(GoogleTest)
else()
  # 如果在本地找到了Google Test库,则使用本地库
  include(GoogleTest)
endif()

# 添加测试源文件
file(GLOB TEST_SOURCES ${PROJECT_SOURCE_DIR}/*.cpp)

# 创建测试可执行文件
add_executable(${PROJECT_NAME} ${TEST_SOURCES})

# 链接Google Test库和待测试的库
target_link_libraries(${PROJECT_NAME} gtest gtest_main atom-component loguru)
