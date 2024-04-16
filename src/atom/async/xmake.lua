-- xmake.lua for Atom-Async
-- This project is licensed under the terms of the GPL3 license.
--
-- Project Name: Atom-Async
-- Description: Async Implementation of Lithium Server and Driver
-- Author: Max Qian
-- License: GPL3

add_rules("mode.debug", "mode.release")

set_project("atom-async")

set_languages("cxx20")

-- 设置CMake最低版本
cmake_minimum_required("version" 3.20)

-- 设置项目名称和描述
set_project_name("atom-async")
set_project_description("Async Implementation of Lithium Server and Driver")

-- 添加源文件
add_files("lock.cpp")
add_files("timer.cpp")

-- 添加头文件
add_header_files("async.hpp")
add_header_files("async_impl.hpp")
add_header_files("lock.hpp")
add_header_files("queue.hpp")
add_header_files("queue.inl")
add_header_files("thread_wrapper.hpp")
add_header_files("timer.hpp")
add_header_files("trigger.hpp")
add_header_files("trigger_impl.hpp")

-- 创建对象库
obj_library("atom_async_object" srcs _all_)
set_property("atom_async_object", {position_independent = true})
target_link_libraries("atom_async_object" "loguru")

-- 创建静态库
static_library("atom_async" srcs _all_ dependencies "atom_async_object")

-- 链接必要的库
target_link_libraries("atom_async" "atom_async_object" ${CMAKE_THREAD_LIBS_INIT})
target_include_directories("atom_async" include_path ".")

-- 设置版本和输出文件名
set_property("atom_async", {version = CMAKE_HYDROGEN_VERSION_STRING, so_version = HYDROGEN_SOVERSION, output_name = "atom_async"})

-- 安装目标
install_target("atom_async" arch "lib" dest "lib")

-- 如果构建Python模块
if (ATOM_BUILD_PYTHON)
    pybind11_add_module("atom_async_py" srcs "_pybind.cpp")
    target_link_libraries("atom_async_py" "atom_async")
endif()