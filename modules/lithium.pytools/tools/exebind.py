#!/usr/bin/env python3
# -*- coding: utf-8 -*-
import os
import re
import subprocess
import sys
import logging

# 设置日志记录级别和格式
logging.basicConfig(level=logging.INFO, format='%(levelname)s: %(message)s')

def get_executable_info(executable_path: str) -> list:
    """
    获取可执行文件的帮助信息，并解析命令选项和描述。

    Args:
        executable_path (str): 可执行文件的路径。

    Returns:
        list: 包含命令选项和描述的元组列表，例如：[('-h, --help', '显示此帮助消息并退出')]。
    """
    try:
        # 运行可执行文件，获取帮助信息
        help_output = subprocess.check_output([executable_path, '--help'], text=True)
    except subprocess.CalledProcessError as e:
        logging.error(f"运行可执行文件时出错: {e}")
        sys.exit(1)

    lines = help_output.splitlines()
    command_info = []

    # 使用正则表达式匹配命令选项和描述
    option_regex = re.compile(r'^\s*(-\w|--\w[\w-]*)(?:\s+<[^>]+>)?\s+(.*)$')

    for line in lines:
        match = option_regex.match(line)
        if match:
            option = match.group(1).strip()
            description = match.group(2).strip()
            command_info.append((option, description))

    return command_info

def generate_pybind11_code(executable_name: str, command_info: list) -> str:
    """
    生成 PyBind11 代码，用于包装可执行文件。

    Args:
        executable_name (str): 可执行文件名。
        command_info (list): 包含命令选项和描述的元组列表。

    Returns:
        str: 生成的 PyBind11 代码。
    """
    bindings_code = f"""
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <cstdlib>
#include <iostream>
#include <string>
#include <sstream>
#include <vector>

namespace py = pybind11;

// 运行命令并返回输出
std::string run_command(const std::string& args) {{
    std::string command = "{executable_name} " + args;
    std::array<char, 128> buffer;
    std::string result;
    std::shared_ptr<FILE> pipe(popen(command.c_str(), "r"), pclose);
    if (!pipe) throw std::runtime_error("popen() failed!");
    while (fgets(buffer.data(), 128, pipe.get()) != nullptr) {{
        result += buffer.data();
    }}
    return result;
}}

// 包装可执行文件的类
class {executable_name}_Wrapper {{
public:
    // 运行可执行文件，并返回输出
    std::string run(const std::string& args) {{
        return run_command(args);
    }}
"""

    # 为每个命令选项生成一个函数
    for option, description in command_info:
        function_name = f'get_{option.lstrip("-").replace("-", "_")}'
        bindings_code += f'    std::string {function_name}(const std::string& args = "") {{ return run_command("{option} " + args); }} // {description}\n'

    bindings_code += f"""
}};

// 创建 Python 模块
PYBIND11_MODULE({executable_name}_bindings, m) {{
    // 添加包装类到模块
    py::class_<{executable_name}_Wrapper>(m, "{executable_name}_Wrapper")
        .def(py::init<>())
        .def("run", &{executable_name}_Wrapper::run)
"""

    # 为每个命令选项添加函数绑定
    for option, description in command_info:
        function_name = f'get_{option.lstrip("-").replace("-", "_")}'
        bindings_code += f'        .def("{function_name}", &{executable_name}_Wrapper::{function_name}, "{description}")\n'

    bindings_code += "    ;\n}\n"
    return bindings_code

def generate_cmake_file(executable_name: str) -> str:
    """
    生成 CMakeLists.txt 文件，用于构建 PyBind11 模块。

    Args:
        executable_name (str): 可执行文件名。

    Returns:
        str: 生成的 CMakeLists.txt 文件内容。
    """
    cmake_content = f"""
cmake_minimum_required(VERSION 3.14)
project({executable_name}_bindings)

set(CMAKE_CXX_STANDARD 17)
find_package(pybind11 REQUIRED)

add_library({executable_name}_bindings MODULE bindings.cpp)
target_link_libraries({executable_name}_bindings PRIVATE pybind11::module)
"""
    return cmake_content

def check_pybind11_installed():
    """
    检查 PyBind11 是否已安装，如果未安装则进行安装。
    """
    try:
        subprocess.check_output(['python', '-m', 'pip', 'show', 'pybind11'], text=True)
    except subprocess.CalledProcessError:
        logging.info("Pybind11 未安装，正在安装...")
        subprocess.check_call(['python', '-m', 'pip', 'install', 'pybind11'])

def main():
    """
    主函数，解析命令行参数并生成 PyBind11 代码和 CMakeLists.txt 文件。
    """
    if len(sys.argv) != 2:
        logging.error(f"使用方法: {sys.argv[0]} <可执行文件路径>")
        sys.exit(1)

    executable_path = sys.argv[1]
    executable_name = os.path.basename(executable_path)

    check_pybind11_installed()

    command_info = get_executable_info(executable_path)

    bindings_code = generate_pybind11_code(executable_name, command_info)
    cmake_content = generate_cmake_file(executable_name)

    os.makedirs('bindings', exist_ok=True)

    with open('bindings/bindings.cpp', 'w') as f:
        f.write(bindings_code)

    with open('bindings/CMakeLists.txt', 'w') as f:
        f.write(cmake_content)

    logging.info("绑定代码和 CMakeLists.txt 文件已生成到 'bindings' 目录中。")

if __name__ == "__main__":
    main()
