#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
@file         pacman.py
@brief        A command-line utility to manage Pacman packages.

@details      This script provides functionality to update the package database, upgrade the system,
              install and remove packages, search for packages, list installed packages, and more.
              It enhances internal operations with robust exception handling and detailed logging using Loguru.

              Usage:
                python pacman.py --update-db
                python pacman.py --upgrade
                python pacman.py --install <PACKAGE>
                python pacman.py --remove <PACKAGE> [--remove-deps]
                python pacman.py --search <QUERY>
                python pacman.py --list-installed
                python pacman.py --package-info <PACKAGE>
                python pacman.py --list-outdated
                python pacman.py --clear-cache
                python pacman.py --list-files <PACKAGE>
                python pacman.py --show-dependencies <PACKAGE>
                python pacman.py --find-file-owner <FILE>
                python pacman.py --fast-mirrors
                python pacman.py --downgrade <PACKAGE> <VERSION>
                python pacman.py --list-cache
                python pacman.py --multithread
                python pacman.py --list-group <GROUP>
                python pacman.py --optional-deps <PACKAGE>
                python pacman.py --enable-color
                python pacman.py --disable-color

@requires     - Python 3.7+
              - `loguru` Python library

@version      2.2
@date         2024-04-27
"""

import subprocess
import platform
import os
import argparse
from loguru import logger
import sys


class PacmanManager:
    def __init__(self):
        # Determine if the system is Windows
        self.is_windows = platform.system().lower() == 'windows'
        # Get the appropriate pacman command based on the platform
        self.pacman_command = self.find_pacman_command()

    def find_pacman_command(self):
        """
        Locate the 'pacman' command, either on Windows (MSYS2) or Linux/Unix systems.
        Returns the pacman executable path for MSYS2 on Windows, or just 'pacman' for Unix.
        """
        if self.is_windows:
            # Possible paths for MSYS2 pacman executable
            possible_paths = [
                r'C:\msys64\usr\bin\pacman.exe',
                r'C:\msys32\usr\bin\pacman.exe'
            ]
            for path in possible_paths:
                if os.path.exists(path):
                    logger.debug(f"Found pacman at: {path}")
                    return path
            logger.error(
                "MSYS2 pacman not found. Please ensure MSYS2 is installed.")
            raise FileNotFoundError(
                "MSYS2 pacman not found. Please ensure MSYS2 is installed.")
        else:
            logger.debug("Using system pacman command.")
            return 'pacman'

    def run_command(self, command):
        """
        Executes a given command using subprocess and handles both success and failure cases.
        If the platform is Windows, prepend the pacman command with the appropriate MSYS2 path.

        Args:
            command (list): The command and its arguments to execute.

        Returns:
            str: The standard output from the command execution.
        """
        if self.is_windows:
            command = [self.pacman_command] + command
        try:
            logger.debug(f"Executing command: {' '.join(command)}")
            result = subprocess.run(
                command, check=True, text=True, capture_output=True
            )
            logger.info(
                f"Command '{' '.join(command)}' executed successfully.")
            logger.debug(f"Command Output: {result.stdout.strip()}")
            return result.stdout.strip()
        except subprocess.CalledProcessError as e:
            logger.error(
                f"Error executing command '{' '.join(command)}': {e.stderr.strip()}")
            return f"Error: {e.stderr.strip()}"
        except FileNotFoundError:
            logger.error(f"Pacman command not found: {' '.join(command)}")
            return "Error: Pacman command not found."
        except Exception as e:
            logger.exception(
                f"Unexpected error executing command '{' '.join(command)}': {e}")
            return f"Error: {e}"

    def update_package_database(self):
        """Update the package database to get the latest package information."""
        logger.info("Updating package database...")
        return self.run_command(['-Sy'])

    def upgrade_system(self):
        """Upgrade the system by updating all installed packages to the latest versions."""
        logger.info("Upgrading the system...")
        return self.run_command(['-Syu'])

    def install_package(self, package_name):
        """Install a specific package."""
        logger.info(f"Installing package: {package_name}")
        return self.run_command(['-S', package_name])

    def remove_package(self, package_name, remove_deps=False):
        """
        Remove a specific package.
        Optionally remove its dependencies if 'remove_deps' is set to True.

        Args:
            package_name (str): The name of the package to remove.
            remove_deps (bool): Whether to remove dependencies as well.
        """
        command = ['-R', package_name]
        if remove_deps:
            command.append('--recursive')
            logger.info(
                f"Removing package '{package_name}' along with its dependencies.")
        else:
            logger.info(f"Removing package '{package_name}'.")
        return self.run_command(command)

    def search_package(self, query):
        """Search for a package by name or description."""
        logger.info(f"Searching for package with query: {query}")
        return self.run_command(['-Ss', query])

    def list_installed_packages(self):
        """List all installed packages on the system."""
        logger.info("Listing all installed packages.")
        return self.run_command(['-Q'])

    def show_package_info(self, package_name):
        """Display detailed information about a specific package."""
        logger.info(f"Showing information for package: {package_name}")
        return self.run_command(['-Qi', package_name])

    def list_outdated_packages(self):
        """List all packages that are outdated and need to be upgraded."""
        logger.info("Listing outdated packages.")
        return self.run_command(['-Qu'])

    def clear_cache(self):
        """Clear the package cache to free up space."""
        logger.info("Clearing package cache.")
        return self.run_command(['-Scc'])

    def list_package_files(self, package_name):
        """List all the files installed by a specific package."""
        logger.info(f"Listing files for package: {package_name}")
        return self.run_command(['-Ql', package_name])

    def show_package_dependencies(self, package_name):
        """Show the dependencies of a specific package."""
        logger.info(f"Showing dependencies for package: {package_name}")
        return self.run_command(['-Qi', package_name])

    def find_file_owner(self, file_path):
        """Find which package owns a specific file."""
        logger.info(f"Finding owner of file: {file_path}")
        return self.run_command(['-Qo', file_path])

    def show_fastest_mirrors(self):
        """Display and select the fastest mirrors for package downloads."""
        logger.info("Updating to the fastest mirrors.")
        return self.run_command(['pacman-mirrors', '--fasttrack'])

    def downgrade_package(self, package_name, version):
        """
        Downgrade a package to a specific version.
        This requires the version to be available in the package cache or repositories.

        Args:
            package_name (str): The name of the package to downgrade.
            version (str): The version to downgrade to.
        """
        pkg_path = f'/var/cache/pacman/pkg/{package_name}-{version}.pkg.tar.zst'
        if not os.path.exists(pkg_path):
            logger.error(f"Package file for downgrade not found: {pkg_path}")
            return f"Error: Package file for downgrade not found: {pkg_path}"
        logger.info(
            f"Downgrading package '{package_name}' to version '{version}'.")
        return self.run_command(['-U', pkg_path])

    def list_cache_packages(self):
        """List all packages currently stored in the local package cache."""
        logger.info("Listing all packages in the local cache.")
        return self.run_command(['-Ql', '/var/cache/pacman/pkg'])

    def enable_multithreaded_downloads(self):
        """Enable multithreaded downloads to speed up package installation."""
        logger.info("Enabling multithreaded downloads in pacman.conf.")
        sed_command = [
            'sed', '-i',
            's/#ParallelDownloads = 5/ParallelDownloads = 5/g',
            '/etc/pacman.conf'
        ]
        return self.run_command(sed_command)

    def list_package_group(self, group_name):
        """List all packages in a specific package group."""
        logger.info(f"Listing all packages in group: {group_name}")
        return self.run_command(['-Sg', group_name])

    def list_optional_dependencies(self, package_name):
        """List optional dependencies of a package."""
        logger.info(
            f"Listing optional dependencies for package: {package_name}")
        return self.run_command(['-Qi', package_name])

    def enable_color_output(self, enable=True):
        """
        Enable or disable color output in pacman command-line results.
        This improves readability of pacman commands.

        Args:
            enable (bool): True to enable color, False to disable.
        """
        if enable:
            command = [
                'sed', '-i',
                's/#Color/Color/g',
                '/etc/pacman.conf'
            ]
            logger.info("Enabling color output in pacman.")
        else:
            command = [
                'sed', '-i',
                's/Color/#Color/g',
                '/etc/pacman.conf'
            ]
            logger.info("Disabling color output in pacman.")
        return self.run_command(command)


def main():
    # Create command-line argument parser
    parser = argparse.ArgumentParser(
        description='Pacman Package Manager CLI Tool')

    # Add arguments for various pacman operations
    parser.add_argument('--update-db', action='store_true',
                        help='Update the package database')
    parser.add_argument('--upgrade', action='store_true',
                        help='Upgrade the system')
    parser.add_argument('--install', type=str,
                        metavar='PACKAGE', help='Install a package')
    parser.add_argument('--remove', type=str,
                        metavar='PACKAGE', help='Remove a package')
    parser.add_argument('--remove-deps', action='store_true',
                        help='Remove a package along with its dependencies')
    parser.add_argument('--search', type=str, metavar='QUERY',
                        help='Search for a package')
    parser.add_argument('--list-installed', action='store_true',
                        help='List all installed packages')
    parser.add_argument('--package-info', type=str,
                        metavar='PACKAGE', help='Show package info')
    parser.add_argument('--list-outdated', action='store_true',
                        help='List outdated packages')
    parser.add_argument('--clear-cache', action='store_true',
                        help='Clear package cache')
    parser.add_argument('--list-files', type=str, metavar='PACKAGE',
                        help='List installed files of a package')
    parser.add_argument('--show-dependencies', type=str,
                        metavar='PACKAGE', help='Show package dependencies')
    parser.add_argument('--find-file-owner', type=str,
                        metavar='FILE', help='Find which package owns a file')
    parser.add_argument('--fast-mirrors', action='store_true',
                        help='Show and use the fastest mirrors')
    parser.add_argument('--downgrade', type=str, nargs=2, metavar=('PACKAGE',
                        'VERSION'), help='Downgrade a package to a specific version')
    parser.add_argument('--list-cache', action='store_true',
                        help='List packages in local cache')
    parser.add_argument('--multithread', action='store_true',
                        help='Enable multithreaded downloads')
    parser.add_argument('--list-group', type=str,
                        metavar='GROUP', help='List all packages in a group')
    parser.add_argument('--optional-deps', type=str, metavar='PACKAGE',
                        help='List optional dependencies of a package')
    parser.add_argument('--enable-color', action='store_true',
                        help='Enable color output in pacman')
    parser.add_argument('--disable-color', action='store_true',
                        help='Disable color output in pacman')

    # Parse the arguments provided by the user via command-line
    args = parser.parse_args()

    # Instantiate the PacmanManager class
    try:
        pacman = PacmanManager()
    except FileNotFoundError as e:
        logger.error(e)
        print(f"Error: {e}")
        sys.exit(1)
    except Exception as e:
        logger.exception(f"Failed to initialize PacmanManager: {e}")
        print(f"An unexpected error occurred: {e}")
        sys.exit(1)

    # Handle command-line arguments and execute the corresponding methods
    try:
        if args.update_db:
            output = pacman.update_package_database()
            print(output)

        if args.upgrade:
            output = pacman.upgrade_system()
            print(output)

        if args.install:
            output = pacman.install_package(args.install)
            print(output)

        if args.remove:
            output = pacman.remove_package(
                args.remove, remove_deps=args.remove_deps)
            print(output)

        if args.search:
            output = pacman.search_package(args.search)
            print(output)

        if args.list_installed:
            output = pacman.list_installed_packages()
            print(output)

        if args.package_info:
            output = pacman.show_package_info(args.package_info)
            print(output)

        if args.list_outdated:
            output = pacman.list_outdated_packages()
            print(output)

        # Handle newly added features
        if args.clear_cache:
            output = pacman.clear_cache()
            print(output)

        if args.list_files:
            output = pacman.list_package_files(args.list_files)
            print(output)

        if args.show_dependencies:
            output = pacman.show_package_dependencies(args.show_dependencies)
            print(output)

        if args.find_file_owner:
            output = pacman.find_file_owner(args.find_file_owner)
            print(output)

        if args.fast_mirrors:
            output = pacman.show_fastest_mirrors()
            print(output)

        if args.downgrade:
            package, version = args.downgrade
            output = pacman.downgrade_package(package, version)
            print(output)

        if args.list_cache:
            output = pacman.list_cache_packages()
            print(output)

        if args.multithread:
            output = pacman.enable_multithreaded_downloads()
            print(output)

        if args.list_group:
            output = pacman.list_package_group(args.list_group)
            print(output)

        if args.optional_deps:
            output = pacman.list_optional_dependencies(args.optional_deps)
            print(output)

        if args.enable_color:
            output = pacman.enable_color_output(enable=True)
            print(output)

        if args.disable_color:
            output = pacman.enable_color_output(enable=False)
            print(output)

        if not any(vars(args).values()):
            # If no arguments are provided, show the help message
            parser.print_help()

    except Exception as e:
        logger.exception(f"An error occurred during execution: {e}")
        print(f"An error occurred: {e}")
        sys.exit(1)


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
