#!/usr/bin/env python3
"""
VcpkgManager: An Enhanced Python-based Package Manager Inspired by vcpkg

This script provides a command-line interface for managing packages similarly to vcpkg.
It allows users to install, remove, update, search for packages, list installed packages,
view package information, and manage package repositories.

Usage:
    python vcpkg_manager.py <command> [<args>]

Commands:
    install <package>           : Install a package
    remove <package>            : Remove a package
    list                        : List all installed packages
    update                      : Update all installed packages
    search <query>              : Search for packages
    info <package>              : Show information about a package
    add-repo <repo_url>         : Add a new package repository
    remove-repo <repo_url>      : Remove an existing package repository
    list-repos                  : List all configured package repositories

Options:
    --root <path>               : Specify the root directory for package management (default: ~/.vcpkg)
    --verbose                   : Enable verbose logging
    --config <config_file>      : Specify a custom configuration file

Examples:
    python vcpkg_manager.py install boost
    python vcpkg_manager.py remove eigen
    python vcpkg_manager.py list
    python vcpkg_manager.py update
    python vcpkg_manager.py search matrix
    python vcpkg_manager.py info opencv
    python vcpkg_manager.py add-repo https://example.com/repo.json
    python vcpkg_manager.py --root /custom/path install boost

Note:
    This is an enhanced version of a package manager inspired by vcpkg. It includes additional
    features for better package management and repository handling. Use it responsibly and
    consider contributing to its development.
"""

import os
import json
import shutil
import argparse
from dataclasses import dataclass, field
from typing import List, Dict
import sys
from loguru import logger

try:
    from rich import print
    from rich.console import Console
    from rich.table import Table
    from rich.progress import Progress, BarColumn, TimeRemainingColumn, TextColumn
except ImportError:
    print("[red]Please install the 'rich' and 'loguru' libraries: pip install rich loguru[/red]")
    sys.exit(1)

# Initialize Rich Console
console = Console()


@dataclass
class Package:
    """
    Represents a package with its name, version, and dependencies.

    Attributes:
        name (str): The name of the package.
        version (str): The version of the package.
        dependencies (List[str]): A list of package names that this package depends on.
        description (str): A brief description of the package.
    """
    name: str
    version: str
    dependencies: List[str]
    description: str = "No description available."


@dataclass
class Repository:
    """
    Represents a package repository.

    Attributes:
        url (str): The URL of the repository.
        name (str): The name of the repository.
    """
    url: str
    name: str = field(init=False)

    def __post_init__(self):
        self.name = os.path.basename(self.url.rstrip(
            '/')).replace('.json', '').capitalize()


class VcpkgManagerError(Exception):
    """Base class for VcpkgManager exceptions"""
    pass


class PackageNotFoundError(VcpkgManagerError):
    """Raised when a package is not found"""
    pass


class InstallationError(VcpkgManagerError):
    """Raised when there's an error during package installation"""
    pass


class RepositoryError(VcpkgManagerError):
    """Raised when there's an error with repository management"""
    pass


