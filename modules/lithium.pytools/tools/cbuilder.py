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
import os
from pathlib import Path
from typing import Literal, List, Optional


class BuildHelperBase:
    """
    Base class for build helpers providing shared functionality.

    Args:
        source_dir (Path): Path to the source directory.
        build_dir (Path): Path to the build directory.
        install_prefix (Path): Directory prefix where the project will be installed.
        options (Optional[List[str]]): Additional options for the build system.
        env_vars (Optional[dict]): Environment variables to set for the build process.
        verbose (bool): Flag to enable verbose output during command execution.

    Methods:
        run_command: Executes shell commands with optional environment variables and verbosity.
        clean: Cleans the build directory by removing all files and subdirectories.
    """

    def __init__(
        self,
        source_dir: Path,
        build_dir: Path,
        install_prefix: Path = None,  # type: ignore
        options: Optional[List[str]] = None,
        env_vars: Optional[dict] = None,
        verbose: bool = False,
    ):
        self.source_dir = source_dir
        self.build_dir = build_dir
        self.install_prefix = install_prefix or build_dir / "install"
        self.options = options or []
        self.env_vars = env_vars or {}
        self.verbose = verbose

    def run_command(self, *cmd: str):
        """
        Helper function to run shell commands with optional environment variables and verbosity.

        Args:
            cmd (str): The command and its arguments to run as separate strings.

        Raises:
            SystemExit: Exits with the command's return code if it fails.
        """
        print(f"Running: {' '.join(cmd)}")
        env = os.environ.copy()
        env.update(self.env_vars)
        try:
            result = subprocess.run(
                cmd, check=True, capture_output=True, text=True, env=env)
            if self.verbose or result.returncode != 0:
                print(result.stdout)
                if result.stderr:
                    print(result.stderr, file=sys.stderr)
        except subprocess.CalledProcessError as e:
            print(f"Error running command: {e}", file=sys.stderr)
            sys.exit(e.returncode)

    def clean(self):
        """
        Cleans the build directory by removing all files and subdirectories.

        This function ensures that the build directory is completely cleaned, making it ready
        for a fresh build by removing all existing files and directories inside the build path.
        """
        if self.build_dir.exists():
            for item in self.build_dir.iterdir():
                if item.is_dir():
                    self.run_command("rm", "-rf", str(item))
                else:
                    item.unlink()
            print(f"Cleaned: {self.build_dir}")


class CMakeBuilder(BuildHelperBase):
    """
    CMakeBuilder is a utility class to handle building projects using CMake.

    Args:
        source_dir (Path): Path to the source directory.
        build_dir (Path): Path to the build directory.
        generator (Literal["Ninja", "Unix Makefiles"]): The CMake generator to use (e.g., Ninja or Unix Makefiles).
        build_type (Literal["Debug", "Release"]): Type of build (Debug or Release).
        install_prefix (Path): Directory prefix where the project will be installed.
        cmake_options (Optional[List[str]]): Additional options for CMake.
        env_vars (Optional[dict]): Environment variables to set for the build process.
        verbose (bool): Flag to enable verbose output during command execution.
        parallel (int): Number of parallel jobs to use for building.

    Methods:
        configure: Configures the CMake build system by generating build files.
        build: Builds the project, optionally specifying a target.
        install: Installs the project to the specified prefix.
        test: Runs tests using CTest with detailed output on failure.
        generate_docs: Generates documentation using the specified documentation target.
    """

    def __init__(
        self,
        source_dir: Path,
        build_dir: Path,
        generator: Literal["Ninja", "Unix Makefiles"] = "Ninja",
        build_type: Literal["Debug", "Release"] = "Debug",
        install_prefix: Path = None,  # type: ignore
        cmake_options: Optional[List[str]] = None,
        env_vars: Optional[dict] = None,
        verbose: bool = False,
        parallel: int = 4,
    ):
        super().__init__(source_dir, build_dir,
                         install_prefix, cmake_options, env_vars, verbose)
        self.generator = generator
        self.build_type = build_type
        self.parallel = parallel

    def configure(self):
        """
        Configures the CMake build system.

        This function generates the necessary build files using CMake based on the
        specified generator, build type, and additional CMake options provided.
        """
        self.build_dir.mkdir(parents=True, exist_ok=True)
        cmake_args = [
            "cmake",
            f"-G{self.generator}",
            f"-DCMAKE_BUILD_TYPE={self.build_type}",
            f"-DCMAKE_INSTALL_PREFIX={self.install_prefix}",
            str(self.source_dir),
        ] + self.options
        self.run_command(*cmake_args)

    def build(self, target: str = ""):
        """
        Builds the project using CMake.

        Args:
            target (str): Specific build target to build (optional). If not specified, the default target is built.

        This function uses the CMake build command, supporting parallel jobs to speed up the build process.
        """
        build_cmd = ["cmake", "--build",
                     str(self.build_dir), "--parallel", str(self.parallel)]
        if target:
            build_cmd += ["--target", target]
        self.run_command(*build_cmd)

    def install(self):
        """
        Installs the project to the specified prefix.

        This function runs the CMake install command, which installs the built
        artifacts to the directory specified by the install prefix.
        """
        self.run_command("cmake", "--install", str(self.build_dir))

    def test(self):
        """
        Runs tests using CTest with detailed output on failure.

        This function runs CTest to execute the project's tests, providing detailed
        output if any tests fail, making it easier to diagnose issues.
        """
        self.run_command("ctest", "--output-on-failure", "-C",
                         self.build_type, "-S", str(self.build_dir))

    def generate_docs(self, doc_target: str = "doc"):
        """
        Generates documentation if the specified documentation target is available.

        Args:
            doc_target (str): The documentation target to build (default is 'doc').

        This function builds the specified documentation target using the CMake build command.
        """
        self.build(doc_target)


