#!/usr/bin/env python3
# -*- coding: utf-8 -*-
import os
import re
import subprocess
import sys
from loguru import logger

# Configure loguru logger
logger.remove()
logger.add(sys.stderr, level="INFO", format="{time} {level} {message}")


def get_executable_info(executable_path: str) -> list:
    """
    Get the help information of the executable and parse command options and descriptions.

    Args:
        executable_path (str): Path to the executable.

    Returns:
        list: A list of tuples containing command options and descriptions, e.g., [('-h, --help', 'Show this help message and exit')].
    """
    logger.info(f"Getting executable info for: {executable_path}")
    try:
        # Run the executable to get help information
        help_output = subprocess.check_output(
            [executable_path, '--help'], text=True)
        logger.debug(f"Help output: {help_output}")
    except subprocess.CalledProcessError as e:
        logger.error(f"Error running the executable: {e}")
        sys.exit(1)

    lines = help_output.splitlines()
    command_info = []

    # Use regular expressions to match command options and descriptions
    option_regex = re.compile(r'^\s*(-\w|--\w[\w-]*)(?:\s+<[^>]+>)?\s+(.*)$')

    for line in lines:
        match = option_regex.match(line)
        if match:
            option = match.group(1).strip()
            description = match.group(2).strip()
            command_info.append((option, description))
            logger.debug(f"Found option: {option}, description: {description}")

    logger.info(f"Parsed {len(command_info)} command options")
    return command_info


def generate_pybind11_code(executable_name: str, command_info: list) -> str:
    """
    Generate PyBind11 code to wrap the executable.

    Args:
        executable_name (str): Name of the executable.
        command_info (list): A list of tuples containing command options and descriptions.

    Returns:
        str: Generated PyBind11 code.
    """
    logger.info(f"Generating PyBind11 code for: {executable_name}")
    bindings_code = f"""
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <cstdlib>
#include <iostream>
#include <string>
#include <sstream>
#include <vector>

namespace py = pybind11;

// Run command and return output
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

// Wrapper class for the executable
class {executable_name}_Wrapper {{
public:
    // Run the executable and return output
    std::string run(const std::string& args) {{
        return run_command(args);
    }}

    // Set environment variables
    void set_env(const std::string& key, const std::string& value) {{
        setenv(key.c_str(), value.c_str(), 1);
    }}

    // Redirect output to a file
    void redirect_output(const std::string& filename) {{
        freopen(filename.c_str(), "w", stdout);
    }}
"""

    # Generate a function for each command option
    for option, description in command_info:
        function_name = f'get_{option.lstrip("-").replace("-", "_")}'
        bindings_code += f'    std::string {function_name}(const std::string& args = "") {{ return run_command("{
            option} " + args); }} // {description}\n'

    bindings_code += f"""
}};

// Create Python module
PYBIND11_MODULE({executable_name}_bindings, m) {{
    // Add wrapper class to module
    py::class_<{executable_name}_Wrapper>(m, "{executable_name}_Wrapper")
        .def(py::init<>())
        .def("run", &{executable_name}_Wrapper::run)
        .def("set_env", &{executable_name}_Wrapper::set_env)
        .def("redirect_output", &{executable_name}_Wrapper::redirect_output)
"""

    # Add function bindings for each command option
    for option, description in command_info:
        function_name = f'get_{option.lstrip("-").replace("-", "_")}'
        bindings_code += f'        .def("{function_name}", &{executable_name}_Wrapper::{
            function_name}, "{description}")\n'

    bindings_code += "    ;\n}\n"
    logger.info("PyBind11 code generation completed")
    return bindings_code


def generate_cmake_file(executable_name: str) -> str:
    """
    Generate CMakeLists.txt file to build the PyBind11 module.

    Args:
        executable_name (str): Name of the executable.

    Returns:
        str: Generated CMakeLists.txt file content.
    """
    logger.info(f"Generating CMakeLists.txt for: {executable_name}")
    cmake_content = f"""
cmake_minimum_required(VERSION 3.14)
project({executable_name}_bindings)

set(CMAKE_CXX_STANDARD 17)
find_package(pybind11 REQUIRED)

add_library({executable_name}_bindings MODULE bindings.cpp)
target_link_libraries({executable_name}_bindings PRIVATE pybind11::module)
"""
    logger.info("CMakeLists.txt generation completed")
    return cmake_content


def check_pybind11_installed():
    """
    Check if PyBind11 is installed, and install it if not.
    """
    logger.info("Checking if PyBind11 is installed")
    try:
        subprocess.check_output(
            ['python', '-m', 'pip', 'show', 'pybind11'], text=True)
        logger.info("PyBind11 is already installed")
    except subprocess.CalledProcessError:
        logger.warning("PyBind11 is not installed, installing...")
        subprocess.check_call(['python', '-m', 'pip', 'install', 'pybind11'])
        logger.info("PyBind11 installation completed")


def main():
    """
    Main function to parse command line arguments and generate PyBind11 code and CMakeLists.txt file.
    """
    if len(sys.argv) != 2:
        logger.error(f"Usage: {sys.argv[0]} <executable_path>")
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
        logger.info("bindings.cpp file has been written")

    with open('bindings/CMakeLists.txt', 'w') as f:
        f.write(cmake_content)
        logger.info("CMakeLists.txt file has been written")

    logger.info(
        "Binding code and CMakeLists.txt file have been generated in the 'bindings' directory.")


if __name__ == "__main__":
    main()
