#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Executable Binder Script

This script generates PyBind11 bindings for an executable by parsing its help information.
It supports dynamic parsing of command-line options, enhanced logging, and beautified terminal output.

Usage:
    python exebind.py <executable_path> [--module-name <module_name>] [--output-dir <output_directory>]

Author:
    Max Qian <lightapt.com>

License:
    GPL-3.0-or-later
"""

import subprocess
import sys
import re
from pathlib import Path
from typing import List, Tuple

from loguru import logger
from rich.console import Console
from rich.table import Table
from rich.prompt import Prompt

# Set up Rich console
console = Console()

# Configure loguru logger
logger.remove()
logger.add(
    sys.stderr,
    level="INFO",
    format="<green>{time:YYYY-MM-DD HH:mm:ss.SSS}</green> | <level>{level}</level> | <cyan>{module}</cyan>:<cyan>{function}</cyan>:<cyan>{line}</cyan> - <level>{message}</level>"
)


def get_executable_info(executable_path: str) -> List[Tuple[str, str]]:
    """
    Get the help information of the executable and parse command options and descriptions.

    Args:
        executable_path (str): Path to the executable.

    Returns:
        List[Tuple[str, str]]: A list of tuples containing command options and descriptions.
    """
    logger.info(f"Getting executable info for: {executable_path}")

    try:
        # Run the executable to get help information
        help_output = subprocess.check_output(
            [executable_path, '--help'], text=True, stderr=subprocess.STDOUT
        )
        logger.debug(f"Help output:\n{help_output}")
    except subprocess.CalledProcessError as e:
        logger.error(f"Error running the executable: {e.output}")
        sys.exit(1)

    lines = help_output.splitlines()
    command_info = []

    # Regular expressions to match command options and descriptions
    option_regex = re.compile(
        r'^\s*(-\w|--\w[\w-]*)(?:[,\s]+(-\w|--\w[\w-]*))?\s+(.*)$')

    for line in lines:
        match = option_regex.match(line)
        if match:
            options = filter(None, match.groups()[:-1])
            option = ', '.join(options)
            description = match.group(3).strip()
            command_info.append((option, description))
            logger.debug(f"Found option: {option}, description: {description}")

    logger.info(f"Parsed {len(command_info)} command options")
    return command_info


def generate_pybind11_code(module_name: str, executable_name: str, command_info: List[Tuple[str, str]]) -> str:
    """
    Generate PyBind11 code to wrap the executable.

    Args:
        module_name (str): Name of the Python module to generate.
        executable_name (str): Name of the executable.
        command_info (List[Tuple[str, str]]): A list of tuples containing command options and descriptions.

    Returns:
        str: Generated PyBind11 code.
    """
    logger.info(f"Generating PyBind11 code for: {executable_name}")

    # Generate header code
    bindings_code = f"""#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <cstdlib>
#include <iostream>
#include <string>
#include <array>
#include <memory>

namespace py = pybind11;

// Function to run a command and capture its output
std::string run_command(const std::string& args) {{
    std::string command = "{executable_name} " + args;
    std::array<char, 128> buffer;
    std::string result;
    std::shared_ptr<FILE> pipe(popen(command.c_str(), "r"), pclose);
    if (!pipe) throw std::runtime_error("popen() failed!");
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {{
        result += buffer.data();
    }}
    return result;
}}

// Wrapper class for the executable
class {executable_name}_Wrapper {{
public:
    // Run the executable with arguments and return output
    std::string run(const std::string& args) {{
        return run_command(args);
    }}

    // Set environment variables
    void set_env(const std::string& key, const std::string& value) {{
        setenv(key.c_str(), value.c_str(), 1);
    }}
""".strip()

    # Generate methods for each command option
    for option, description in command_info:
        method_name = f'run_{option.lstrip("-").replace("-", "_").replace(", ", "_")}'
        bindings_code += f"""
    // {description}
    std::string {method_name}(const std::string& args = "") {{
        return run_command("{option} " + args);
    }}"""

    bindings_code += f"""
}};

