import subprocess
import platform
import os
import argparse
from loguru import logger


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
                    return path
            raise FileNotFoundError(
                "MSYS2 pacman not found. Please ensure MSYS2 is installed.")
        else:
            return 'pacman'

    def run_command(self, command):
        """
        Executes a given command using subprocess and handles both success and failure cases.
        If the platform is Windows, prepend the pacman command with the appropriate MSYS2 path.
        """
        if self.is_windows:
            command.insert(0, self.pacman_command)
        try:
            # Run the command and capture output
            result = subprocess.run(
                command, check=True, text=True, capture_output=True)
            logger.info(f"Command {' '.join(command)} executed successfully.")
            return result.stdout
        except subprocess.CalledProcessError as e:
            # Log any errors during command execution
            logger.error(f"Error executing command: {e.stderr}")
            return f"Error: {e.stderr}"

    def update_package_database(self):
        """Update the package database to get the latest package information."""
        return self.run_command(['sudo', 'pacman', '-Sy'])

    def upgrade_system(self):
        """Upgrade the system by updating all installed packages to the latest versions."""
        return self.run_command(['sudo', 'pacman', '-Syu'])

    def install_package(self, package_name):
        """Install a specific package."""
        return self.run_command(['sudo', 'pacman', '-S', package_name])

    def remove_package(self, package_name, remove_deps=False):
        """
        Remove a specific package.
        Optionally remove its dependencies if 'remove_deps' is set to True.
        """
        command = ['sudo', 'pacman', '-R', package_name]
        if remove_deps:
            command.append('--recursive')  # Add flag to remove dependencies
        return self.run_command(command)

    def search_package(self, query):
        """Search for a package by name or description."""
        return self.run_command(['pacman', '-Ss', query])

    def list_installed_packages(self):
        """List all installed packages on the system."""
        return self.run_command(['pacman', '-Q'])

    def show_package_info(self, package_name):
        """Display detailed information about a specific package."""
        return self.run_command(['pacman', '-Qi', package_name])

    def list_outdated_packages(self):
        """List all packages that are outdated and need to be upgraded."""
        return self.run_command(['pacman', '-Qu'])

    def clear_cache(self):
        """Clear the package cache to free up space."""
        return self.run_command(['sudo', 'pacman', '-Scc'])

    def list_package_files(self, package_name):
        """List all the files installed by a specific package."""
        return self.run_command(['pacman', '-Ql', package_name])

    def show_package_dependencies(self, package_name):
        """Show the dependencies of a specific package."""
        return self.run_command(['pacman', '-Qi', package_name])

    def find_file_owner(self, file_path):
        """Find which package owns a specific file."""
        return self.run_command(['pacman', '-Qo', file_path])

    def show_fastest_mirrors(self):
        """Display and select the fastest mirrors for package downloads."""
        return self.run_command(['sudo', 'pacman-mirrors', '--fasttrack'])

    def downgrade_package(self, package_name, version):
        """
        Downgrade a package to a specific version.
        This requires the version to be available in the package cache or repositories.
        """
        return self.run_command(['sudo', 'pacman', '-U', f'/var/cache/pacman/pkg/{package_name}-{version}.pkg.tar.zst'])

    def list_cache_packages(self):
        """List all packages currently stored in the local package cache."""
        return self.run_command(['ls', '/var/cache/pacman/pkg'])

    def enable_multithreaded_downloads(self):
        """Enable multithreaded downloads to speed up package installation."""
        return self.run_command(['sudo', 'sed', '-i', 's/#ParallelDownloads = 5/ParallelDownloads = 5/g', '/etc/pacman.conf'])

    def list_package_group(self, group_name):
        """List all packages in a specific package group."""
        return self.run_command(['pacman', '-Sg', group_name])

    def list_optional_dependencies(self, package_name):
        """List optional dependencies of a package."""
        return self.run_command(['pacman', '-Qi', package_name])

    def enable_color_output(self, enable=True):
        """
        Enable or disable color output in pacman command-line results.
        This improves readability of pacman commands.
        """
        if enable:
            return self.run_command(['sudo', 'sed', '-i', 's/#Color/Color/g', '/etc/pacman.conf'])
        else:
            return self.run_command(['sudo', 'sed', '-i', 's/Color/#Color/g', '/etc/pacman.conf'])


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

    # New features
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
    pacman = PacmanManager()

    # Handle command-line arguments and execute the corresponding methods
    if args.update_db:
        print(pacman.update_package_database())
    elif args.upgrade:
        print(pacman.upgrade_system())
    elif args.install:
        print(pacman.install_package(args.install))
    elif args.remove:
        print(pacman.remove_package(args.remove, remove_deps=args.remove_deps))
    elif args.search:
        print(pacman.search_package(args.search))
    elif args.list_installed:
        print(pacman.list_installed_packages())
    elif args.package_info:
        print(pacman.show_package_info(args.package_info))
    elif args.list_outdated:
        print(pacman.list_outdated_packages())

    # Handle newly added features
    elif args.clear_cache:
        print(pacman.clear_cache())
    elif args.list_files:
        print(pacman.list_package_files(args.list_files))
    elif args.show_dependencies:
        print(pacman.show_package_dependencies(args.show_dependencies))
    elif args.find_file_owner:
        print(pacman.find_file_owner(args.find_file_owner))
    elif args.fast_mirrors:
        print(pacman.show_fastest_mirrors())
    elif args.downgrade:
        package, version = args.downgrade
        print(pacman.downgrade_package(package, version))
    elif args.list_cache:
        print(pacman.list_cache_packages())
    elif args.multithread:
        print(pacman.enable_multithreaded_downloads())
    elif args.list_group:
        print(pacman.list_package_group(args.list_group))
    elif args.optional_deps:
        print(pacman.list_optional_dependencies(args.optional_deps))
    elif args.enable_color:
        print(pacman.enable_color_output(enable=True))
    elif args.disable_color:
        print(pacman.enable_color_output(enable=False))
    else:
        # If no arguments are provided, show the help message
        parser.print_help()


# Run the command-line tool
if __name__ == "__main__":
    main()
