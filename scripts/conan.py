"""
Advanced Conan Dependency Management Tool with Rich Enhancements
"""

import subprocess
import argparse
from typing import Optional
from dataclasses import dataclass
from rich.console import Console
from rich.progress import Progress, SpinnerColumn, TextColumn
from rich.table import Table
from rich import box

console = Console()


def run_command(command: str) -> bool:
    """
    Execute a system command and return whether it was successful.

    Args:
        command (str): The system command to execute.

    Returns:
        bool: True if the command was successful, False otherwise.
    """
    try:
        result = subprocess.run(command, shell=True, text=True, check=True)
        console.log(f"[green]Command succeeded:[/green] {command}")
        return True
    except subprocess.CalledProcessError as e:
        console.log(f"[red]Command failed:[/red] {command}")
        console.log(f"[red]Error Output:[/red] {e.stderr.strip()}")
        return False


def install_dependencies(build_type: str = "Release") -> bool:
    """
    Install dependencies with an optional build configuration.

    Args:
        build_type (str, optional): The build type (e.g., Release, Debug). Defaults to "Release".

    Returns:
        bool: True if installation was successful, False otherwise.
    """
    console.print(
        f"[bold green]Installing dependencies for {build_type}...[/bold green]")
    with Progress(SpinnerColumn(), TextColumn("[progress.description]{task.description}")) as progress:
        task = progress.add_task("Installing dependencies...", start=True)
        success = run_command(
            f"conan install . --build=missing -s build_type={build_type}")
        progress.update(
            task, description="Installation completed.", completed=100)
    if success:
        console.print(
            f"[green]Dependencies installed successfully for {build_type} build.[/green]")
    else:
        console.print(
            f"[red]Failed to install dependencies for {build_type} build.[/red]")
    return success


def uninstall_dependencies() -> bool:
    """
    Uninstall dependencies by removing the build directory.

    Returns:
        bool: True if uninstallation was successful, False otherwise.
    """
    console.print(
        "[bold red]Uninstalling dependencies (cleaning build folder)...[/bold red]")
    with Progress(SpinnerColumn(), TextColumn("[progress.description]{task.description}")) as progress:
        task = progress.add_task("Uninstalling dependencies...", start=True)
        success = run_command("rm -rf build")
        progress.update(
            task, description="Uninstallation completed.", completed=100)
    if success:
        console.print("[green]Dependencies uninstalled successfully.[/green]")
    else:
        console.print("[red]Failed to uninstall dependencies.[/red]")
    return success


def update_dependencies(build_type: str = "Release") -> bool:
    """
    Update dependencies with an optional build configuration.

    Args:
        build_type (str, optional): The build type (e.g., Release, Debug). Defaults to "Release".

    Returns:
        bool: True if update was successful, False otherwise.
    """
    console.print(
        f"[bold yellow]Updating dependencies for {build_type}...[/bold yellow]")
    with Progress(SpinnerColumn(), TextColumn("[progress.description]{task.description}")) as progress:
        task = progress.add_task("Updating dependencies...", start=True)
        success = run_command(
            f"conan install . --update --build=missing -s build_type={build_type}")
        progress.update(task, description="Update completed.", completed=100)
    if success:
        console.print(
            f"[green]Dependencies updated successfully for {build_type} build.[/green]")
    else:
        console.print(
            f"[red]Failed to update dependencies for {build_type} build.[/red]")
    return success


def clean_build() -> bool:
    """
    Clean build files by removing specific directories and files.

    Returns:
        bool: True if cleaning was successful, False otherwise.
    """
    console.print("[bold magenta]Cleaning build files...[/bold magenta]")
    with Progress(SpinnerColumn(), TextColumn("[progress.description]{task.description}")) as progress:
        task = progress.add_task("Cleaning build files...", start=True)
        success = run_command("rm -rf build CMakeFiles CMakeCache.txt")
        progress.update(task, description="Cleaning completed.", completed=100)
    if success:
        console.print("[green]Build files cleaned successfully.[/green]")
    else:
        console.print("[red]Failed to clean build files.[/red]")
    return success


