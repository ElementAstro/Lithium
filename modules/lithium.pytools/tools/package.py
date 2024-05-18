#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import subprocess
import sys

def run_command(command : list) -> str:
    """
    Run a system command and return the output.

    Args:
        command (list): The command to run as a list of arguments.

    Returns:
        str: The standard output of the command.

    Raises:
        SystemExit: If the command returns a non-zero exit code.
    """
    result = subprocess.run(command, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
    if result.returncode != 0:
        print(f"Error: {result.stderr}")
    return result.stdout.strip()

def ensure_package_installed(package_name):
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

# Ensure the required dependencies are installed
ensure_package_installed("requests")
ensure_package_installed("packaging")
ensure_package_installed("importlib_metadata")
ensure_package_installed("importlib_resources")

import requests
from packaging import version
try:
    import importlib.metadata as importlib_metadata  # Python 3.8+
except ImportError:
    import importlib_metadata  # Python 3.7

def is_package_installed(package_name):
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

def get_installed_version(package_name):
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

def list_available_versions(package_name):
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
        response = requests.get(f"https://pypi.org/pypi/{package_name}/json")
        response.raise_for_status()
        data = response.json()
        versions = sorted(data['releases'].keys(), key=version.parse, reverse=True)
        return versions
    except requests.RequestException as e:
        print(f"Error fetching versions: {e}")
        return []

def install_package(package_name, package_version=None):
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

def upgrade_package(package_name):
    """
    Upgrade a Python package to the latest version using pip.

    Args:
        package_name (str): The name of the package to upgrade.

    Returns:
        None
    """
    command = [sys.executable, "-m", "pip", "install", "--upgrade", package_name]
    run_command(command)

def main():
    """
    Main function to handle user interactions and package management logic.

    Prompts the user for a package name, checks if it's installed, and allows the user
    to install, upgrade, or query package versions.

    Returns:
        None
    """
    package_name = input("Enter the package name: ").strip()
    
    if is_package_installed(package_name):
        installed_version = get_installed_version(package_name)
        print(f"Package '{package_name}' is already installed, version: {installed_version}")
        print("Checking for updates... Please wait...")
        available_versions = list_available_versions(package_name)
        if available_versions:
            latest_version = available_versions[0]
            if version.parse(installed_version) < version.parse(latest_version):
                print(f"A newer version is available: {latest_version}")
                update_choice = input("Do you want to update to the latest version? (y/n): ").strip().lower()
                if update_choice == 'y':
                    upgrade_package(package_name)
                else:
                    print("Skipping update.")
            else:
                print("Already at the latest version.")
        else:
            print(f"Could not fetch versions for '{package_name}'.")
    else:
        print(f"Package '{package_name}' is not installed.")
    
        action = input(
            "Choose an action:\n"
            "1. Install or Upgrade to the latest version\n"
            "2. Install a specific version\n"
            "3. List available versions\n"
            "Enter your choice (1/2/3): "
        ).strip()
        
        if action == '1':
            if is_package_installed(package_name):
                upgrade_package(package_name)
            else:
                install_package(package_name)
        
        elif action == '2':
            available_versions = list_available_versions(package_name)
            if available_versions:
                print("Available versions:")
                for v in available_versions:
                    print(v)
                version_to_install = input("Enter the version to install: ").strip()
                if version_to_install in available_versions:
                    install_package(package_name, version_to_install)
                else:
                    print("Invalid version. Please choose a valid version from the list.")
            else:
                print(f"No versions found for package '{package_name}'.")
        
        elif action == '3':
            available_versions = list_available_versions(package_name)
            if available_versions:
                print("Available versions:")
                for v in available_versions:
                    print(v)
            else:
                print(f"No versions found for package '{package_name}'.")
        
        else:
            print("Invalid choice.")

if __name__ == "__main__":
    main()