class MesonBuilder(BuildHelperBase):
    """
    MesonBuilder is a utility class to handle building projects using Meson.

    Args:
        source_dir (Path): Path to the source directory.
        build_dir (Path): Path to the build directory.
        build_type (Literal["debug", "release"]): Type of build (debug or release).
        install_prefix (Path): Directory prefix where the project will be installed.
        meson_options (Optional[List[str]]): Additional options for Meson.
        env_vars (Optional[dict]): Environment variables to set for the build process.
        verbose (bool): Flag to enable verbose output during command execution.
        parallel (int): Number of parallel jobs to use for building.

    Methods:
        configure: Configures the Meson build system by generating build files.
        build: Builds the project, optionally specifying a target.
        install: Installs the project to the specified prefix.
        test: Runs tests using Meson, with error logs printed on failures.
        generate_docs: Generates documentation using the specified documentation target.
    """

    def __init__(
        self,
        source_dir: Path,
        build_dir: Path,
        build_type: Literal["debug", "release"] = "debug",
        install_prefix: Path = None,  # type: ignore
        meson_options: Optional[List[str]] = None,
        env_vars: Optional[dict] = None,
        verbose: bool = False,
        parallel: int = 4,
    ):
        super().__init__(source_dir, build_dir,
                         install_prefix, meson_options, env_vars, verbose)
        self.build_type = build_type
        self.parallel = parallel

    def configure(self):
        """
        Configures the Meson build system.

        This function sets up the Meson build system, generating necessary build files based on
        the specified build type and additional Meson options provided.
        """
        self.build_dir.mkdir(parents=True, exist_ok=True)
        meson_args = [
            "meson",
            "setup",
            str(self.build_dir),
            str(self.source_dir),
            f"--buildtype={self.build_type}",
            f"--prefix={self.install_prefix}",
        ] + self.options
        self.run_command(*meson_args)

    def build(self, target: str = ""):
        """
        Builds the project using Meson.

        Args:
            target (str): Specific target to build (optional). If not specified, the default target is built.

        This function compiles the project using Meson's compile command, with support for parallel jobs
        to speed up the build process.
        """
        build_cmd = ["meson", "compile", "-C",
                     str(self.build_dir), f"-j{self.parallel}"]
        if target:
            build_cmd += ["--target", target]
        self.run_command(*build_cmd)

    def install(self):
        """
        Installs the project to the specified prefix.

        This function runs the Meson install command, which installs the built
        artifacts to the directory specified by the install prefix.
        """
        self.run_command("meson", "install", "-C", str(self.build_dir))

    def test(self):
        """
        Runs tests using Meson, with error logs printed on failures.

        This function runs Meson tests, displaying error logs for any failed tests
        to provide detailed feedback and aid in debugging.
        """
        self.run_command("meson", "test", "-C",
                         str(self.build_dir), "--print-errorlogs")

    def generate_docs(self, doc_target: str = "doc"):
        """
        Generates documentation if the specified documentation target is available.

        Args:
            doc_target (str): The documentation target to build (default is 'doc').

        This function builds the specified documentation target using the Meson build system.
        """
        self.build(doc_target)