class VcpkgManager:
    """
    A class that manages package installation, removal, updating,
    information retrieval, and repository management.

    Attributes:
        root_dir (str): The root directory for package management.
        packages_dir (str): The directory where packages are stored.
        installed_file (str): The JSON file that keeps track of installed packages.
        repos_file (str): The JSON file that keeps track of repositories.
        log_file (str): The file where logs are written.
        repositories (List[Repository]): List of configured repositories.
    """

    def __init__(self, root_dir, verbose=False, config_file=None):
        """
        Initialize the VcpkgManager with the given root directory.

        Args:
            root_dir (str): The root directory for package management.
            verbose (bool): Enable verbose logging.
            config_file (str): Path to a custom configuration file.
        """
        self.root_dir = os.path.expanduser(root_dir)
        self.packages_dir = os.path.join(self.root_dir, "packages")
        self.installed_file = os.path.join(self.root_dir, "installed.json")
        self.repos_file = os.path.join(self.root_dir, "repositories.json")
        self.log_file = os.path.join(self.root_dir, "vcpkg.log")

        # Setup logging
        logger.remove()
        log_level = "DEBUG" if verbose else "INFO"
        logger.add(self.log_file, level=log_level,
                   rotation="10 MB", compression="zip")
        logger.add(sys.stderr, level=log_level)

        # Create necessary directories and files
        self._initialize_directories()
        self.repositories = self._load_repositories()

    def _initialize_directories(self):
        """Initialize required directories and files."""
        os.makedirs(self.packages_dir, exist_ok=True)
        if not os.path.exists(self.installed_file):
            with open(self.installed_file, 'w', encoding='utf-8') as f:
                json.dump({}, f, indent=2)
        if not os.path.exists(self.repos_file):
            with open(self.repos_file, 'w', encoding='utf-8') as f:
                json.dump([], f, indent=2)

    def _load_repositories(self) -> List[Repository]:
        """Load configured repositories from the repositories file."""
        try:
            with open(self.repos_file, 'r', encoding='utf-8') as f:
                repos_data = json.load(f)
            return [Repository(**repo) for repo in repos_data]
        except Exception as e:
            logger.error("Failed to load repositories: %s", str(e))
            return []

    def _save_repositories(self):
        """Save the current list of repositories to the repositories file."""
        try:
            with open(self.repos_file, 'w', encoding='utf-8') as f:
                json.dump(
                    [repo.__dict__ for repo in self.repositories], f, indent=2)
            logger.info("Repositories saved successfully.")
        except Exception as e:
            logger.error("Failed to save repositories: %s", str(e))
            raise RepositoryError(
                f"Failed to save repositories: {str(e)}") from e

    def add_repo(self, repo_url: str):
        """
        Add a new package repository.

        Args:
            repo_url (str): The URL of the repository to add.

        Raises:
            RepositoryError: If the repository cannot be added.
        """
        try:
            if any(repo.url == repo_url for repo in self.repositories):
                console.print(
                    f"[yellow]Repository '{repo_url}' is already added.[/yellow]")
                return
            # Simulate fetching repository data
            # In a real-world scenario, you would fetch and validate the repository's package data
            new_repo = Repository(url=repo_url)
            self.repositories.append(new_repo)
            self._save_repositories()
            console.print(
                f"[green]Repository '{new_repo.name}' added successfully.[/green]")
            logger.info("Repository '%s' added.", new_repo.name)
        except Exception as e:
            logger.error("Error adding repository %s: %s", repo_url, str(e))
            raise RepositoryError(
                f"Failed to add repository '{repo_url}': {str(e)}") from e

    def remove_repo(self, repo_url: str):
        """
        Remove an existing package repository.

        Args:
            repo_url (str): The URL of the repository to remove.

        Raises:
            RepositoryError: If the repository cannot be removed.
        """
        try:
            repo_to_remove = next(
                (repo for repo in self.repositories if repo.url == repo_url), None)
            if not repo_to_remove:
                console.print(f"[red]Repository '{repo_url}' not found.[/red]")
                return
            self.repositories.remove(repo_to_remove)
            self._save_repositories()
            console.print(
                f"[green]Repository '{repo_to_remove.name}' removed successfully.[/green]")
            logger.info("Repository '%s' removed.", repo_to_remove.name)
        except Exception as e:
            logger.error("Error removing repository %s: %s", repo_url, str(e))
            raise RepositoryError(
                f"Failed to remove repository '{repo_url}': {str(e)}") from e

    def list_repos(self):
        """List all configured package repositories."""
        if not self.repositories:
            console.print("[yellow]No repositories configured.[/yellow]")
            return
        table = Table(title="Configured Repositories")
        table.add_column("Name", style="cyan", no_wrap=True)
        table.add_column("URL", style="magenta")
        for repo in self.repositories:
            table.add_row(repo.name, repo.url)
        console.print(table)

    def install(self, package_name: str, version: str = None):
        """
        Install a package and its dependencies.

        Args:
            package_name (str): The name of the package to install.
            version (str, optional): Specific version to install. Defaults to latest.

        Raises:
            InstallationError: If there's an error during the installation process.
        """
        try:
            logger.info("Installing package: %s", package_name)
            package = self._fetch_package_info(package_name, version)
            self._install_dependencies(package.dependencies)

            package_dir = os.path.join(self.packages_dir, package.name)
            os.makedirs(package_dir, exist_ok=True)

            # Simulate package installation by creating dummy files
            with open(os.path.join(package_dir, f"{package.name}.h"), 'w', encoding='utf-8') as f:
                f.write(f"// Header for {package.name} v{package.version}\n")

            with open(os.path.join(package_dir, f"{package.name}.txt"), 'w', encoding='utf-8') as f:
                f.write(
                    f"{package.name} version {package.version} installed successfully.\nDescription: {package.description}")

            # Update installed packages
            with open(self.installed_file, 'r+', encoding='utf-8') as f:
                installed = json.load(f)
                installed[package.name] = {
                    "version": package.version,
                    "dependencies": package.dependencies,
                    "description": package.description
                }
                f.seek(0)
                json.dump(installed, f, indent=2)
                f.truncate()

            console.print(
                f"[green]{package.name} v{package.version} installed successfully.[/green]")
            logger.info("Package '%s' v%s installed successfully.",
                        package.name, package.version)
        except PackageNotFoundError as e:
            console.print(f"[red]{str(e)}[/red]")
            logger.error("Installation failed: %s", str(e))
        except InstallationError as e:
            console.print(f"[red]{str(e)}[/red]")
            logger.error("Installation failed: %s", str(e))
        except Exception as e:
            console.print(
                f"[red]Unexpected error during installation: {str(e)}[/red]")
            logger.error(
                "Unexpected error during installation of %s: %s", package_name, str(e))

    def _install_dependencies(self, dependencies: List[str]):
        """
        Install all dependencies of a package.

        Args:
            dependencies (List[str]): A list of package names to install.
        """
        for dep in dependencies:
            if not self._is_installed(dep):
                console.print(f"[cyan]Installing dependency: {dep}[/cyan]")
                self.install(dep)

    def _is_installed(self, package_name: str) -> bool:
        """
        Check if a package is already installed.

        Args:
            package_name (str): The name of the package to check.

        Returns:
            bool: True if the package is installed, False otherwise.
        """
        with open(self.installed_file, 'r', encoding='utf-8') as f:
            installed = json.load(f)
        return package_name in installed

    def _fetch_package_info(self, package_name: str, version: str = None) -> Package:
        """
        Fetch information about a package from configured repositories.

        Args:
            package_name (str): The name of the package to fetch information for.
            version (str, optional): Specific version to fetch. Defaults to latest.

        Returns:
            Package: A Package object containing the package information.

        Raises:
            PackageNotFoundError: If the package is not found in any repository.
        """
        # Search through all configured repositories
        for repo in self.repositories:
            try:
                # Simulate fetching package info from repository
                # In a real-world scenario, you would perform HTTP requests to the repository's API
                package = self._simulate_repo_fetch(
                    repo, package_name, version)
                return package
            except PackageNotFoundError:
                continue
        # If not found in any repository
        raise PackageNotFoundError(
            f"Package '{package_name}' not found in any configured repository.")

    def _simulate_repo_fetch(self, repo: Repository, package_name: str, version: str = None) -> Package:
        """
        Simulate fetching package information from a repository.

        Args:
            repo (Repository): The repository to fetch from.
            package_name (str): The name of the package.
            version (str, optional): Specific version to fetch.

        Returns:
            Package: A Package object with package information.

        Raises:
            PackageNotFoundError: If the package is not found in the repository.
        """
        # Simulated repository data
        simulated_repo = {
            "boost": {
                "latest": {
                    "version": "1.76.0",
                    "dependencies": ["zlib", "bzip2"],
                    "description": "Boost C++ Libraries"
                },
                "1.75.0": {
                    "version": "1.75.0",
                    "dependencies": ["zlib", "bzip2"],
                    "description": "Boost C++ Libraries"
                }
            },
            "eigen": {
                "latest": {
                    "version": "3.4.0",
                    "dependencies": [],
                    "description": "C++ Template Library for Linear Algebra"
                }
            },
            "zlib": {
                "latest": {
                    "version": "1.2.11",
                    "dependencies": [],
                    "description": "Compression Library"
                }
            },
            "bzip2": {
                "latest": {
                    "version": "1.0.8",
                    "dependencies": [],
                    "description": "Compression Utility"
                }
            },
            "opencv": {
                "latest": {
                    "version": "4.5.3",
                    "dependencies": ["zlib"],
                    "description": "Open Source Computer Vision Library"
                }
            }
        }

        packages = simulated_repo.get(package_name.lower())
        if not packages:
            raise PackageNotFoundError(
                f"Package '{package_name}' not found in repository '{repo.name}'.")

        if version:
            pkg_info = packages.get(version)
            if not pkg_info:
                raise PackageNotFoundError(
                    f"Version '{version}' of package '{package_name}' not found in repository '{repo.name}'.")
        else:
            pkg_info = packages.get("latest")
            if not pkg_info:
                raise PackageNotFoundError(
                    f"Latest version of package '{package_name}' not found in repository '{repo.name}'.")

        return Package(
            name=package_name,
            version=pkg_info["version"],
            dependencies=pkg_info["dependencies"],
            description=pkg_info["description"]
        )

    def remove(self, package_name: str):
        """
        Remove an installed package.

        Args:
            package_name (str): The name of the package to remove.

        Raises:
            PackageNotFoundError: If the package is not installed.
            InstallationError: If there's an error during the removal process.
        """
        try:
            logger.info("Removing package: %s", package_name)
            package_dir = os.path.join(self.packages_dir, package_name)
            if os.path.exists(package_dir):
                # Check if other packages depend on this package
                dependents = self._find_dependents(package_name)
                if dependents:
                    console.print(
                        f"[red]Cannot remove '{package_name}' as it is required by: {', '.join(dependents)}[/red]")
                    logger.warning(
                        "Removal failed: '%s' is required by %s", package_name, dependents)
                    return

                shutil.rmtree(package_dir)

                with open(self.installed_file, 'r+', encoding='utf-8') as f:
                    installed = json.load(f)
                    if package_name in installed:
                        del installed[package_name]
                        f.seek(0)
                        json.dump(installed, f, indent=2)
                        f.truncate()

                console.print(
                    f"[green]{package_name} removed successfully.[/green]")
                logger.info("Package '%s' removed successfully.", package_name)
            else:
                raise PackageNotFoundError(
                    f"Package '{package_name}' is not installed.")
        except PackageNotFoundError as e:
            console.print(f"[red]{str(e)}[/red]")
            logger.error("Removal failed: %s", str(e))
        except InstallationError as e:
            console.print(f"[red]{str(e)}[/red]")
            logger.error("Removal failed: %s", str(e))
        except Exception as e:
            console.print(
                f"[red]Unexpected error during removal: {str(e)}[/red]")
            logger.error("Unexpected error during removal of %s: %s",
                         package_name, str(e))

    def _find_dependents(self, package_name: str) -> List[str]:
        """
        Find all installed packages that depend on the given package.

        Args:
            package_name (str): The package to check dependencies for.

        Returns:
            List[str]: A list of package names that depend on the given package.
        """
        dependents = []
        with open(self.installed_file, 'r', encoding='utf-8') as f:
            installed = json.load(f)
        for pkg, info in installed.items():
            if package_name in [dep.lower() for dep in info.get("dependencies", [])]:
                dependents.append(pkg)
        return dependents

    def list_installed(self):
        """
        List all installed packages and their versions.
        """
        try:
            with open(self.installed_file, 'r', encoding='utf-8') as f:
                installed = json.load(f)

            if installed:
                table = Table(title="Installed Packages")
                table.add_column("Package Name", style="cyan", no_wrap=True)
                table.add_column("Version", style="magenta")
                table.add_column("Description", style="green")

                for package, info in installed.items():
                    table.add_row(package, info['version'], info.get(
                        'description', 'No description'))
                console.print(table)
            else:
                console.print("[yellow]No packages installed.[/yellow]")
        except Exception as e:
            logger.error("Error listing installed packages: %s", str(e))
            console.print(
                f"[red]Error listing installed packages: {str(e)}[/red]")

    def update(self):
        """
        Update all installed packages to their latest versions.
        """
        try:
            logger.info("Updating installed packages")
            console.print("[bold]Updating installed packages...[/bold]")
            with open(self.installed_file, 'r', encoding='utf-8') as f:
                installed = json.load(f)

            for package in list(installed.keys()):
                try:
                    console.print(f"Checking for updates: {package}...")
                    current_version = installed[package]["version"]
                    latest_package = self._fetch_package_info(package)
                    if self._is_newer_version(latest_package.version, current_version):
                        console.print(
                            f"[cyan]Updating {package} from v{current_version} to v{latest_package.version}[/cyan]")
                        self.remove(package)
                        self.install(package)
                    else:
                        console.print(
                            f"[green]{package} is already up to date (v{current_version}).[/green]")
                except PackageNotFoundError:
                    console.print(
                        f"[yellow]Package '{package}' not found in repositories. Skipping update.[/yellow]")
                except InstallationError as e:
                    console.print(
                        f"[red]Failed to update '{package}': {str(e)}[/red]")
                    logger.error("Failed to update '%s': %s", package, str(e))

            console.print(
                "[bold green]All packages are up to date.[/bold green]")
            logger.info("All packages updated successfully.")
        except Exception as e:
            logger.error("Error updating packages: %s", str(e))
            console.print(f"[red]Error updating packages: {str(e)}[/red]")

    def _is_newer_version(self, latest: str, current: str) -> bool:
        """
        Compare two version strings to determine if the latest is newer than the current.

        Args:
            latest (str): The latest version string.
            current (str): The current version string.

        Returns:
            bool: True if latest is newer than current, False otherwise.
        """
        from packaging import version
        return version.parse(latest) > version.parse(current)

    def search(self, query: str):
        """
        Search for packages matching the given query across all repositories.

        Args:
            query (str): The search query string.
        """
        try:
            logger.info("Searching for packages matching '%s'", query)
            console.print(f"Searching for packages matching '{query}'...")
            results = []

            # Simulate searching through all repositories
            for repo in self.repositories:
                # In a real-world scenario, perform API calls to search packages
                # Here, we simulate search results
                simulated_search = self._simulate_repo_search(repo, query)
                results.extend(simulated_search)

            if results:
                table = Table(title="Search Results")
                table.add_column("Package Name", style="cyan", no_wrap=True)
                table.add_column("Version", style="magenta")
                table.add_column("Description", style="green")

                for result in results:
                    table.add_row(
                        result['name'], result['version'], result['description'])
                console.print(table)
            else:
                console.print(
                    "[yellow]No packages found matching your query.[/yellow]")
        except Exception as e:
            logger.error("Error searching for packages: %s", str(e))
            console.print(f"[red]Error searching for packages: {str(e)}[/red]")

    def _simulate_repo_search(self, repo: Repository, query: str) -> List[Dict]:
        """
        Simulate searching for packages within a repository.

        Args:
            repo (Repository): The repository to search in.
            query (str): The search query.

        Returns:
            List[Dict]: A list of matching package dictionaries.
        """
        # Simulated package data
        simulated_packages = {
            "boost": {
                "name": "boost",
                "version": "1.76.0",
                "description": "Boost C++ Libraries"
            },
            "eigen": {
                "name": "eigen",
                "version": "3.4.0",
                "description": "C++ Template Library for Linear Algebra"
            },
            "zlib": {
                "name": "zlib",
                "version": "1.2.11",
                "description": "Compression Library"
            },
            "bzip2": {
                "name": "bzip2",
                "version": "1.0.8",
                "description": "Compression Utility"
            },
            "opencv": {
                "name": "opencv",
                "version": "4.5.3",
                "description": "Open Source Computer Vision Library"
            },
            "matrixlib": {
                "name": "matrixlib",
                "version": "2.0.0",
                "description": "Matrix Operations Library"
            }
        }

        matching_packages = []
        for pkg in simulated_packages.values():
            if query.lower() in pkg['name'].lower() or query.lower() in pkg['description'].lower():
                matching_packages.append(pkg)
        return matching_packages

    def info(self, package_name: str):
        """
        Display detailed information about a specific package.

        Args:
            package_name (str): The name of the package to display information for.
        """
        try:
            with open(self.installed_file, 'r', encoding='utf-8') as f:
                installed = json.load(f)

            if package_name in installed:
                info = installed[package_name]
                table = Table(title=f"Information for {package_name}")
                table.add_column("Attribute", style="cyan", no_wrap=True)
                table.add_column("Details", style="magenta")

                table.add_row("Package Name", package_name)
                table.add_row("Version", info['version'])
                table.add_row("Description", info.get(
                    'description', 'No description available.'))
                table.add_row("Dependencies", ", ".join(
                    info['dependencies']) if info['dependencies'] else "None")
                table.add_row("Status", "Installed")

                console.print(table)
            else:
                console.print(
                    f"[yellow]Package '{package_name}' is not installed.[/yellow]")
                console.print("Fetching online information...")
                try:
                    package = self._fetch_package_info(package_name)
                    table = Table(title=f"Information for {package_name}")
                    table.add_column("Attribute", style="cyan", no_wrap=True)
                    table.add_column("Details", style="magenta")

                    table.add_row("Package Name", package.name)
                    table.add_row("Version", package.version)
                    table.add_row("Description", package.description)
                    table.add_row("Dependencies", ", ".join(
                        package.dependencies) if package.dependencies else "None")
                    table.add_row("Status", "Not Installed")

                    console.print(table)
                except PackageNotFoundError:
                    console.print(
                        f"[red]No information found for package '{package_name}'.[/red]")
        except Exception as e:
            logger.error("Error retrieving package info: %s", str(e))
            console.print(
                f"[red]Error retrieving package info: {str(e)}[/red]")