// Create Python module
PYBIND11_MODULE({module_name}, m) {{
    // Add wrapper class to module
    py::class_<{executable_name}_Wrapper>(m, "{executable_name}_Wrapper")
        .def(py::init<>())
        .def("run", &{executable_name}_Wrapper::run, "Run the executable with arguments")
        .def("set_env", &{executable_name}_Wrapper::set_env, "Set an environment variable")
""".strip()

    # Add method bindings for each command option
    for option, description in command_info:
        method_name = f'run_{option.lstrip("-").replace("-", "_").replace(", ", "_")}'
        bindings_code += f"""
        .def("{method_name}", &{executable_name}_Wrapper::{method_name}, "{description}")"""

    bindings_code += ";\n}"
    logger.info("PyBind11 code generation completed")
    return bindings_code


def generate_cmake_file(module_name: str) -> str:
    """
    Generate CMakeLists.txt file to build the PyBind11 module.

    Args:
        module_name (str): Name of the Python module to generate.

    Returns:
        str: Generated CMakeLists.txt file content.
    """
    logger.info(f"Generating CMakeLists.txt for: {module_name}")

    cmake_content = f"""cmake_minimum_required(VERSION 3.14)
project({module_name})

set(CMAKE_CXX_STANDARD 17)
find_package(pybind11 REQUIRED)

pybind11_add_module({module_name} bindings.cpp)
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


def parse_arguments():
    """
    Parse command-line arguments.

    Returns:
        argparse.Namespace: Parsed arguments.
    """
    import argparse

    parser = argparse.ArgumentParser(
        description="Generate PyBind11 bindings for an executable."
    )
    parser.add_argument(
        "executable_path",
        type=str,
        help="Path to the executable"
    )
    parser.add_argument(
        "--module-name",
        type=str,
        default=None,
        help="Name of the Python module to generate (default: basename of the executable)"
    )
    parser.add_argument(
        "--output-dir",
        type=str,
        default="bindings",
        help="Output directory for the generated code (default: 'bindings')"
    )
    args = parser.parse_args()
    return args


def display_command_options(command_info: List[Tuple[str, str]]):
    """
    Display the parsed command options in a table.

    Args:
        command_info (List[Tuple[str, str]]): List of command options and descriptions.
    """
    table = Table(title="Parsed Command Options")
    table.add_column("Option", style="cyan")
    table.add_column("Description", style="magenta")

    for option, description in command_info:
        table.add_row(option, description)

    console.print(table)


def main():
    """
    Main function to parse command line arguments and generate PyBind11 code and CMakeLists.txt file.
    """
    args = parse_arguments()

    executable_path = args.executable_path
    executable_name = Path(executable_path).name
    module_name = args.module_name or f"{executable_name}_bindings"
    output_dir = Path(args.output_dir)

    check_pybind11_installed()

    command_info = get_executable_info(executable_path)
    display_command_options(command_info)

    # Confirm with the user before proceeding
    proceed = Prompt.ask("Proceed with generating bindings?",
                         choices=["y", "n"], default="y")
    if proceed.lower() != 'y':
        logger.info("Operation cancelled by the user.")
        sys.exit(0)

    bindings_code = generate_pybind11_code(
        module_name, executable_name, command_info)
    cmake_content = generate_cmake_file(module_name)

    output_dir.mkdir(parents=True, exist_ok=True)
    logger.info(f"Writing files to directory: {output_dir}")

    bindings_file = output_dir / 'bindings.cpp'
    with bindings_file.open('w', encoding='utf-8') as f:
        f.write(bindings_code)
        logger.info(f"'bindings.cpp' file has been written: {bindings_file}")

    cmake_file = output_dir / 'CMakeLists.txt'
    with cmake_file.open('w', encoding='utf-8') as f:
        f.write(cmake_content)
        logger.info(f"'CMakeLists.txt' file has been written: {cmake_file}")

    console.print(
        f"[bold green]Binding code and CMakeLists.txt have been generated in '{output_dir}'[/bold green]")
    logger.info("All tasks completed successfully.")


if __name__ == "__main__":
    try:
        main()
    except Exception as e:
        logger.exception(f"An unexpected error occurred: {e}")
        sys.exit(1)
