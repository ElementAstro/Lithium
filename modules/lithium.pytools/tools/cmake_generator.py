#!/usr/bin/env python3
"""
This script automates the generation of CMake build configuration files for C++ projects. It supports multi-directory 
project structures, the creation of custom FindXXX.cmake files for third-party libraries, and the use of JSON configuration 
files for specifying project settings. 

Key features:
1. Multi-directory support with separate CMakeLists.txt for each subdirectory.
2. Custom FindXXX.cmake generation for locating third-party libraries.
3. JSON-based project configuration to streamline the CMakeLists.txt generation process.
4. Customizable compiler flags, linker flags, and dependencies.
"""
import argparse
import json
from pathlib import Path
from dataclasses import dataclass, field
import platform


@dataclass
class ProjectConfig:
    """
    Dataclass to hold the project configuration.

    Attributes:
        project_name (str): The name of the project.
        version (str): The version of the project.
        cpp_standard (str): The C++ standard to use (default is C++11).
        executable (bool): Whether to generate an executable target.
        static_library (bool): Whether to generate a static library.
        shared_library (bool): Whether to generate a shared library.
        enable_testing (bool): Whether to enable testing with CMake's `enable_testing()`.
        include_dirs (list): List of directories to include in the project.
        sources (str): Glob pattern to specify source files (default is `src/*.cpp`).
        compiler_flags (list): List of compiler flags (e.g., `-O3`, `-Wall`).
        linker_flags (list): List of linker flags (e.g., `-lpthread`).
        dependencies (list): List of third-party dependencies.
        subdirs (list): List of subdirectories for multi-directory project structure.
        install_path (str): Custom installation path (default is `bin`).
        test_framework (str): The test framework to be used (optional, e.g., `GoogleTest`).
    """
    project_name: str
    version: str = "1.0"
    cpp_standard: str = "11"
    executable: bool = True
    static_library: bool = False
    shared_library: bool = False
    enable_testing: bool = False
    include_dirs: list = field(default_factory=list)
    sources: str = "src/*.cpp"
    compiler_flags: list = field(default_factory=list)
    linker_flags: list = field(default_factory=list)
    dependencies: list = field(default_factory=list)
    subdirs: list = field(default_factory=list)
    install_path: str = "bin"
    test_framework: str = None  # Optional: e.g., GoogleTest


def detect_os() -> str:
    """
    Detects the current operating system and returns a suitable CMake system name.

    Returns:
        str: The appropriate CMake system name for the current OS (Windows, Darwin for macOS, Linux).
    """
    current_os = platform.system()
    if current_os == "Windows":
        return "set(CMAKE_SYSTEM_NAME Windows)\n"
    elif current_os == "Darwin":
        return "set(CMAKE_SYSTEM_NAME Darwin)\n"
    elif current_os == "Linux":
        return "set(CMAKE_SYSTEM_NAME Linux)\n"
    return ""


def generate_cmake(config: ProjectConfig) -> str:
    """
    Generates the content of the main CMakeLists.txt based on the provided project configuration.

    Args:
        config (ProjectConfig): The project configuration object containing all settings.

    Returns:
        str: The content of the generated CMakeLists.txt file.
    """
    cmake_template = f"""cmake_minimum_required(VERSION 3.15)

# Project name and version
project({config.project_name} VERSION {config.version})

# Set C++ standard
set(CMAKE_CXX_STANDARD {config.cpp_standard})
set(CMAKE_CXX_STANDARD_REQUIRED True)

# OS-specific settings
{detect_os()}

# Source files
file(GLOB_RECURSE SOURCES "{config.sources}")

# Include directories
"""
    if config.include_dirs:
        for include_dir in config.include_dirs:
            cmake_template += f'include_directories("{include_dir}")\n'

    # Compiler flags
    if config.compiler_flags:
        cmake_template += "add_compile_options("
        cmake_template += " ".join(config.compiler_flags) + ")\n"

    # Linker flags
    if config.linker_flags:
        cmake_template += "add_link_options("
        cmake_template += " ".join(config.linker_flags) + ")\n"

    # Dependencies (find_package)
    if config.dependencies:
        for dep in config.dependencies:
            cmake_template += f'find_package({dep} REQUIRED)\n'

    # Subdirectory handling for multi-directory support
    if config.subdirs:
        for subdir in config.subdirs:
            cmake_template += f'add_subdirectory({subdir})\n'

    # Create targets: executable or library
    if config.executable:
        cmake_template += f'add_executable({
            config.project_name} ${{{{SOURCES}}}})\n'
    elif config.static_library:
        cmake_template += f'add_library({
            config.project_name} STATIC ${{{{SOURCES}}}})\n'
    elif config.shared_library:
        cmake_template += f'add_library({
            config.project_name} SHARED ${{{{SOURCES}}}})\n'

    # Testing support
    if config.enable_testing:
        cmake_template += """
# Enable testing
enable_testing()
add_subdirectory(tests)
"""
    # Custom install path
    cmake_template += f"""
# Installation rule
install(TARGETS {config.project_name} DESTINATION {config.install_path})
"""

    return cmake_template


