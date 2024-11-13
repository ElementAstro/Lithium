#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Compiler Helper Script

This script provides a utility to detect available C++ compilers (GCC, Clang, MSVC),
select the desired C++ version, and compile or link source files using specified options.

Features:
- Detect and select compilers (GCC, Clang, MSVC)
- Support various C++ versions (from C++98 to C++23)
- Compile and link source files
- Load additional compile/link options from JSON files
- Enhanced logging with Loguru
- Robust exception handling

Usage:
    python compiler_helper.py source1.cpp source2.cpp -o output.o --compiler GCC --cpp_version c++20 --link --flags -O3

Author:
    Max Qian <lightapt.com>

License:
    GPL-3.0-or-later
"""

import subprocess
import sys
from dataclasses import dataclass, field
from enum import Enum, auto
from pathlib import Path
from typing import List, Optional, Dict
import argparse

from loguru import logger
import json


class CppVersion(Enum):
    """
    Enum representing the supported C++ versions.
    """
    CPP98 = "c++98"
    CPP03 = "c++03"
    CPP11 = "c++11"
    CPP14 = "c++14"
    CPP17 = "c++17"
    CPP20 = "c++20"
    CPP23 = "c++23"


class CompilerType(Enum):
    """Enum representing the types of compilers."""
    GCC = auto()
    CLANG = auto()
    MSVC = auto()


@dataclass
class Compiler:
    """Class representing a compiler with its command and options."""
    name: str
    command: str
    compiler_type: CompilerType
    cpp_flags: Dict[CppVersion, str] = field(default_factory=dict)
    additional_compile_flags: List[str] = field(default_factory=list)
    additional_link_flags: List[str] = field(default_factory=list)

    def compile(self, source_files: List[Path], output_file: Path, cpp_version: CppVersion, additional_flags: Optional[List[str]] = None):
        """
        Compile source files into an object file or executable.

        Args:
            source_files (List[Path]): List of source files to compile.
            output_file (Path): Path to the output file.
            cpp_version (CppVersion): C++ version to use.
            additional_flags (Optional[List[str]]): Additional flags for compilation.

        Raises:
            SystemExit: If the C++ version is not supported or compilation fails.
        """
        additional_flags = additional_flags or []
        version_flag = self.cpp_flags.get(cpp_version)
        if not version_flag:
            logger.error(f"Unsupported C++ version: {cpp_version.value}")
            sys.exit(1)

        compile_cmd = [self.command, version_flag] + self.additional_compile_flags + \
            additional_flags + \
            ["-c"] + [str(f) for f in source_files] + ["-o", str(output_file)]
        logger.info(f"Running compile command: {' '.join(compile_cmd)}")
        try:
            subprocess.run(compile_cmd, check=True)
            logger.success(f"Compilation successful: {output_file}")
        except subprocess.CalledProcessError as e:
            logger.error(f"Compilation failed: {e}")
            sys.exit(1)

    def link(self, object_files: List[Path], output_file: Path, additional_flags: Optional[List[str]] = None):
        """
        Link object files into an executable.

        Args:
            object_files (List[Path]): List of object files to link.
            output_file (Path): Path to the output file.
            additional_flags (Optional[List[str]]): Additional flags for linking.

        Raises:
            SystemExit: If linking fails.
        """
        additional_flags = additional_flags or []
        link_cmd = [self.command] + self.additional_link_flags + \
            [str(f) for f in object_files] + \
            additional_flags + ["-o", str(output_file)]
        logger.info(f"Running link command: {' '.join(link_cmd)}")
        try:
            subprocess.run(link_cmd, check=True)
            logger.success(f"Linking successful: {output_file}")
        except subprocess.CalledProcessError as e:
            logger.error(f"Linking failed: {e}")
            sys.exit(1)


def setup_logging() -> None:
    """
    Configure Loguru for logging.
    """
    logger.remove()
    logger.add(
        "compiler_helper.log",
        rotation="10 MB",
        retention="7 days",
        compression="zip",
        enqueue=True,
        encoding="utf-8",
        format="<green>{time:YYYY-MM-DD HH:mm:ss}</green> | <level>{level}</level> | {message}",
        level="DEBUG"
    )
    logger.add(
        sys.stdout,
        level="INFO",
        format="<level>{message}</level>",
    )
    logger.debug("Logging is configured.")


def detect_compilers() -> List[Compiler]:
    """
    Detect available compilers on the system.

    Returns:
        List[Compiler]: List of detected compilers.
    """
    compilers = []

    gcc_path = find_command("gcc")
    if gcc_path:
        compilers.append(Compiler(
            name="GCC",
            command=gcc_path,
            compiler_type=CompilerType.GCC,
            cpp_flags={
                CppVersion.CPP98: "-std=c++98",
                CppVersion.CPP03: "-std=c++03",
                CppVersion.CPP11: "-std=c++11",
                CppVersion.CPP14: "-std=c++14",
                CppVersion.CPP17: "-std=c++17",
                CppVersion.CPP20: "-std=c++20",
                CppVersion.CPP23: "-std=c++23",
            },
            additional_compile_flags=["-Wall", "-Wextra", "-Werror"],
            additional_link_flags=[]
        ))
        logger.debug("GCC compiler detected.")

    clang_path = find_command("clang")
    if clang_path:
        compilers.append(Compiler(
            name="Clang",
            command=clang_path,
            compiler_type=CompilerType.CLANG,
            cpp_flags={
                CppVersion.CPP98: "-std=c++98",
                CppVersion.CPP03: "-std=c++03",
                CppVersion.CPP11: "-std=c++11",
                CppVersion.CPP14: "-std=c++14",
                CppVersion.CPP17: "-std=c++17",
                CppVersion.CPP20: "-std=c++20",
                CppVersion.CPP23: "-std=c++23",
            },
            additional_compile_flags=["-Wall", "-Wextra", "-Werror"],
            additional_link_flags=[]
        ))
        logger.debug("Clang compiler detected.")

    msvc_path = find_command("cl")
    if msvc_path:
        compilers.append(Compiler(
            name="MSVC",
            command=msvc_path,
            compiler_type=CompilerType.MSVC,
            cpp_flags={
                CppVersion.CPP98: "/std:c++98",
                CppVersion.CPP03: "/std:c++03",
                CppVersion.CPP11: "/std:c++11",
                CppVersion.CPP14: "/std:c++14",
                CppVersion.CPP17: "/std:c++17",
                CppVersion.CPP20: "/std:c++20",
                CppVersion.CPP23: "/std:c++latest",
            },
            additional_compile_flags=["/W4", "/WX"],
            additional_link_flags=["/DEBUG"]
        ))
        logger.debug("MSVC compiler detected.")

    if not compilers:
        logger.error("No suitable compiler found on the system.")
    return compilers


def find_command(command: str) -> Optional[str]:
    """
    Find a command in the system path.

    Args:
        command (str): Command to find.

    Returns:
        Optional[str]: Path to the command if found, otherwise None.
    """
    try:
        result = subprocess.run(["which", command], stdout=subprocess.PIPE,
                                stderr=subprocess.PIPE, text=True, check=True)
        path = result.stdout.strip()
        logger.debug(f"Found command '{command}' at: {path}")
        return path
    except subprocess.CalledProcessError:
        logger.warning(f"Command '{command}' not found.")
        return None


def select_compiler(compilers: List[Compiler]) -> Compiler:
    """
    Select a compiler from the list of detected compilers.

    Args:
        compilers (List[Compiler]): List of detected compilers.

    Returns:
        Compiler: Selected compiler.

    Raises:
        SystemExit: If the selection is invalid.
    """
    if len(compilers) == 1:
        logger.info(
            f"Only one compiler detected: {compilers[0].name}. Selecting it by default.")
        return compilers[0]

    print("Available compilers:")
    for idx, compiler in enumerate(compilers, start=1):
        print(f"{idx}. {compiler.name}")

    while True:
        choice = input("Select a compiler by number: ").strip()
        try:
            selected = compilers[int(choice) - 1]
            logger.info(f"Selected compiler: {selected.name}")
            return selected
        except (ValueError, IndexError):
            print("Invalid selection. Please enter a valid number.")


def select_cpp_version() -> CppVersion:
    """
    Select a C++ version from the available options.

    Returns:
        CppVersion: Selected C++ version.

    Raises:
        SystemExit: If the selection is invalid.
    """
    print("Available C++ versions:")
    versions = list(CppVersion)
    for idx, version in enumerate(versions, start=1):
        print(f"{idx}. {version.value}")

    while True:
        choice = input("Select a C++ version by number: ").strip()
        try:
            selected = versions[int(choice) - 1]
            logger.info(f"Selected C++ version: {selected.value}")
            return selected
        except (ValueError, IndexError):
            print("Invalid selection. Please enter a valid number.")


def load_options_from_json(file_path: str) -> Dict[str, List[str]]:
    """
    Load compiler options from a JSON file.

    Args:
        file_path (str): Path to the JSON file.

    Returns:
        Dict[str, List[str]]: Dictionary containing compile and link flags.

    Raises:
        SystemExit: If the JSON file cannot be loaded or is invalid.
    """
    try:
        with open(file_path, 'r', encoding="utf-8") as f:
            data = json.load(f)
        logger.debug(f"Loaded options from JSON file: {file_path}")
        return {
            "compile_flags": data.get("compile_flags", []),
            "link_flags": data.get("link_flags", []),
        }
    except FileNotFoundError:
        logger.error(f"JSON options file not found: {file_path}")
        sys.exit(1)
    except json.JSONDecodeError as e:
        logger.error(f"Invalid JSON format in {file_path}: {e}")
        sys.exit(1)


def main():
    """
    Main function to run the compiler helper.
    """
    setup_logging()
    logger.info("Starting Compiler Helper.")

    parser = argparse.ArgumentParser(description="Compiler Helper")
    parser.add_argument("source_files", nargs="+", type=Path,
                        help="Source files to compile")
    parser.add_argument("-o", "--output", type=Path,
                        required=True, help="Output file")
    parser.add_argument("--link", action="store_true",
                        help="Link object files into an executable")
    parser.add_argument("--compiler", type=str,
                        help="Specify the compiler to use (GCC, Clang, MSVC)")
    parser.add_argument("--cpp_version", type=str,
                        help="Specify the C++ version to use (e.g., c++17, c++20)")
    parser.add_argument("--flags", nargs="*",
                        help="Additional flags for compilation or linking")
    parser.add_argument("--compile-flags", nargs="*",
                        help="Additional compilation flags")
    parser.add_argument("--link-flags", nargs="*",
                        help="Additional linking flags")
    parser.add_argument("--json-options", type=str,
                        help="Path to JSON file containing additional compile/link options")

    args = parser.parse_args()

    compilers = detect_compilers()
    if not compilers:
        logger.critical("No suitable compiler found. Exiting.")
        sys.exit(1)

    if args.compiler:
        compiler = next((c for c in compilers if c.name.lower()
                        == args.compiler.lower()), None)
        if not compiler:
            logger.error(
                f"Compiler '{args.compiler}' not found among detected compilers.")
            sys.exit(1)
        logger.info(f"User selected compiler: {compiler.name}")
    else:
        compiler = select_compiler(compilers)

    if args.cpp_version:
        try:
            cpp_version = CppVersion(args.cpp_version.lower())
        except ValueError:
            logger.error(f"Invalid C++ version specified: {args.cpp_version}")
            sys.exit(1)
    else:
        cpp_version = select_cpp_version()

    additional_compile_flags = args.compile_flags or []
    additional_link_flags = args.link_flags or []
    additional_flags = args.flags or []

    # Load additional options from JSON file if provided
    if args.json_options:
        json_options = load_options_from_json(args.json_options)
        additional_compile_flags.extend(json_options.get("compile_flags", []))
        additional_link_flags.extend(json_options.get("link_flags", []))

    try:
        if args.link:
            logger.info("Starting linking process.")
            compiler.link(args.source_files, args.output,
                          additional_flags + additional_link_flags)
        else:
            logger.info("Starting compilation process.")
            compiler.compile(args.source_files, args.output, cpp_version,
                             additional_flags + additional_compile_flags)
    except Exception as e:
        logger.exception(f"An unexpected error occurred: {e}")
        sys.exit(1)

    logger.success("Compiler Helper finished successfully.")


if __name__ == "__main__":
    try:
        main()
    except KeyboardInterrupt:
        logger.warning("Operation interrupted by user.")
        sys.exit(0)
