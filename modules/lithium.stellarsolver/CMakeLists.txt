# CMakeLists.txt

cmake_minimum_required(VERSION 3.5)
project(ssbindings)

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 查找所需的包
find_package(pybind11 REQUIRED)
find_package(Qt6 COMPONENTS Core REQUIRED)
find_package(StellarSolver REQUIRED)

# 包含目录
include_directories(
    ${Qt5Core_INCLUDE_DIRS}
    ${STELLARSOLVER_INCLUDE_DIRS}
)

# 源文件
set(SOURCE_FILES
    binding.cpp
    structures.cpp
    ss.cpp
)

# 添加 PyBind11 模块
pybind11_add_module(ssbindings MODULE ${SOURCE_FILES})

# 链接库
target_link_libraries(ssbindings PRIVATE
    Qt6::Core
    stellarsolver
)

# 定义编译标志（根据需要）
# 设置目标属性
set_target_properties(ssbindings PROPERTIES
    AUTOMOC ON
)

# 包含目录
target_include_directories(ssbindings PRIVATE
    ${pybind11_INCLUDE_DIRS}
    ${Qt6Core_INCLUDE_DIRS}
)