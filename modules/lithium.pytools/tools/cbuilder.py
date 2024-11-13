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
- Enhanced logging with Loguru
- Improved exception handling
- Additional functionalities: build reporting and environment setup

Usage:
    python build_system_helper.py --builder cmake --source_dir src --build_dir build --install --test
    python build_system_helper.py --builder meson --source_dir src --build_dir build --clean --generate_docs

Author:
    Max Qian <lightapt.com>

License:
    GPL-3.0-or-later
"""

import argparse
from datetime import datetime
import subprocess
import sys
import os
from pathlib import Path
from typing import Literal, List, Optional, Dict

from loguru import logger


class BuildHelperBase:
    """
    Base class for build helpers providing shared functionality.

    Args:
        source_dir (Path): Path to the source directory.
        build_dir (Path): Path to the build directory.
        install_prefix (Path): Directory prefix where the project will be installed.
        options (Optional[List[str]]): Additional options for the build system.
        env_vars (Optional[Dict[str, str]]): Environment variables to set for the build process.
        verbose (bool): Flag to enable verbose output during command execution.
        parallel (int): Number of parallel jobs to use for building.

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
        env_vars: Optional[Dict[str, str]] = None,
        verbose: bool = False,
        parallel: int = 4,
    ):
        self.source_dir = source_dir
        self.build_dir = build_dir
        self.install_prefix = install_prefix or build_dir / "install"
        self.options = options or []
        self.env_vars = env_vars or {}
        self.verbose = verbose
        self.parallel = parallel

    def run_command(self, *cmd: str):
        """
        Helper function to run shell commands with optional environment variables and verbosity.

        Args:
            cmd (str): The command and its arguments to run as separate strings.

        Raises:
            subprocess.CalledProcessError: If the command execution fails.
        """
        command_str = ' '.join(cmd)
        logger.debug(f"Executing command: {command_str}")
        try:
            result = subprocess.run(
                cmd,
                check=True,
                capture_output=True,
                text=True,
                env=os.environ | self.env_vars
            )
            if self.verbose:
                logger.info(result.stdout)
                if result.stderr:
                    logger.warning(result.stderr)
        except subprocess.CalledProcessError as e:
            logger.error(f"Command failed: {command_str}")
            logger.error(f"Return Code: {e.returncode}")
            logger.error(f"Output: {e.output}")
            logger.error(f"Error Output: {e.stderr}")
            raise

    def clean(self):
        """
        Cleans the build directory by removing all files and subdirectories.

        This function ensures that the build directory is completely cleaned, making it ready
        for a fresh build by removing all existing files and directories inside the build path.
        """
        if self.build_dir.exists():
            logger.info(f"Cleaning build directory: {self.build_dir}")
            try:
                for item in self.build_dir.iterdir():
                    if item.is_dir():
                        self.run_command("rm", "-rf", str(item))
                    else:
                        item.unlink()
                logger.success(
                    f"Build directory {self.build_dir} cleaned successfully.")
            except Exception as e:
                logger.exception(
                    f"Failed to clean build directory {self.build_dir}: {e}")
                raise


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
        env_vars (Optional[Dict[str, str]]): Environment variables to set for the build process.
        verbose (bool): Flag to enable verbose output during command execution.
        parallel (int): Number of parallel jobs to use for building.

    Methods:
        configure: Configures the CMake build system by generating build files.
        build: Builds the project, optionally specifying a target.
        install: Installs the project to the specified prefix.
        test: Runs tests using CTest with detailed output on failure.
        generate_docs: Generates documentation using the specified documentation target.
        generate_build_report: Generates a build report summarizing the build process.
    """

    def __init__(
        self,
        source_dir: Path,
        build_dir: Path,
        generator: Literal["Ninja", "Unix Makefiles"] = "Ninja",
        build_type: Literal["Debug", "Release"] = "Debug",
        install_prefix: Path = None,  # type: ignore
        cmake_options: Optional[List[str]] = None,
        env_vars: Optional[Dict[str, str]] = None,
        verbose: bool = False,
        parallel: int = 4,
    ):
        super().__init__(source_dir, build_dir, install_prefix,
                         cmake_options, env_vars, verbose, parallel)
        self.generator = generator
        self.build_type = build_type

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
            str(self.source_dir)
        ] + self.options
        logger.info("Configuring CMake project...")
        self.run_command(*cmake_args)
        logger.success("CMake configuration completed.")

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
        logger.info("Building the project with CMake...")
        self.run_command(*build_cmd)
        logger.success("Build completed successfully.")

    def install(self):
        """
        Installs the project to the specified prefix.

        This function runs the CMake install command, which installs the built
        artifacts to the directory specified by the install prefix.
        """
        logger.info("Installing the project...")
        self.run_command("cmake", "--install", str(self.build_dir))
        logger.success(
            f"Project installed to {self.install_prefix} successfully.")

    def test(self):
        """
        Runs tests using CTest with detailed output on failure.

        This function runs CTest to execute the project's tests, providing detailed
        output if any tests fail, making it easier to diagnose issues.
        """
        logger.info("Running tests with CTest...")
        try:
            self.run_command("ctest", "--output-on-failure",
                             "-C", self.build_type, "-S", str(self.build_dir))
            logger.success("All tests passed successfully.")
        except subprocess.CalledProcessError:
            logger.error(
                "Some tests failed. Check the output above for details.")
            raise

    def generate_docs(self, doc_target: str = "doc"):
        """
        Generates documentation if the specified documentation target is available.

        Args:
            doc_target (str): The documentation target to build (default is 'doc').

        This function builds the specified documentation target using the CMake build command.
        """
        logger.info(f"Generating documentation using target '{doc_target}'...")
        self.build(doc_target)
        logger.success("Documentation generated successfully.")

    def generate_build_report(self, report_file: Optional[Path] = None):
        """
        Generates a build report summarizing the build process.

        Args:
            report_file (Optional[Path]): Path to save the build report. If None, defaults to 'build_report.txt' in build_dir.
        """
        report_file = report_file or self.build_dir / "build_report.txt"
        logger.info(f"Generating build report at {report_file}...")
        try:
            with open(report_file, 'w') as f:
                f.write(f"CMake Build Report - {datetime.now()}\n")
                f.write(f"Source Directory: {self.source_dir}\n")
                f.write(f"Build Directory: {self.build_dir}\n")
                f.write(f"Generator: {self.generator}\n")
                f.write(f"Build Type: {self.build_type}\n")
                f.write(f"Install Prefix: {self.install_prefix}\n")
                f.write(f"Additional Options: {' '.join(self.options)}\n")
                f.write(f"Environment Variables: {self.env_vars}\n")
                f.write(f"Parallel Jobs: {self.parallel}\n")
            logger.success(f"Build report generated at {report_file}.")
        except Exception as e:
            logger.error(f"Failed to generate build report: {e}")
            raise


class MesonBuilder(BuildHelperBase):
    """
    MesonBuilder is a utility class to handle building projects using Meson.

    Args:
        source_dir (Path): Path to the source directory.
        build_dir (Path): Path to the build directory.
        build_type (Literal["debug", "release"]): Type of build (debug or release).
        install_prefix (Path): Directory prefix where the project will be installed.
        meson_options (Optional[List[str]]): Additional options for Meson.
        env_vars (Optional[Dict[str, str]]): Environment variables to set for the build process.
        verbose (bool): Flag to enable verbose output during command execution.
        parallel (int): Number of parallel jobs to use for building.

    Methods:
        configure: Configures the Meson build system by generating build files.
        build: Builds the project, optionally specifying a target.
        install: Installs the project to the specified prefix.
        test: Runs tests using Meson, with error logs printed on failures.
        generate_docs: Generates documentation using the specified documentation target.
        generate_build_report: Generates a build report summarizing the build process.
    """

    def __init__(
        self,
        source_dir: Path,
        build_dir: Path,
        build_type: Literal["debug", "release"] = "debug",
        install_prefix: Path = None,  # type: ignore
        meson_options: Optional[List[str]] = None,
        env_vars: Optional[Dict[str, str]] = None,
        verbose: bool = False,
        parallel: int = 4,
    ):
        super().__init__(source_dir, build_dir, install_prefix,
                         meson_options, env_vars, verbose, parallel)
        self.build_type = build_type

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
            f"--prefix={self.install_prefix}"
        ] + self.options
        logger.info("Configuring Meson project...")
        self.run_command(*meson_args)
        logger.success("Meson configuration completed.")

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
        logger.info("Building the project with Meson...")
        self.run_command(*build_cmd)
        logger.success("Build completed successfully.")

    def install(self):
        """
        Installs the project to the specified prefix.

        This function runs the Meson install command, which installs the built
        artifacts to the directory specified by the install prefix.
        """
        logger.info("Installing the project...")
        self.run_command("meson", "install", "-C", str(self.build_dir))
        logger.success(
            f"Project installed to {self.install_prefix} successfully.")

    def test(self):
        """
        Runs tests using Meson, with error logs printed on failures.

        This function runs Meson tests, displaying error logs for any failed tests
        to provide detailed feedback and aid in debugging.
        """
        logger.info("Running tests with Meson...")
        try:
            self.run_command("meson", "test", "-C",
                             str(self.build_dir), "--print-errorlogs")
            logger.success("All tests passed successfully.")
        except subprocess.CalledProcessError:
            logger.error(
                "Some tests failed. Check the output above for details.")
            raise

    def generate_docs(self, doc_target: str = "doc"):
        """
        Generates documentation if the specified documentation target is available.

        Args:
            doc_target (str): The documentation target to build (default is 'doc').

        This function builds the specified documentation target using the Meson build system.
        """
        logger.info(f"Generating documentation using target '{doc_target}'...")
        self.build(doc_target)
        logger.success("Documentation generated successfully.")

    def generate_build_report(self, report_file: Optional[Path] = None):
        """
        Generates a build report summarizing the build process.

        Args:
            report_file (Optional[Path]): Path to save the build report. If None, defaults to 'build_report.txt' in build_dir.
        """
        report_file = report_file or self.build_dir / "build_report.txt"
        logger.info(f"Generating build report at {report_file}...")
        try:
            with open(report_file, 'w') as f:
                f.write(f"Meson Build Report - {datetime.now()}\n")
                f.write(f"Source Directory: {self.source_dir}\n")
                f.write(f"Build Directory: {self.build_dir}\n")
                f.write(f"Build Type: {self.build_type}\n")
                f.write(f"Install Prefix: {self.install_prefix}\n")
                f.write(f"Additional Options: {' '.join(self.options)}\n")
                f.write(f"Environment Variables: {self.env_vars}\n")
                f.write(f"Parallel Jobs: {self.parallel}\n")
            logger.success(f"Build report generated at {report_file}.")
        except Exception as e:
            logger.error(f"Failed to generate build report: {e}")
            raise


def parse_args(args: List[str]) -> argparse.Namespace:
    """
    Parses command-line arguments.

    Args:
        args (List[str]): List of command-line arguments.

    Returns:
        argparse.Namespace: Parsed arguments.
    """
    parser = argparse.ArgumentParser(description="Build System Helper Script")
    parser.add_argument(
        "--source_dir",
        type=Path,
        default=Path(".").resolve(),
        help="Source directory"
    )
    parser.add_argument(
        "--build_dir",
        type=Path,
        default=Path("build").resolve(),
        help="Build directory"
    )
    parser.add_argument(
        "--builder",
        choices=["cmake", "meson"],
        required=True,
        help="Choose the build system"
    )
    parser.add_argument(
        "--generator",
        choices=["Ninja", "Unix Makefiles"],
        default="Ninja",
        help="CMake generator to use"
    )
    parser.add_argument(
        "--build_type",
        choices=["Debug", "Release", "debug", "release"],
        default="Debug",
        help="Build type"
    )
    parser.add_argument(
        "--target",
        default="",
        help="Specify a build target"
    )
    parser.add_argument(
        "--install",
        action="store_true",
        help="Install the project"
    )
    parser.add_argument(
        "--clean",
        action="store_true",
        help="Clean the build directory"
    )
    parser.add_argument(
        "--test",
        action="store_true",
        help="Run the tests"
    )
    parser.add_argument(
        "--cmake_options",
        nargs="*",
        default=[],
        help="Custom CMake options (e.g. -DVAR=VALUE)"
    )
    parser.add_argument(
        "--meson_options",
        nargs="*",
        default=[],
        help="Custom Meson options (e.g. -Dvar=value)"
    )
    parser.add_argument(
        "--generate_docs",
        action="store_true",
        help="Generate documentation"
    )
    parser.add_argument(
        "--env",
        nargs="*",
        default=[],
        help="Set environment variables (e.g. VAR=value)"
    )
    parser.add_argument(
        "--verbose",
        action="store_true",
        help="Enable verbose output"
    )
    parser.add_argument(
        "--parallel",
        type=int,
        default=4,
        help="Number of parallel jobs for building"
    )
    return parser.parse_args(args)


def setup_logging(verbose: bool) -> None:
    """
    Configures Loguru for logging.

    Args:
        verbose (bool): Flag to enable verbose logging.
    """
    logger.remove()
    logger.add(
        "build_helper.log",
        rotation="5 MB",
        retention="14 days",
        compression="zip",
        enqueue=True,
        encoding="utf-8",
        level="DEBUG",
        format="<green>{time:YYYY-MM-DD HH:mm:ss}</green> | <level>{level}</level> | <level>{message}</level>",
    )
    if verbose:
        logger.add(
            sys.stdout,
            level="DEBUG",
            format="<level>{message}</level>",
        )
    else:
        logger.add(
            sys.stdout,
            level="INFO",
            format="<level>{message}</level>",
        )


def parse_env_vars(env_list: List[str]) -> Dict[str, str]:
    """
    Parses environment variables from a list of strings.

    Args:
        env_list (List[str]): List of environment variable assignments (e.g., ["VAR=value"]).

    Returns:
        Dict[str, str]: Dictionary of environment variables.
    """
    env_vars = {}
    for env in env_list:
        if '=' in env:
            key, value = env.split('=', 1)
            env_vars[key] = value
        else:
            logger.warning(
                f"Ignoring invalid environment variable format: {env}")
    return env_vars


def main():
    """
    Main function to run the build system helper.

    This function parses command-line arguments to determine the build system (CMake or Meson),
    source and build directories, build options, and actions (clean, build, install, test, generate docs).
    It then initializes the appropriate builder class and performs the requested operations.
    """
    args = parse_args(sys.argv[1:])
    setup_logging(args.verbose)
    env_vars = parse_env_vars(args.env)

    logger.debug(f"Parsed arguments: {args}")

    # Initialize the appropriate builder based on the specified build system
    if args.builder == "cmake":
        builder = CMakeBuilder(
            source_dir=args.source_dir,
            build_dir=args.build_dir,
            generator=args.generator,
            build_type=args.build_type,
            install_prefix=args.build_dir / "install",
            options=args.cmake_options,
            env_vars=env_vars,
            verbose=args.verbose,
            parallel=args.parallel,
        )
    elif args.builder == "meson":
        builder = MesonBuilder(
            source_dir=args.source_dir,
            build_dir=args.build_dir,
            build_type=args.build_type,
            install_prefix=args.build_dir / "install",
            options=args.meson_options,
            env_vars=env_vars,
            verbose=args.verbose,
            parallel=args.parallel,
        )
    else:
        logger.error(f"Unsupported builder: {args.builder}")
        sys.exit(1)

    try:
        # Perform cleaning if requested
        if args.clean:
            builder.clean()

        # Configure the build system
        builder.configure()

        # Build the project with the specified target
        if args.target:
            logger.info(f"Building target: {args.target}")
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

        # Generate a build report
        builder.generate_build_report()

    except subprocess.CalledProcessError:
        logger.error("Build process terminated due to an error.")
        sys.exit(1)
    except Exception as e:
        logger.exception(f"An unexpected error occurred: {e}")
        sys.exit(1)


if __name__ == "__main__":
    try:
        main()
    except KeyboardInterrupt:
        logger.warning("Operation interrupted by user.")
        sys.exit(0)