def main():
    """
    Main function to run the build system helper.

    This function parses command-line arguments to determine the build system (CMake or Meson),
    source and build directories, build options, and actions (clean, build, install, test, generate docs).
    It then initializes the appropriate builder class and performs the requested operations.

    Command-line Arguments:
        --source_dir: Specifies the source directory of the project.
        --build_dir: Specifies the build directory where build files and artifacts will be generated.
        --builder: Specifies the build system to use ('cmake' or 'meson').
        --generator: Specifies the CMake generator (e.g., Ninja, Unix Makefiles) if using CMake.
        --build_type: Specifies the build type ('Debug', 'Release', 'debug', 'release').
        --target: Specifies a specific build target to build.
        --install: Flag to indicate that the project should be installed after building.
        --clean: Flag to indicate that the build directory should be cleaned before building.
        --test: Flag to indicate that tests should be run after building.
        --cmake_options: Additional options for CMake.
        --meson_options: Additional options for Meson.
        --generate_docs: Flag to indicate that documentation should be generated.
        --env: Environment variables to set during the build process.
        --verbose: Enables verbose output during command execution.
        --parallel: Number of parallel jobs to use for building.
    """
    parser = argparse.ArgumentParser(description="Build System Python Builder")
    parser.add_argument("--source_dir", type=Path,
                        default=Path(".").resolve(), help="Source directory")
    parser.add_argument("--build_dir", type=Path,
                        default=Path("build").resolve(), help="Build directory")
    parser.add_argument(
        "--builder", choices=["cmake", "meson"], required=True, help="Choose the build system")
    parser.add_argument(
        "--generator", choices=["Ninja", "Unix Makefiles"], default="Ninja", help="CMake generator to use")
    parser.add_argument("--build_type", choices=[
                        "Debug", "Release", "debug", "release"], default="Debug", help="Build type")
    parser.add_argument("--target", default="", help="Specify a build target")
    parser.add_argument("--install", action="store_true",
                        help="Install the project")
    parser.add_argument("--clean", action="store_true",
                        help="Clean the build directory")
    parser.add_argument("--test", action="store_true", help="Run the tests")
    parser.add_argument("--cmake_options", nargs="*", default=[],
                        help="Custom CMake options (e.g. -DVAR=VALUE)")
    parser.add_argument("--meson_options", nargs="*", default=[],
                        help="Custom Meson options (e.g. -Dvar=value)")
    parser.add_argument("--generate_docs", action="store_true",
                        help="Generate documentation")
    parser.add_argument("--env", nargs="*", default=[],
                        help="Set environment variables (e.g. VAR=value)")
    parser.add_argument("--verbose", action="store_true",
                        help="Enable verbose output")
    parser.add_argument("--parallel", type=int, default=4,
                        help="Number of parallel jobs for building")

    args = parser.parse_args()

    # Parse environment variables from the command line
    env_vars = {var.split("=")[0]: var.split("=")[1] for var in args.env}

    # Initialize the appropriate builder based on the specified build system
    if args.builder == "cmake":
        builder = CMakeBuilder(
            source_dir=args.source_dir,
            build_dir=args.build_dir,
            generator=args.generator,
            build_type=args.build_type,
            cmake_options=args.cmake_options,
            env_vars=env_vars,
            verbose=args.verbose,
            parallel=args.parallel,
        )
    elif args.builder == "meson":
        builder = MesonBuilder(
            source_dir=args.source_dir,
            build_dir=args.build_dir,
            build_type=args.build_type,
            meson_options=args.meson_options,
            env_vars=env_vars,
            verbose=args.verbose,
            parallel=args.parallel,
        )

    # Perform cleaning if requested
    if args.clean:
        builder.clean()

    # Configure the build system
    builder.configure()

    # Build the project with the specified target
    builder.build(args.target)

    # Install the project if the install flag is set
    if args.install:
        builder.install()

    # Run tests if the test flag is set
    if args.test:
        builder.test()

    # Generate documentation if the generate_docs flag is set
    if args.generate_docs:
        builder.generate_docs()


if __name__ == "__main__":
    main()
