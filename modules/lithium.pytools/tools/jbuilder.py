#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
JavaScript Build System Helper Script

This script provides a utility to build JavaScript projects using npm or Yarn.

Features:
- Supports both npm and Yarn package managers
- Allows specifying custom build options
- Supports cleaning, testing, linting, formatting, and generating documentation
- Automatically checks for the required package manager and installs it if missing
- Supports automatic installation on various platforms (Linux, macOS, Windows)
- Configurable via command-line arguments

Usage:
    python js_build_system_helper.py --package_manager npm --project_dir src --clean --test
    python js_build_system_helper.py --package_manager yarn --project_dir src --install

Author:
    Max Qian <lightapt.com>

License:
    GPL-3.0-or-later
"""

import argparse
import platform
import subprocess
import sys
from pathlib import Path
from typing import List, Optional


class JavaScriptBuilder:
    """
    JavaScriptBuilder is a utility class to handle building JavaScript projects using npm or Yarn.

    Args:
        project_dir (Path): Path to the project directory.
        package_manager (str): Package manager to use (npm or yarn).
        build_options (Optional[List[str]]): Additional options for the build command.

    Methods:
        run_command: Helper function to run shell commands.
        check_package_manager: Check if the required package manager is installed.
        install_package_manager: Install the required package manager.
        install_dependencies: Install project dependencies.
        build: Build the project.
        clean: Clean the project by removing node_modules.
        test: Run tests for the project.
        lint: Lint the project code.
        format: Format the project code.
        start: Start the development server.
        generate_docs: Generate documentation using a documentation tool.
    """

    def __init__(
        self,
        project_dir: Path,
        package_manager: str,
        build_options: Optional[List[str]] = None,
    ):
        self.project_dir = project_dir
        self.package_manager = package_manager
        self.build_options = build_options or []

    def run_command(self, *cmd: str):
        """
        Helper function to run shell commands.

        Args:
            cmd (str): The command and its arguments to run.
        """
        print(f"Running: {' '.join(cmd)}")
        result = subprocess.run(cmd, cwd=self.project_dir,
                                check=True, capture_output=True, text=True)
        print(result.stdout)
        if result.stderr:
            print(result.stderr, file=sys.stderr)

    def check_package_manager(self):
        """
        Check if the required package manager is installed.

        Returns:
            bool: True if the package manager is installed, False otherwise.
        """
        try:
            self.run_command(self.package_manager, "--version")
        except FileNotFoundError:
            return False
        return True

    def install_package_manager(self):
        """
        Install the required package manager.

        Raises:
            ValueError: If the platform is unsupported.
        """
        if platform.system() == "Linux":
            if self.package_manager == "npm":
                self.run_command("sudo", "apt", "install", "-y", "npm")
            elif self.package_manager == "yarn":
                self.run_command("sudo", "npm", "install", "-g", "yarn")
        elif platform.system() == "Darwin":  # macOS
            if self.package_manager == "npm":
                self.run_command("brew", "install", "node")
            elif self.package_manager == "yarn":
                self.run_command("brew", "install", "yarn")
        elif platform.system() == "Windows":
            if self.package_manager == "npm":
                self.run_command("choco", "install", "nodejs")
            elif self.package_manager == "yarn":
                self.run_command("choco", "install", "yarn")
        else:
            raise ValueError(f"Unsupported platform: {platform.system()}")

    def install_dependencies(self):
        """
        Install project dependencies.
        """
        self.run_command(self.package_manager, "install")

    def build(self):
        """
        Build the project.
        """
        build_cmd = [self.package_manager, "run", "build"] + self.build_options
        self.run_command(*build_cmd)

    def clean(self):
        """
        Clean the project by removing node_modules.
        """
        self.run_command("rm", "-rf", "node_modules")

    def test(self):
        """
        Run tests for the project.
        """
        self.run_command(self.package_manager, "test")

    def lint(self):
        """
        Lint the project code.
        """
        self.run_command(self.package_manager, "run", "lint")

    def format(self):
        """
        Format the project code.
        """
        self.run_command(self.package_manager, "run", "format")

    def start(self):
        """
        Start the development server.
        """
        self.run_command(self.package_manager, "start")

    def generate_docs(self):
        """
        Generate documentation using a documentation tool.
        """
        self.run_command(self.package_manager, "run", "docs")


def main():
    """
    Main function to run the JavaScript build system helper.
    """
    parser = argparse.ArgumentParser(
        description="JavaScript Build System Helper")
    parser.add_argument(
        "--project_dir", type=Path, default=Path(".").resolve(), help="Project directory"
    )
    parser.add_argument(
        "--package_manager", choices=["npm", "yarn"], required=True, help="Choose the package manager")
    parser.add_argument("--install", action="store_true",
                        help="Install project dependencies")
    parser.add_argument("--build", action="store_true",
                        help="Build the project")
    parser.add_argument("--clean", action="store_true",
                        help="Clean the project")
    parser.add_argument("--test", action="store_true", help="Run tests")
    parser.add_argument("--lint", action="store_true",
                        help="Lint the project code")
    parser.add_argument("--format", action="store_true",
                        help="Format the project code")
    parser.add_argument("--start", action="store_true",
                        help="Start the development server")
    parser.add_argument(
        "--build_options",
        nargs="*",
        default=[],
        help="Custom build options",
    )
    parser.add_argument("--generate_docs", action="store_true",
                        help="Generate documentation")

    args = parser.parse_args()

    builder = JavaScriptBuilder(
        project_dir=args.project_dir,
        package_manager=args.package_manager,
        build_options=args.build_options,
    )

    if not builder.check_package_manager():
        print(f"{args.package_manager} is not installed. Installing now...")
        builder.install_package_manager()

    if args.clean:
        builder.clean()

    if args.install:
        builder.install_dependencies()

    if args.build:
        builder.build()

    if args.test:
        builder.test()

    if args.lint:
        builder.lint()

    if args.format:
        builder.format()

    if args.start:
        builder.start()

    if args.generate_docs:
        builder.generate_docs()


if __name__ == "__main__":
    main()
