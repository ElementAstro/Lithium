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

from .pyjson import load_json

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
    additional_link_flags: List[str] = field(default_factory.list)

    def compile(self, source_files: List[Path], output_file: Path, cpp_version: CppVersion, additional_flags: Optional[List[str]] = None):
        """
        Compile source files into an object file or executable.

        Args:
            source_files (List[Path]): List of source files to compile.
            output_file (Path): Path to the output file.
            cpp_version (CppVersion): C++ version to use.
            additional_flags (Optional[List[str]]): Additional flags for compilation.

        Raises:
            SystemExit: If the C++ version is not supported.
        """
        additional_flags = additional_flags or []
        if cpp_version in self.cpp_flags:
            version_flag = self.cpp_flags[cpp_version]
        else:
            print(f"Unsupported C++ version: {cpp_version}")
            sys.exit(1)

        compile_cmd = [self.command, version_flag] + self.additional_compile_flags + additional_flags + ["-c"] + [str(f) for f in source_files] + ["-o", str(output_file)]
        print(f"Running compile command: {' '.join(compile_cmd)}")
        subprocess.run(compile_cmd, check=True)

    def link(self, object_files: List[Path], output_file: Path, additional_flags: Optional[List[str]] = None):
        """
        Link object files into an executable.

        Args:
            object_files (List[Path]): List of object files to link.
            output_file (Path): Path to the output file.
            additional_flags (Optional[List[str]]): Additional flags for linking.
        """
        additional_flags = additional_flags or []
        link_cmd = [self.command] + self.additional_link_flags + [str(f) for f in object_files] + additional_flags + ["-o", str(output_file)]
        print(f"Running link command: {' '.join(link_cmd)}")
        subprocess.run(link_cmd, check=True)

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

    return compilers

def find_command(command: str) -> Optional[str]:
    """
    Find a command in the system path.

    Args:
        command (str): Command to find.

    Returns:
        Optional[str]: Path to the command if found, otherwise None.
    """
    result = subprocess.run(["which", command], stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
    if result.returncode == 0:
        return result.stdout.strip()
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
    print("Available compilers:")
    for idx, compiler in enumerate(compilers, start=1):
        print(f"{idx}. {compiler.name}")

    choice = input("Select a compiler by number: ").strip()
    try:
        return compilers[int(choice) - 1]
    except (ValueError, IndexError):
        print("Invalid selection.")
        sys.exit(1)

def select_cpp_version() -> CppVersion:
    """
    Select a C++ version from the available options.

    Returns:
        CppVersion: Selected C++ version.

    Raises:
        SystemExit: If the selection is invalid.
    """
    print("Available C++ versions:")
    for idx, version in enumerate(CppVersion, start=1):
        print(f"{idx}. {version.value}")

    choice = input("Select a C++ version by number: ").strip()
    try:
        return list(CppVersion)[int(choice) - 1]
    except (ValueError, IndexError):
        print("Invalid selection.")
        sys.exit(1)

def load_options_from_json(file_path: str) -> Dict[str, List[str]]:
    """
    Load compiler options from a JSON file.

    Args:
        file_path (str): Path to the JSON file.

    Returns:
        Dict[str, List[str]]: Dictionary containing compile and link flags.
    """
    data = load_json(file_path)
    return {
        "compile_flags": data.get("compile_flags", []),
        "link_flags": data.get("link_flags", []),
    }

def main():
    """
    Main function to run the compiler helper.
    """
    parser = argparse.ArgumentParser(description="Compiler Helper")
    parser.add_argument("source_files", nargs="+", type=Path, help="Source files to compile")
    parser.add_argument("-o", "--output", type=Path, required=True, help="Output file")
    parser.add_argument("--link", action="store_true", help="Link object files into an executable")
    parser.add_argument("--compiler", type=str, help="Specify the compiler to use (GCC, Clang, MSVC)")
    parser.add_argument("--cpp_version", type=str, help="Specify the C++ version to use (e.g., c++17, c++20)")
    parser.add_argument("--flags", nargs="*", help="Additional flags for compilation or linking")
    parser.add_argument("--compile-flags", nargs="*", help="Additional compilation flags")
    parser.add_argument("--link-flags", nargs="*", help="Additional linking flags")
    parser.add_argument("--json-options", type=str, help="Path to JSON file containing additional compile/link options")

    args = parser.parse_args()

    compilers = detect_compilers()
    if not compilers:
        print("No suitable compiler found.")
        sys.exit(1)

    if args.compiler:
        compiler = next((c for c in compilers if c.name.lower() == args.compiler.lower()), None)
        if not compiler:
            print(f"Compiler '{args.compiler}' not found.")
            sys.exit(1)
    else:
        compiler = select_compiler(compilers)

    if args.cpp_version:
        try:
            cpp_version = CppVersion[args.cpp_version.replace("++", "").upper()]
        except KeyError:
            print(f"Invalid C++ version: {args.cpp_version}")
            sys.exit(1)
    else:
        cpp_version = select_cpp_version()

    additional_compile_flags = args.compile_flags or []
    additional_link_flags = args.link_flags or []
    additional_flags = args.flags or []

    # Load additional options from JSON file if provided
    if args.json_options:
        json_options = load_options_from_json(args.json_options)
        additional_compile_flags.extend(json_options["compile_flags"])
        additional_link_flags.extend(json_options["link_flags"])

    if args.link:
        compiler.link(args.source_files, args.output, additional_flags + additional_link_flags)
    else:
        compiler.compile(args.source_files, args.output, cpp_version, additional_flags + additional_compile_flags)

if __name__ == "__main__":
    main()
