#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
@file         package.py
@brief        A command-line utility to manage Python packages.

@details      This script provides functionality to check, install, upgrade, and uninstall Python packages.
              It also allows listing installed packages, generating a requirements.txt file, and checking for
              dependency updates based on requirements.txt.

              Usage:
                python package.py --check <package_name>
                python package.py --install <package_name> [--version <version>]
                python package.py --upgrade <package_name>
                python package.py --uninstall <package_name>
                python package.py --list-installed
                python package.py --freeze [--output <file>]
                python package.py --check-updates [--requirements <file>]

@requires     - Python 3.7+
              - `requests` Python library
              - `loguru` Python library
              - `packaging` Python library

@version      2.1
@date         2024-04-27
"""

import subprocess
import sys
import argparse
import requests
from packaging import version as pkg_version
from pathlib import Path

try:
    import importlib.metadata as importlib_metadata  # Python 3.8+
except ImportError:
    import importlib_metadata  # Python 3.7

from loguru import logger

# Configure Loguru
logger.remove()
logger.add(
    "package_manager.log",
    rotation="10 MB",
    retention="7 days",
    compression="zip",
    enqueue=True,
    encoding="utf-8",
    format="<green>{time:YYYY-MM-DD HH:mm:ss}</green> | <level>{level}</level> | {message}",
    level="DEBUG",
)
logger.add(
    sys.stderr,
    level="INFO",
    format="<level>{message}</level>",
)
logger.debug("Logging is configured.")


def run_command(command: list) -> str:
    """
    Run a system command and return the output.

    Args:
        command (list): The command to run as a list of arguments.

    Returns:
        str: The standard output of the command.

    Raises:
        SystemExit: If the command returns a non-zero exit code.
    """
    try:
        logger.debug(f"Running command: {' '.join(command)}")
        result = subprocess.run(
            command, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True, check=True
        )
        logger.debug(f"Command output: {result.stdout.strip()}")
        return result.stdout.strip()
    except subprocess.CalledProcessError as e:
        logger.error(
            f"Command '{' '.join(command)}' failed with error: {e.stderr.strip()}")
        sys.exit(e.returncode)
    except Exception as e:
        logger.exception(
            f"Unexpected error running command '{' '.join(command)}': {e}")
        sys.exit(1)


def is_package_installed(package_name: str) -> bool:
    """
    Check if a Python package is installed.

    Args:
        package_name (str): The name of the package to check.

    Returns:
        bool: True if the package is installed, False otherwise.
    """
    try:
        installed_version = importlib_metadata.version(package_name)
        logger.info(
            f"Package '{package_name}' is installed (version {installed_version}).")
        return True
    except importlib_metadata.PackageNotFoundError:
        logger.info(f"Package '{package_name}' is not installed.")
        return False
    except Exception as e:
        logger.exception(
            f"Error checking if package '{package_name}' is installed: {e}")
        return False


def get_installed_version(package_name: str) -> str:
    """
    Get the installed version of a Python package.

    Args:
        package_name (str): The name of the package to check.

    Returns:
        str: The installed version of the package, or None if not installed.
    """
    try:
        return importlib_metadata.version(package_name)
    except importlib_metadata.PackageNotFoundError:
        return None
    except Exception as e:
        logger.exception(
            f"Error retrieving version for package '{package_name}': {e}")
        return None


def list_available_versions(package_name: str) -> list:
    """
    List all available versions of a Python package from PyPI.

    Args:
        package_name (str): The name of the package to check.

    Returns:
        list: A sorted list of available versions (from latest to oldest).
    """
    try:
        logger.debug(
            f"Fetching available versions for package '{package_name}' from PyPI.")
        response = requests.get(
            f"https://pypi.org/pypi/{package_name}/json", timeout=10)
        response.raise_for_status()
        data = response.json()
        versions = sorted(
            data['releases'].keys(),
            key=lambda v: pkg_version.parse(v),
            reverse=True
        )
        logger.debug(f"Available versions for '{package_name}': {versions}")
        return versions
    except requests.RequestException as e:
        logger.error(
            f"Error fetching versions for package '{package_name}': {e}")
        return []
    except ValueError as e:
        logger.error(f"Invalid version data for package '{package_name}': {e}")
        return []
    except Exception as e:
        logger.exception(
            f"Unexpected error listing versions for package '{package_name}': {e}")
        return []


def install_package(package_name: str, package_version: str = None):
    """
    Install a Python package using pip.

    Args:
        package_name (str): The name of the package to install.
        package_version (str, optional): The specific version of the package to install. Defaults to None.

    Returns:
        None
    """
    try:
        if package_version:
            available_versions = list_available_versions(package_name)
            if package_version not in available_versions:
                logger.error(
                    f"Version '{package_version}' of package '{package_name}' is not available.")
                print(
                    f"Version '{package_version}' of package '{package_name}' is not available.")
                sys.exit(1)
            package = f"{package_name}=={package_version}"
        else:
            package = package_name

        logger.info(f"Installing package '{package}'.")
        run_command([sys.executable, "-m", "pip", "install", package])
        logger.success(f"Package '{package}' installed successfully.")
    except Exception as e:
        logger.exception(f"Failed to install package '{package_name}': {e}")
        sys.exit(1)


def upgrade_package(package_name: str):
    """
    Upgrade a Python package to the latest version using pip.

    Args:
        package_name (str): The name of the package to upgrade.

    Returns:
        None
    """
    try:
        if not is_package_installed(package_name):
            logger.error(
                f"Package '{package_name}' is not installed. Cannot upgrade.")
            print(
                f"Package '{package_name}' is not installed. Cannot upgrade.")
            sys.exit(1)

        logger.info(
            f"Upgrading package '{package_name}' to the latest version.")
        run_command([sys.executable, "-m", "pip",
                    "install", "--upgrade", package_name])
        logger.success(f"Package '{package_name}' upgraded successfully.")
    except Exception as e:
        logger.exception(f"Failed to upgrade package '{package_name}': {e}")
        sys.exit(1)


def uninstall_package(package_name: str):
    """
    Uninstall a Python package using pip.

    Args:
        package_name (str): The name of the package to uninstall.

    Returns:
        None
    """
    try:
        if not is_package_installed(package_name):
            logger.error(
                f"Package '{package_name}' is not installed. Cannot uninstall.")
            print(
                f"Package '{package_name}' is not installed. Cannot uninstall.")
            sys.exit(1)

        confirmation = input(
            f"Are you sure you want to uninstall '{package_name}'? [y/N]: ")
        if confirmation.lower() != 'y':
            logger.info(
                f"Uninstallation of package '{package_name}' canceled by user.")
            print("Uninstallation canceled.")
            return

        logger.info(f"Uninstalling package '{package_name}'.")
        run_command([sys.executable, "-m", "pip",
                    "uninstall", "-y", package_name])
        logger.success(f"Package '{package_name}' uninstalled successfully.")
    except Exception as e:
        logger.exception(f"Failed to uninstall package '{package_name}': {e}")
        sys.exit(1)


def list_installed_packages():
    """
    List all installed Python packages.

    Returns:
        None
    """
    try:
        logger.info("Listing all installed Python packages.")
        output = run_command([sys.executable, "-m", "pip", "list"])
        print(output)
        logger.debug("Displayed installed packages.")
    except Exception as e:
        logger.exception(f"Failed to list installed packages: {e}")
        sys.exit(1)


def freeze_installed_packages(output_file: str = "requirements.txt"):
    """
    Generate a requirements.txt file for the current environment.

    Args:
        output_file (str): The name of the output file. Defaults to "requirements.txt".

    Returns:
        None
    """
    try:
        logger.info(f"Generating requirements file '{output_file}'.")
        output = run_command([sys.executable, "-m", "pip", "freeze"])
        with open(output_file, "w", encoding="utf-8") as f:
            f.write(output)
        logger.success(f"Requirements written to '{output_file}'.")
        print(f"Requirements written to '{output_file}'.")
    except Exception as e:
        logger.exception(
            f"Failed to generate requirements file '{output_file}': {e}")
        sys.exit(1)


def check_updates_from_requirements(requirements_file: str = "requirements.txt"):
    """
    Check for updates for packages listed in the requirements.txt file.

    Args:
        requirements_file (str): Path to the requirements.txt file. Defaults to "requirements.txt".

    Returns:
        None
    """
    try:
        req_path = Path(requirements_file)
        if not req_path.exists():
            logger.error(
                f"Requirements file '{requirements_file}' does not exist.")
            print(f"Requirements file '{requirements_file}' does not exist.")
            sys.exit(1)

        logger.info(f"Checking for updates based on '{requirements_file}'.")
        with req_path.open("r", encoding="utf-8") as f:
            requirements = f.readlines()

        outdated_packages = []

        for req in requirements:
            parsed = req.strip().split("==")
            if len(parsed) != 2:
                logger.warning(
                    f"Skipping invalid requirement line: '{req.strip()}'")
                continue
            pkg, installed_ver = parsed
            logger.debug(
                f"Checking package '{pkg}' (installed version: {installed_ver})")

            try:
                response = requests.get(
                    f"https://pypi.org/pypi/{pkg}/json", timeout=10)
                response.raise_for_status()
                data = response.json()
                latest_version = sorted(
                    data['releases'].keys(),
                    key=lambda v: pkg_version.parse(v),
                    reverse=True
                )[0]
                logger.debug(
                    f"Latest version of '{pkg}' is '{latest_version}'")

                if pkg_version.parse(latest_version) > pkg_version.parse(installed_ver):
                    outdated_packages.append(
                        (pkg, installed_ver, latest_version))
                    logger.info(
                        f"Package '{pkg}' is outdated: {installed_ver} -> {latest_version}")

            except requests.RequestException as e:
                logger.error(
                    f"Error fetching information for package '{pkg}': {e}")
            except (IndexError, KeyError) as e:
                logger.error(
                    f"Error parsing version data for package '{pkg}': {e}")
            except Exception as e:
                logger.exception(
                    f"Unexpected error checking package '{pkg}': {e}")

        if outdated_packages:
            print("Outdated packages found:")
            for pkg, current, latest in outdated_packages:
                print(f"- {pkg}: {current} -> {latest}")
            logger.info(f"Total outdated packages: {len(outdated_packages)}")
        else:
            print("All packages are up to date.")
            logger.info("All packages are up to date.")

    except Exception as e:
        logger.exception(f"Failed to check updates from requirements.txt: {e}")
        sys.exit(1)


def main():
    """
    Main function to handle user interactions and package management logic.

    Returns:
        None
    """
    parser = argparse.ArgumentParser(
        description="Python Package Management Utility")
    parser.add_argument("--check", metavar="PACKAGE",
                        help="Check if a specific package is installed")
    parser.add_argument("--install", metavar="PACKAGE",
                        help="Install a specific package")
    parser.add_argument("--version", metavar="VERSION",
                        help="Specify the version of the package to install")
    parser.add_argument("--upgrade", metavar="PACKAGE",
                        help="Upgrade a specific package to the latest version")
    parser.add_argument("--uninstall", metavar="PACKAGE",
                        help="Uninstall a specific package")
    parser.add_argument("--list-installed", action="store_true",
                        help="List all installed packages")
    parser.add_argument("--freeze", metavar="FILE", nargs="?",
                        const="requirements.txt", help="Generate a requirements.txt file")
    parser.add_argument("--check-updates", metavar="FILE", nargs="?",
                        const="requirements.txt", help="Check for updates based on a requirements.txt file")

    args = parser.parse_args()

    if args.check:
        if is_package_installed(args.check):
            installed_version = get_installed_version(args.check)
            print(
                f"Package '{args.check}' is installed, version: {installed_version}")
            logger.info(
                f"Checked package '{args.check}': Installed version {installed_version}.")
        else:
            print(f"Package '{args.check}' is not installed.")
            logger.info(f"Checked package '{args.check}': Not installed.")

    if args.install:
        install_package(args.install, args.version)

    if args.upgrade:
        upgrade_package(args.upgrade)

    if args.uninstall:
        uninstall_package(args.uninstall)

    if args.list_installed:
        list_installed_packages()

    if args.freeze:
        output = args.freeze if isinstance(
            args.freeze, str) else "requirements.txt"
        freeze_installed_packages(output)

    if args.check_updates:
        output = args.check_updates if isinstance(
            args.check_updates, str) else "requirements.txt"
        check_updates_from_requirements(output)


if __name__ == "__main__":
    try:
        main()
    except KeyboardInterrupt:
        logger.warning("Operation interrupted by user.")
        print("\nOperation interrupted by user.")
        sys.exit(0)
    except Exception as e:
        logger.exception(f"An unexpected error occurred: {e}")
        print(f"An unexpected error occurred: {e}")
        sys.exit(1)