def generate_find_cmake(dependency_name: str) -> str:
    """
    Generates a FindXXX.cmake template for a specified third-party dependency.

    Args:
        dependency_name (str): The name of the third-party library (e.g., Boost, OpenCV).

    Returns:
        str: The content of the FindXXX.cmake file to locate the library.
    """
    return f"""# Find{dependency_name}.cmake - Find {dependency_name} library

# Locate the {dependency_name} library and headers
find_path({dependency_name}_INCLUDE_DIR
    NAMES {dependency_name}.h
    PATHS /usr/local/include /usr/include
)

find_library({dependency_name}_LIBRARY
    NAMES {dependency_name}
    PATHS /usr/local/lib /usr/lib
)

if({dependency_name}_INCLUDE_DIR AND {dependency_name}_LIBRARY)
    set({dependency_name}_FOUND TRUE)
    message(STATUS "{dependency_name} found")
else()
    set({dependency_name}_FOUND FALSE)
    message(FATAL_ERROR "{dependency_name} not found")
endif()

# Mark variables as advanced
mark_as_advanced({dependency_name}_INCLUDE_DIR {dependency_name}_LIBRARY)
"""


def save_file(content: str, directory: str = ".", filename: str = "CMakeLists.txt"):
    """
    Saves the provided content to a file.

    Args:
        content (str): The content to save to the file.
        directory (str): The directory where the file should be saved.
        filename (str): The name of the file (default is CMakeLists.txt).
    """
    directory_path = Path(directory)
    directory_path.mkdir(parents=True, exist_ok=True)

    file_path = directory_path / filename
    file_path.write_text(content, encoding='utf-8')

    print(f"{filename} generated in {file_path}")


def generate_from_json(json_file: str) -> ProjectConfig:
    """
    Reads project configuration from a JSON file and converts it into a ProjectConfig object.

    Args:
        json_file (str): Path to the JSON configuration file.

    Returns:
        ProjectConfig: A project configuration object with settings parsed from the JSON file.
    """
    with open(json_file, "r", encoding="utf-8") as file:
        data = json.load(file)

    return ProjectConfig(
        project_name=data.get("project_name", "MyProject"),
        version=data.get("version", "1.0"),
        cpp_standard=data.get("cpp_standard", "11"),
        executable=data.get("executable", True),
        static_library=data.get("static_library", False),
        shared_library=data.get("shared_library", False),
        enable_testing=data.get("enable_testing", False),
        include_dirs=data.get("include_dirs", []),
        sources=data.get("sources", "src/*.cpp"),
        compiler_flags=data.get("compiler_flags", []),
        linker_flags=data.get("linker_flags", []),
        dependencies=data.get("dependencies", []),
        subdirs=data.get("subdirs", []),
        install_path=data.get("install_path", "bin"),
        test_framework=data.get("test_framework", None)
    )


def parse_arguments():
    """
    Parses command-line arguments to either generate CMakeLists.txt files, FindXXX.cmake files, or handle JSON input.

    Returns:
        argparse.Namespace: Parsed command-line arguments.
    """
    parser = argparse.ArgumentParser(
        description="Generate a CMake template for C++ projects.")
    parser.add_argument(
        "--json", type=str, help="Path to JSON config file to generate CMakeLists.txt.")
    parser.add_argument("--find-package", type=str,
                        help="Generate a FindXXX.cmake file for a specified library.")

    return parser.parse_args()


if __name__ == "__main__":
    args = parse_arguments()

    if args.json:
        # Generate CMakeLists.txt from JSON configuration
        config = generate_from_json(args.json)
        cmake_content = generate_cmake(config)
        save_file(cmake_content)

        # Generate FindXXX.cmake files for each dependency specified in the JSON file
        for dep in config.dependencies:
            find_cmake_content = generate_find_cmake(dep)
            save_file(find_cmake_content, directory="cmake",
                      filename=f"Find{dep}.cmake")

    if args.find_package:
        # Generate a single FindXXX.cmake file for the specified dependency
        find_cmake_content = generate_find_cmake(args.find_package)
        save_file(find_cmake_content, directory="cmake",
                  filename=f"Find{args.find_package}.cmake")
