cmake_minimum_required(VERSION 3.10)

# 项目名称
project(AutoTargets VERSION 1.0 LANGUAGES CXX)

# 设置目标源文件路径
set(SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})

# 递归查找当前目录及其子目录下的所有 .cpp 文件
file(GLOB_RECURSE cpp_files "${SOURCE_DIR}/*.cpp")

# 设置编译选项
set(CMAKE_CXX_STANDARD 17)  # 设置C++标准
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_EXTENSIONS OFF)

# 创建一个用于存放所有目标的输出目录
set(OUTPUT_DIR ${CMAKE_BINARY_DIR}/bin)

# 创建一个包含目录，用于存储头文件
include_directories(${SOURCE_DIR}/include)

# 遍历所有的cpp文件，为每个生成一个可执行文件目标
foreach(cpp_file ${cpp_files})
    # 获取文件名（不带路径和后缀）
    get_filename_component(target_name ${cpp_file} NAME_WE)
    
    # 为每个 .cpp 文件生成一个可执行文件目标
    add_executable(${target_name} ${cpp_file})

    # 设置每个目标的输出目录
    set_target_properties(${target_name} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIR}
    )

    # 为每个目标设置不同的编译选项
    target_compile_options(${target_name} PRIVATE
        $<$<CONFIG:Debug>:-g>           # Debug 模式下的编译选项
        $<$<CONFIG:Release>:-O3>        # Release 模式下的编译选项
        -Wall -Wextra                   # 所有模式的编译警告
    )

    # 如果需要链接一些外部库，可以通过 target_link_libraries
    # target_link_libraries(${target_name} PRIVATE some_library)

    # 打印每个目标的生成情况
    message(STATUS "Added target: ${target_name} from source: ${cpp_file}")
endforeach()

# 打印输出目录信息
message(STATUS "All binaries will be output to: ${OUTPUT_DIR}")
