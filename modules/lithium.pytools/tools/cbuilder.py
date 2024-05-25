#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Build System Helper Script

This script provides a utility to build projects using CMake or Meson.

Features:
- Supports both CMake and Meson build systems
- Allows specifying custom build options
- Supports cleaning, testing, and generating documentation
- Configurable via command-line arguments

Usage:
    python build_system_helper.py --builder cmake --source_dir src --build_dir build --install --test
    python build_system_helper.py --builder meson --source_dir src --build_dir build --clean --generate_docs

Author:
    Max Qian <lightapt.com>

License:
    GPL-3.0-or-later
"""

import argparse
import subprocess
import sys
from pathlib import Path
from typing import Literal, List, Optional

class CMakeBuilder:
    """
    CMakeBuilder is a utility class to handle building projects using CMake.

    Args:
        source_dir (Path): Path to the source directory.
        build_dir (Path): Path to the build directory.
        generator (Literal["Ninja", "Unix Makefiles"]): CMake generator to use.
        build_type (Literal["Debug", "Release"]): Build type (Debug or Release).
        install_prefix (Path): Installation prefix directory.
        cmake_options (Optional[List[str]]): Additional options for CMake.

    Methods:
        run_command: Helper function to run shell commands.
        configure: Configure the CMake build system.
        build: Build the project.
        install: Install the project.
        clean: Clean the build directory.
        test: Run CTest for the project.
        generate_docs: Generate documentation if documentation target is available.
    """
    def __init__(
        self,
        source_dir: Path,
        build_dir: Path,
        generator: Literal["Ninja", "Unix Makefiles"] = "Ninja",
        build_type: Literal["Debug", "Release"] = "Debug",
        install_prefix: Path = None, # type: ignore
        cmake_options: Optional[List[str]] = None,
    ):
        self.source_dir = source_dir
        self.build_dir = build_dir
        self.generator = generator
        self.build_type = build_type
        self.install_prefix = install_prefix or build_dir / "install"
        self.cmake_options = cmake_options or []

    def run_command(self, *cmd: str):
        """
        Helper function to run shell commands.

        Args:
            cmd (str): The command and its arguments to run.
        """
        print(f"Running: {' '.join(cmd)}")
        result = subprocess.run(cmd, check=True, capture_output=True, text=True)
        print(result.stdout)
        if result.stderr:
            print(result.stderr, file=sys.stderr)

    def configure(self):
        """Configure the CMake build system."""
        self.build_dir.mkdir(parents=True, exist_ok=True)
        cmake_args = [
            "cmake",
            f"-G{self.generator}",
            f"-DCMAKE_BUILD_TYPE={self.build_type}",
            f"-DCMAKE_INSTALL_PREFIX={self.install_prefix}",
            str(self.source_dir),
        ] + self.cmake_options
        self.run_command(*cmake_args)

    def build(self, target: str = ""):
        """
        Build the project.

        Args:
            target (str): Specific build target to build.
        """
        build_cmd = ["cmake", "--build", str(self.build_dir)]
        if target:
            build_cmd += ["--target", target]
        self.run_command(*build_cmd)

    def install(self):
        """Install the project."""
        self.run_command("cmake", "--install", str(self.build_dir))

    def clean(self):
        """Clean the build directory."""
        if self.build_dir.exists():
            for item in self.build_dir.iterdir():
                if item.is_dir():
                    self.run_command("rm", "-rf", str(item))
                else:
                    item.unlink()

    def test(self):
        """Run CTest for the project."""
        self.run_command("ctest", "--output-on-failure", "-C", self.build_type, "-S", str(self.build_dir))

    def generate_docs(self, doc_target: str = "doc"):
        """
        Generate documentation if documentation target is available.

        Args:
            doc_target (str): Documentation target to build.
        """
        self.build(doc_target)

class MesonBuilder:
    """
    MesonBuilder is a utility class to handle building projects using Meson.

    Args:
        source_dir (Path): Path to the source directory.
        build_dir (Path): Path to the build directory.
        build_type (Literal["debug", "release"]): Build type (debug or release).
        install_prefix (Path): Installation prefix directory.
        meson_options (Optional[List[str]]): Additional options for Meson.

    Methods:
        run_command: Helper function to run shell commands.
        configure: Configure the Meson build system.
        build: Build the project.
        install: Install the project.
        clean: Clean the build directory.
        test: Run Meson tests for the project.
        generate_docs: Generate documentation if documentation target is available.
    """
    def __init__(
        self,
        source_dir: Path,
        build_dir: Path,
        build_type: Literal["debug", "release"] = "debug",
        install_prefix: Path = None, # type: ignore
        meson_options: Optional[List[str]] = None,
    ):
        self.source_dir = source_dir
        self.build_dir = build_dir
        self.build_type = build_type
        self.install_prefix = install_prefix or build_dir / "install"
        self.meson_options = meson_options or []

    def run_command(self, *cmd: str):
        """
        Helper function to run shell commands.

        Args:
            cmd (str): The command and its arguments to run.
        """
        print(f"Running: {' '.join(cmd)}")
        result = subprocess.run(cmd, check=True, capture_output=True, text=True)
        print(result.stdout)
        if result.stderr:
            print(result.stderr, file=sys.stderr)

    def configure(self):
        """Configure the Meson build system。"""
        self.build_dir.mkdir(parents=True, exist_ok=True)
        meson_args = [
            "meson",
            "setup",
            str(self.build_dir),
            str(self.source_dir),
            f"--buildtype={self.build_type}",
            f"--prefix={self.install_prefix}",
        ] + self.meson_options
        self.run_command(*meson_args)

    def build(self, target: str = ""):
        """
        Build the project.

        Args:
            target (str): Specific target to build.
        """
        build_cmd = ["meson", "compile", "-C", str(self.build_dir)]
        if target:
            build_cmd += ["--target", target]
        self.run_command(*build_cmd)

    def install(self):
        """Install the project。"""
        self.run_command("meson", "install", "-C", str(self.build_dir))

    def clean(self):
        """Clean the build directory。"""
        if self.build_dir.exists():
            for item in self.build_dir.iterdir():
                if item.is_dir():
                    self.run_command("rm", "-rf", str(item))
                else:
                    item.unlink()

    def test(self):
        """Run Meson tests for the project。"""
        self.run_command("meson", "test", "-C", str(self.build_dir), "--print-errorlogs")

    def generate_docs(self, doc_target: str = "doc"):
        """
        Generate documentation if documentation target is available.

        Args:
            doc_target (str): Documentation target to build.
        """
        self.build(doc_target)

def main():
    """
    Main function to run the build system helper.
    """
    parser = argparse.ArgumentParser(description="Build System Python Builder")
    parser.add_argument(
        "--source_dir", type=Path, default=Path(".").resolve(), help="Source directory"
    )
    parser.add_argument(
        "--build_dir",
        type=Path,
        default=Path("build").resolve(),
        help="Build directory",
    )
    parser.add_argument("--builder", choices=["cmake", "meson"], required=True, help="Choose the build system")
    parser.add_argument("--generator", choices=["Ninja", "Unix Makefiles"], default="Ninja")
    parser.add_argument("--build_type", choices=["Debug", "Release", "debug", "release"], default="Debug")
    parser.add_argument("--target", default="")
    parser.add_argument("--install", action="store_true", help="Install the project")
    parser.add_argument("--clean", action="store_true", help="Clean the build directory")
    parser.add_argument("--test", action="store_true", help="Run the tests")
    parser.add_argument(
        "--cmake_options",
        nargs="*",
        default=[],
        help="Custom CMake options (e.g. -DVAR=VALUE)",
    )
    parser.add_argument(
        "--meson_options",
        nargs="*",
        default=[],
        help="Custom Meson options (e.g. -Dvar=value)",
    )
    parser.add_argument("--generate_docs", action="store_true", help="Generate documentation")

    args = parser.parse_args()

    if args.builder == "cmake":
        builder = CMakeBuilder(
            source_dir=args.source_dir,
            build_dir=args.build_dir,
            generator=args.generator,
            build_type=args.build_type,
            cmake_options=args.cmake_options,
        )
    elif args.builder == "meson":
        builder = MesonBuilder(
            source_dir=args.source_dir,
            build_dir=args.build_dir,
            build_type=args.build_type,
            meson_options=args.meson_options,
        )

    if args.clean:
        builder.clean()

    builder.configure()
    builder.build(args.target)

    if args.install:
        builder.install()

    if args.test:
        builder.test()

    if args.generate_docs:
        builder.generate_docs()

if __name__ == "__main__":
    main()
