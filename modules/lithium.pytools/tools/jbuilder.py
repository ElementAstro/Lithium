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
- Enhanced logging with Loguru
- Robust exception handling
- Cross-platform support for cleanup operations
- Configuration file support for build options

Usage:
    python jbuilder.py --package_manager npm --project_dir src --clean --test
    python jbuilder.py --package_manager yarn --project_dir src --install

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
from typing import Any, List, Optional, Dict
import json
import shutil

from loguru import logger


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
        list_available_scripts: List available npm/Yarn scripts from package.json.
    """

    def __init__(
        self,
        project_dir: Path,
        package_manager: str,
        build_options: Optional[List[str]] = None,
    ):
        self.project_dir = project_dir
        self.package_manager = package_manager.lower()
        self.build_options = build_options or []

    def run_command(self, *cmd: str) -> None:
        """
        Helper function to run shell commands.

        Args:
            cmd (str): The command and its arguments to run.
        """
        logger.debug(f"Executing command: {' '.join(cmd)}")
        try:
            result = subprocess.run(
                cmd,
                cwd=self.project_dir,
                check=True,
                capture_output=True,
                text=True,
            )
            if result.stdout:
                logger.debug(result.stdout)
            if result.stderr:
                logger.warning(result.stderr)
        except subprocess.CalledProcessError as e:
            logger.error(
                f"Command '{' '.join(cmd)}' failed with exit code {e.returncode}")
            if e.stderr:
                logger.error(e.stderr)
            raise
        except FileNotFoundError:
            logger.error(f"Command not found: {cmd[0]}")
            raise
        except Exception as e:
            logger.exception(
                f"Unexpected error while running command '{' '.join(cmd)}': {e}")
            raise

    def check_package_manager(self) -> bool:
        """
        Check if the required package manager is installed.

        Returns:
            bool: True if the package manager is installed, False otherwise.
        """
        try:
            self.run_command(self.package_manager, "--version")
            logger.info(f"{self.package_manager} is installed.")
            return True
        except Exception:
            logger.info(f"{self.package_manager} is not installed.")
            return False

    def install_package_manager(self) -> None:
        """
        Install the required package manager.

        Raises:
            ValueError: If the platform is unsupported or installation fails.
        """
        system = platform.system()
        logger.info(
            f"Attempting to install {self.package_manager} on {system} system.")

        try:
            if system == "Linux":
                if self.package_manager == "npm":
                    self.run_command("sudo", "apt-get", "update")
                    self.run_command("sudo", "apt-get", "install", "-y", "npm")
                elif self.package_manager == "yarn":
                    self.run_command("sudo", "npm", "install", "-g", "yarn")
            elif system == "Darwin":  # macOS
                if self.package_manager == "npm":
                    self.run_command("brew", "install", "node")
                elif self.package_manager == "yarn":
                    self.run_command("brew", "install", "yarn")
            elif system == "Windows":
                if self.package_manager == "npm":
                    self.run_command("choco", "install", "nodejs", "-y")
                elif self.package_manager == "yarn":
                    self.run_command("choco", "install", "yarn", "-y")
            else:
                raise ValueError(f"Unsupported platform: {system}")
            logger.info(f"{self.package_manager} installation attempted.")
        except subprocess.CalledProcessError as e:
            logger.error(f"Failed to install {self.package_manager}: {e}")
            raise
        except ValueError as ve:
            logger.error(ve)
            raise

    def install_dependencies(self) -> None:
        """
        Install project dependencies.
        """
        logger.info("Installing project dependencies.")
        self.run_command(self.package_manager, "install")

    def build(self) -> None:
        """
        Build the project.
        """
        logger.info("Building the project.")
        build_cmd = [self.package_manager, "run", "build"] + self.build_options
        self.run_command(*build_cmd)

    def clean(self) -> None:
        """
        Clean the project by removing node_modules.
        """
        node_modules = self.project_dir / "node_modules"
        logger.info("Cleaning the project by removing node_modules.")
        try:
            if node_modules.exists() and node_modules.is_dir():
                shutil.rmtree(node_modules)
                logger.success("node_modules removed successfully.")
            else:
                logger.warning("node_modules directory does not exist.")
        except Exception as e:
            logger.error(f"Failed to remove node_modules: {e}")
            raise

    def test(self) -> None:
        """
        Run tests for the project.
        """
        logger.info("Running project tests.")
        self.run_command(self.package_manager, "test")

    def lint(self) -> None:
        """
        Lint the project code.
        """
        logger.info("Linting the project code.")
        self.run_command(self.package_manager, "run", "lint")

    def format(self) -> None:
        """
        Format the project code.
        """
        logger.info("Formatting the project code.")
        self.run_command(self.package_manager, "run", "format")

    def start(self) -> None:
        """
        Start the development server.
        """
        logger.info("Starting the development server.")
        self.run_command(self.package_manager, "start")

    def generate_docs(self) -> None:
        """
        Generate documentation using a documentation tool.
        """
        logger.info("Generating project documentation.")
        self.run_command(self.package_manager, "run", "docs")

    def list_available_scripts(self) -> None:
        """
        List available npm/Yarn scripts from package.json.
        """
        package_json = self.project_dir / "package.json"
        if not package_json.exists():
            logger.error("package.json not found in the project directory.")
            return

        try:
            with open(package_json, 'r', encoding='utf-8') as f:
                data = json.load(f)
            scripts = data.get("scripts", {})
            if scripts:
                logger.info("Available scripts:")
                for script in scripts:
                    logger.info(f"- {script}")
            else:
                logger.warning("No scripts found in package.json.")
        except json.JSONDecodeError as e:
            logger.error(f"Invalid JSON format in package.json: {e}")
        except Exception as e:
            logger.exception(f"Unexpected error while listing scripts: {e}")


def setup_logging() -> None:
    """
    Configure Loguru for logging.
    """
    logger.remove()
    logger.add(
        "jbuilder.log",
        rotation="10 MB",
        retention="7 days",
        compression="zip",
        enqueue=True,
        encoding="utf-8",
        format="<green>{time:YYYY-MM-DD HH:mm:ss}</green> | <level>{level}</level> | {message}",
        level="DEBUG"
    )
    logger.add(
        sys.stderr,
        level="INFO",
        format="<level>{message}</level>",
    )
    logger.debug("Logging is configured.")


def load_config(config_path: Optional[Path]) -> Dict[str, Any]:
    """
    Load build options from a configuration file.

    Args:
        config_path (Optional[Path]): Path to the configuration file.

    Returns:
        Dict[str, Any]: Configuration options.
    """
    if config_path and config_path.exists():
        try:
            with open(config_path, 'r', encoding='utf-8') as f:
                config = json.load(f)
            logger.debug(f"Loaded configuration from {config_path}")
            return config
        except json.JSONDecodeError as e:
            logger.error(f"Invalid JSON format in configuration file: {e}")
        except Exception as e:
            logger.exception(f"Unexpected error while loading config: {e}")
    else:
        if config_path:
            logger.warning(f"Configuration file not found: {config_path}")
    return {}


def parse_arguments() -> argparse.Namespace:
    """
    Parse command-line arguments.

    Returns:
        argparse.Namespace: Parsed arguments.
    """
    parser = argparse.ArgumentParser(
        description="JavaScript Build System Helper with Enhanced Logging and Exception Handling"
    )
    parser.add_argument(
        "--project_dir", type=Path, default=Path(".").resolve(), help="Project directory"
    )
    parser.add_argument(
        "--package_manager", choices=["npm", "yarn"], required=True, help="Choose the package manager"
    )
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
    parser.add_argument("--generate_docs", action="store_true",
                        help="Generate documentation")
    parser.add_argument(
        "--build_options",
        nargs="*",
        default=[],
        help="Custom build options",
    )
    parser.add_argument(
        "--config",
        type=Path,
        help="Path to a JSON configuration file with build options",
    )
    parser.add_argument(
        "--list_scripts",
        action="store_true",
        help="List available npm/Yarn scripts from package.json",
    )

    return parser.parse_args()


def main():
    """
    Main function to run the JavaScript build system helper.
    """
    setup_logging()
    args = parse_arguments()

    config = load_config(args.config)
    additional_build_options = args.build_options + \
        config.get("build_options", [])

    builder = JavaScriptBuilder(
        project_dir=args.project_dir,
        package_manager=args.package_manager,
        build_options=additional_build_options,
    )

    try:
        if not builder.check_package_manager():
            logger.info(
                f"{args.package_manager} is not installed. Installing now...")
            builder.install_package_manager()
            if not builder.check_package_manager():
                logger.critical(
                    f"Failed to install {args.package_manager}. Exiting.")
                sys.exit(1)

        if args.list_scripts:
            builder.list_available_scripts()

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

    except Exception as e:
        logger.exception(f"An unexpected error occurred: {e}")
        sys.exit(1)

    logger.success("JavaScript Build System Helper finished successfully.")


if __name__ == "__main__":
    try:
        main()
    except KeyboardInterrupt:
        logger.warning("Operation interrupted by user.")
        sys.exit(0)
