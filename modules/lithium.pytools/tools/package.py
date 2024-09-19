#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
@file         package.py
@brief        A command-line utility to manage Python packages.

@details      This script provides functionality to check, install, upgrade, and uninstall Python packages.
              It also allows listing installed packages and generating a requirements.txt file.

              Usage:
                python package.py --check <package_name>
                python package.py --install <package_name> [--version <version>]
                python package.py --upgrade <package_name>
                python package.py --uninstall <package_name>
                python package.py --list-installed
                python package.py --freeze

@requires     - Python 3.x
              - `requests` Python library

@version      1.2
@date         Date of creation or last modification
"""

import subprocess
import sys
import argparse
import requests
from packaging import version

try:
    import importlib.metadata as importlib_metadata  # Python 3.8+
except ImportError:
    import importlib_metadata  # Python 3.7


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
    result = subprocess.run(command, stdout=subprocess.PIPE,
                            stderr=subprocess.PIPE, text=True, check=False)
    if result.returncode != 0:
        print(f"Error: {result.stderr}")
        sys.exit(result.returncode)
    return result.stdout.strip()


def ensure_package_installed(package_name: str):
    """
    Ensure that a Python package is installed via pip.

    Args:
        package_name (str): The name of the package to check and install if necessary.

    Returns:
        None
    """
    try:
        __import__(package_name)
    except ImportError:
        print(f"'{package_name}' is not installed. Installing...")
        run_command([sys.executable, "-m", "pip", "install", package_name])


def is_package_installed(package_name: str) -> bool:
    """
    Check if a Python package is installed.

    Args:
        package_name (str): The name of the package to check.

    Returns:
        bool: True if the package is installed, False otherwise.
    """
    try:
        importlib_metadata.version(package_name)
        return True
    except importlib_metadata.PackageNotFoundError:
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


def list_available_versions(package_name: str) -> list:
    """
    List all available versions of a Python package from PyPI.

    Args:
        package_name (str): The name of the package to check.

    Returns:
        list: A sorted list of available versions (from latest to oldest).

    Raises:
        requests.RequestException: If there's an error while fetching data from PyPI.
    """
    try:
        response = requests.get(f"https://pypi.org/pypi/{package_name}/json", timeout=5)
        response.raise_for_status()
        data = response.json()
        versions = sorted(data['releases'].keys(),
                          key=version.parse, reverse=True)
        return versions
    except requests.RequestException as e:
        print(f"Error fetching versions: {e}")
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
    if package_version:
        package = f"{package_name}=={package_version}"
    else:
        package = package_name
    command = [sys.executable, "-m", "pip", "install", package]
    run_command(command)


def upgrade_package(package_name: str):
    """
    Upgrade a Python package to the latest version using pip.

    Args:
        package_name (str): The name of the package to upgrade.

    Returns:
        None
    """
    command = [sys.executable, "-m", "pip",
               "install", "--upgrade", package_name]
    run_command(command)


def uninstall_package(package_name: str):
    """
    Uninstall a Python package using pip.

    Args:
        package_name (str): The name of the package to uninstall.

    Returns:
        None
    """
    command = [sys.executable, "-m", "pip", "uninstall", "-y", package_name]
    run_command(command)


def list_installed_packages():
    """
    List all installed Python packages.

    Returns:
        None
    """
    command = [sys.executable, "-m", "pip", "list"]
    output = run_command(command)
    print(output)


def freeze_installed_packages(output_file: str = "requirements.txt"):
    """
    Generate a requirements.txt file for the current environment.

    Args:
        output_file (str): The name of the output file. Defaults to "requirements.txt".

    Returns:
        None
    """
    command = [sys.executable, "-m", "pip", "freeze"]
    output = run_command(command)
    with open(output_file, "w", encoding="utf-8") as f:
        f.write(output)
    print(f"Requirements written to {output_file}")


def main():
    """
    Main function to handle user interactions and package management logic.

    Prompts the user for a package name, checks if it's installed, and allows the user
    to install, upgrade, uninstall, or query package versions.

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

    args = parser.parse_args()

    if args.check:
        if is_package_installed(args.check):
            print(f"Package '{args.check}' is installed, version: {
                  get_installed_version(args.check)}")
        else:
            print(f"Package '{args.check}' is not installed.")

    if args.install:
        install_package(args.install, args.version)

    if args.upgrade:
        upgrade_package(args.upgrade)

    if args.uninstall:
        uninstall_package(args.uninstall)

    if args.list_installed:
        list_installed_packages()

    if args.freeze:
        freeze_installed_packages(args.freeze)


if __name__ == "__main__":
    main()