def main():
    """
    Main function to handle command-line arguments and execute corresponding actions.
    """
    default_root = os.path.join(os.path.expanduser("~"), ".vcpkg")
    parser = argparse.ArgumentParser(
        description="vcpkg-like Package Manager",
        formatter_class=argparse.RawTextHelpFormatter,
        epilog="""
Examples:
    Install a package:
        python vcpkg_manager.py install boost

    Remove a package:
        python vcpkg_manager.py remove eigen

    List installed packages:
        python vcpkg_manager.py list

    Update all installed packages:
        python vcpkg_manager.py update

    Search for packages:
        python vcpkg_manager.py search matrix

    Show package information:
        python vcpkg_manager.py info opencv

    Add a new repository:
        python vcpkg_manager.py add-repo https://example.com/repo.json

    Remove an existing repository:
        python vcpkg_manager.py remove-repo https://example.com/repo.json

    List all repositories:
        python vcpkg_manager.py list-repos

    Specify a custom root directory and enable verbose logging:
        python vcpkg_manager.py --root /custom/path --verbose install boost
"""
    )

    parser.add_argument("--root", default=default_root,
                        help=f"Root directory for vcpkg (default: {default_root})")
    parser.add_argument("--verbose", action="store_true",
                        help="Enable verbose logging")
    parser.add_argument("--config", type=str,
                        help="Specify a custom configuration file")

    subparsers = parser.add_subparsers(
        dest="command", help="Available commands")

    # Install command
    install_parser = subparsers.add_parser("install", help="Install a package")
    install_parser.add_argument("package", help="Package name to install")
    install_parser.add_argument(
        "--version", type=str, help="Specify package version to install")

    # Remove command
    remove_parser = subparsers.add_parser("remove", help="Remove a package")
    remove_parser.add_argument("package", help="Package name to remove")

    # List command
    subparsers.add_parser("list", help="List installed packages")

    # Update command
    subparsers.add_parser("update", help="Update all installed packages")

    # Search command
    search_parser = subparsers.add_parser("search", help="Search for packages")
    search_parser.add_argument("query", help="Search query")

    # Info command
    info_parser = subparsers.add_parser(
        "info", help="Show package information")
    info_parser.add_argument("package", help="Package name")

    # Add Repository command
    add_repo_parser = subparsers.add_parser(
        "add-repo", help="Add a new package repository")
    add_repo_parser.add_argument(
        "repo_url", help="URL of the repository to add")

    # Remove Repository command
    remove_repo_parser = subparsers.add_parser(
        "remove-repo", help="Remove an existing package repository")
    remove_repo_parser.add_argument(
        "repo_url", help="URL of the repository to remove")

    # List Repositories command
    subparsers.add_parser(
        "list-repos", help="List all configured package repositories")

    args = parser.parse_args()

    manager = VcpkgManager(
        root_dir=args.root, verbose=args.verbose, config_file=args.config)

    try:
        if args.command == "install":
            manager.install(args.package, version=args.version)
        elif args.command == "remove":
            manager.remove(args.package)
        elif args.command == "list":
            manager.list_installed()
        elif args.command == "update":
            manager.update()
        elif args.command == "search":
            manager.search(args.query)
        elif args.command == "info":
            manager.info(args.package)
        elif args.command == "add-repo":
            manager.add_repo(args.repo_url)
        elif args.command == "remove-repo":
            manager.remove_repo(args.repo_url)
        elif args.command == "list-repos":
            manager.list_repos()
        else:
            parser.print_help()
    except VcpkgManagerError as e:
        console.print(f"[red]Error: {str(e)}[/red]")
        sys.exit(1)


if __name__ == "__main__":
    main()