def show_dependency_info() -> bool:
    """
    Display the dependency tree information using Conan.

    Returns:
        bool: True if the information was displayed successfully, False otherwise.
    """
    console.print("[bold blue]Displaying dependency tree...[/bold blue]")
    with Progress(SpinnerColumn(), TextColumn("[progress.description]{task.description}")) as progress:
        task = progress.add_task("Fetching dependency info...", start=True)
        success = run_command("conan info .")
        progress.update(
            task, description="Dependency info displayed.", completed=100)
    if success:
        console.print(
            "[green]Dependency tree information displayed successfully.[/green]")
    else:
        console.print(
            "[red]Failed to display dependency tree information.[/red]")
    return success


def add_remote(remote_name: str, remote_url: str) -> bool:
    """
    Add a new remote repository to Conan.

    Args:
        remote_name (str): The name of the remote repository.
        remote_url (str): The URL of the remote repository.

    Returns:
        bool: True if the remote was added successfully, False otherwise.
    """
    console.print(
        f"[bold green]Adding remote: {remote_name} -> {remote_url}[/bold green]")
    with Progress(SpinnerColumn(), TextColumn("[progress.description]{task.description}")) as progress:
        task = progress.add_task("Adding remote repository...", start=True)
        success = run_command(f"conan remote add {remote_name} {remote_url}")
        progress.update(
            task, description="Remote repository added.", completed=100)
    if success:
        console.print(
            f"[green]Remote '{remote_name}' added successfully.[/green]")
    else:
        console.print(f"[red]Failed to add remote '{remote_name}'.[/red]")
    return success


def list_remotes() -> bool:
    """
    List all remote repositories configured in Conan.

    Returns:
        bool: True if remotes were listed successfully, False otherwise.
    """
    console.print("[bold cyan]Listing all remote repositories...[/bold cyan]")
    with Progress(SpinnerColumn(), TextColumn("[progress.description]{task.description}")) as progress:
        task = progress.add_task("Listing remotes...", start=True)
        success = run_command("conan remote list")
        progress.update(task, description="Remotes listed.", completed=100)
    if success:
        console.print(
            "[green]Remote repositories listed successfully.[/green]")
    else:
        console.print("[red]Failed to list remote repositories.[/red]")
    return success


def remove_remote(remote_name: str) -> bool:
    """
    Remove an existing remote repository from Conan.

    Args:
        remote_name (str): The name of the remote repository to remove.

    Returns:
        bool: True if the remote was removed successfully, False otherwise.
    """
    console.print(f"[bold red]Removing remote: {remote_name}[/bold red]")
    with Progress(SpinnerColumn(), TextColumn("[progress.description]{task.description}")) as progress:
        task = progress.add_task("Removing remote repository...", start=True)
        success = run_command(f"conan remote remove {remote_name}")
        progress.update(
            task, description="Remote repository removed.", completed=100)
    if success:
        console.print(
            f"[green]Remote '{remote_name}' removed successfully.[/green]")
    else:
        console.print(f"[red]Failed to remove remote '{remote_name}'.[/red]")
    return success


def build_project(build_type: str = "Release") -> bool:
    """
    Build the project with the specified build type using CMake.

    Args:
        build_type (str, optional): The build type (e.g., Release, Debug). Defaults to "Release".

    Returns:
        bool: True if the build was successful, False otherwise.
    """
    console.print(
        f"[bold green]Building project in {build_type} mode...[/bold green]")
    with Progress(SpinnerColumn(), TextColumn("[progress.description]{task.description}")) as progress:
        task = progress.add_task("Building project...", start=True)
        success = run_command(
            f"cmake -DCMAKE_BUILD_TYPE={build_type} . && cmake --build .")
        progress.update(task, description="Build completed.", completed=100)
    if success:
        console.print(
            f"[green]Project built successfully in {build_type} mode.[/green]")
    else:
        console.print(
            f"[red]Failed to build project in {build_type} mode.[/red]")
    return success


def run_tests() -> bool:
    """
    Run unit tests using CTest.

    Returns:
        bool: True if the tests ran successfully, False otherwise.
    """
    console.print("[bold blue]Running unit tests...[/bold blue]")
    with Progress(SpinnerColumn(), TextColumn("[progress.description]{task.description}")) as progress:
        task = progress.add_task("Executing tests...", start=True)
        success = run_command("ctest")
        progress.update(task, description="Tests completed.", completed=100)
    if success:
        console.print("[green]All unit tests passed successfully.[/green]")
    else:
        console.print("[red]Some unit tests failed.[/red]")
    return success


def generate_docs() -> bool:
    """
    Generate project documentation using Doxygen.

    Returns:
        bool: True if documentation was generated successfully, False otherwise.
    """
    console.print(
        "[bold magenta]Generating project documentation...[/bold magenta]")
    with Progress(SpinnerColumn(), TextColumn("[progress.description]{task.description}")) as progress:
        task = progress.add_task("Generating documentation...", start=True)
        success = run_command("doxygen Doxyfile")
        progress.update(
            task, description="Documentation generated.", completed=100)
    if success:
        console.print("[green]Documentation generated successfully.[/green]")
    else:
        console.print("[red]Failed to generate documentation.[/red]")
    return success


def main() -> None:
    """
    Main function to parse command-line arguments and execute corresponding functions.
    """
    parser = argparse.ArgumentParser(
        description="Conan Dependency Management Script with Rich Enhancements"
    )
    subparsers = parser.add_subparsers(
        dest="command", help="Available commands")

    # Install command
    parser_install = subparsers.add_parser(
        "install", help="Install dependencies")
    parser_install.add_argument(
        "--build-type", type=str, default="Release", choices=["Release", "Debug"],
        help="Specify build type (Release/Debug). Default is Release."
    )

    # Uninstall command
    subparsers.add_parser("uninstall", help="Uninstall dependencies")

    # Update command
    parser_update = subparsers.add_parser("update", help="Update dependencies")
    parser_update.add_argument(
        "--build-type", type=str, default="Release", choices=["Release", "Debug"],
        help="Specify build type (Release/Debug). Default is Release."
    )

    # Clean command
    subparsers.add_parser("clean", help="Clean build files")

    # Info command
    subparsers.add_parser("info", help="Show dependency tree information")

    # Add-remote command
    parser_add_remote = subparsers.add_parser(
        "add-remote", help="Add a new remote repository")
    parser_add_remote.add_argument(
        "remote_name", type=str, help="Name of the remote repository")
    parser_add_remote.add_argument(
        "remote_url", type=str, help="URL of the remote repository")

    # List-remotes command
    subparsers.add_parser("list-remotes", help="List all remote repositories")

    # Remove-remote command
    parser_remove_remote = subparsers.add_parser(
        "remove-remote", help="Remove a remote repository")
    parser_remove_remote.add_argument(
        "remote_name", type=str, help="Name of the remote repository to remove")

    # Build command
    parser_build = subparsers.add_parser("build", help="Build the project")
    parser_build.add_argument(
        "--build-type", type=str, default="Release", choices=["Release", "Debug"],
        help="Specify build type (Release/Debug). Default is Release."
    )

    # Test command
    subparsers.add_parser("test", help="Run unit tests")

    # Docs command
    subparsers.add_parser("docs", help="Generate project documentation")

    # Parse arguments
    args = parser.parse_args()

    # Execute corresponding function based on the command
    if args.command == "install":
        install_dependencies(build_type=args.build_type)
    elif args.command == "uninstall":
        uninstall_dependencies()
    elif args.command == "update":
        update_dependencies(build_type=args.build_type)
    elif args.command == "clean":
        clean_build()
    elif args.command == "info":
        show_dependency_info()
    elif args.command == "add-remote":
        add_remote(remote_name=args.remote_name, remote_url=args.remote_url)
    elif args.command == "list-remotes":
        list_remotes()
    elif args.command == "remove-remote":
        remove_remote(remote_name=args.remote_name)
    elif args.command == "build":
        build_project(build_type=args.build_type)
    elif args.command == "test":
        run_tests()
    elif args.command == "docs":
        generate_docs()
    else:
        parser.print_help()


if __name__ == "__main__":
    main()
