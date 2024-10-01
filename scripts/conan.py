import subprocess
import argparse

# Helper function to run system commands


def run_command(command: str) -> bool:
    """
    Run a system command and return whether it was successful.

    Args:
        command (str): The system command to run.

    Returns:
        bool: True if the command was successful, False otherwise.
    """
    result = subprocess.run(command, shell=True, text=True, check=False)
    if result.returncode != 0:
        print(f"Error: Command '{command}' failed with exit code {
              result.returncode}")
    return result.returncode == 0

# Install dependencies with optional configuration


def install_dependencies(build_type: str = "Release"):
    """
    Install dependencies with optional configuration.

    Args:
        build_type (str): The build type (e.g., Release, Debug). Default is "Release".

    Returns:
        bool: True if the installation was successful, False otherwise.
    """
    print(f"Installing dependencies for {build_type}...")
    return run_command(f"conan install . --build=missing -s build_type={build_type}")

# Uninstall dependencies


def uninstall_dependencies():
    """
    Uninstall dependencies by cleaning the build folder.

    Returns:
        bool: True if the uninstallation was successful, False otherwise.
    """
    print("Uninstalling dependencies (cleaning build folder)...")
    return run_command("rm -rf build")

# Update dependencies with optional configuration


def update_dependencies(build_type: str = "Release"):
    """
    Update dependencies with optional configuration.

    Args:
        build_type (str): The build type (e.g., Release, Debug). Default is "Release".

    Returns:
        bool: True if the update was successful, False otherwise.
    """
    print(f"Updating dependencies for {build_type}...")
    return run_command(f"conan install . --update --build=missing -s build_type={build_type}")

# Clean build files


def clean_build():
    """
    Clean build files.

    Returns:
        bool: True if the cleaning was successful, False otherwise.
    """
    print("Cleaning build files...")
    return run_command("rm -rf build CMakeFiles CMakeCache.txt")

# Show dependency information


def show_dependency_info():
    """
    Display the dependency tree information.

    Returns:
        bool: True if the information was displayed successfully, False otherwise.
    """
    print("Displaying dependency tree...")
    return run_command("conan info .")

# Add a remote repository


def add_remote(remote_name: str, remote_url: str):
    """
    Add a remote repository.

    Args:
        remote_name (str): The name of the remote repository.
        remote_url (str): The URL of the remote repository.

    Returns:
        bool: True if the remote was added successfully, False otherwise.
    """
    print(f"Adding remote: {remote_name} -> {remote_url}")
    return run_command(f"conan remote add {remote_name} {remote_url}")

# List all remote repositories


def list_remotes():
    """
    List all remote repositories.

    Returns:
        bool: True if the remotes were listed successfully, False otherwise.
    """
    print("Listing all remotes...")
    return run_command("conan remote list")

# Remove a remote repository


def remove_remote(remote_name: str):
    """
    Remove a remote repository.

    Args:
        remote_name (str): The name of the remote repository to remove.

    Returns:
        bool: True if the remote was removed successfully, False otherwise.
    """
    print(f"Removing remote: {remote_name}")
    return run_command(f"conan remote remove {remote_name}")

# Build the project


def build_project(build_type: str = "Release"):
    """
    Build the project with the specified build type.

    Args:
        build_type (str): The build type (e.g., Release, Debug). Default is "Release".

    Returns:
        bool: True if the build was successful, False otherwise.
    """
    print(f"Building project in {build_type} mode...")
    return run_command(f"cmake -DCMAKE_BUILD_TYPE={build_type} . && cmake --build .")

# Run unit tests


def run_tests():
    """
    Run unit tests.

    Returns:
        bool: True if the tests ran successfully, False otherwise.
    """
    print("Running tests...")
    return run_command("ctest")

# Generate project documentation (assuming Doxygen is used)


def generate_docs():
    """
    Generate project documentation.

    Returns:
        bool: True if the documentation was generated successfully, False otherwise.
    """
    print("Generating project documentation...")
    return run_command("doxygen Doxyfile")

# Main function to handle command-line arguments using argparse


def main():
    """
    Main function to handle command-line arguments and execute corresponding functions.
    """
    parser = argparse.ArgumentParser(
        description="Conan Dependency Management Script")
    subparsers = parser.add_subparsers(dest="command")

    # install command
    parser_install = subparsers.add_parser(
        "install", help="Install dependencies")
    parser_install.add_argument(
        "--build-type", default="Release", help="Specify build type (Release/Debug)")

    # uninstall command
    subparsers.add_parser("uninstall", help="Uninstall dependencies")

    # update command
    parser_update = subparsers.add_parser("update", help="Update dependencies")
    parser_update.add_argument(
        "--build-type", default="Release", help="Specify build type (Release/Debug)")

    # clean command
    subparsers.add_parser("clean", help="Clean build files")

    # info command
    subparsers.add_parser("info", help="Show dependency tree information")

    # add-remote command
    parser_add_remote = subparsers.add_parser(
        "add-remote", help="Add a new remote repository")
    parser_add_remote.add_argument(
        "remote_name", help="The name of the remote repository")
    parser_add_remote.add_argument(
        "remote_url", help="The URL of the remote repository")

    # list-remotes command
    subparsers.add_parser("list-remotes", help="List all remote repositories")

    # remove-remote command
    parser_remove_remote = subparsers.add_parser(
        "remove-remote", help="Remove a remote repository")
    parser_remove_remote.add_argument(
        "remote_name", help="The name of the remote repository to remove")

    # build command
    parser_build = subparsers.add_parser("build", help="Build the project")
    parser_build.add_argument(
        "--build-type", default="Release", help="Specify build type (Release/Debug)")

    # test command
    subparsers.add_parser("test", help="Run unit tests")

    # docs command
    subparsers.add_parser("docs", help="Generate project documentation")

    # Parse arguments
    args = parser.parse_args()

    # Execute corresponding function based on the command
    if args.command == "install":
        install_dependencies(args.build_type)
    elif args.command == "uninstall":
        uninstall_dependencies()
    elif args.command == "update":
        update_dependencies(args.build_type)
    elif args.command == "clean":
        clean_build()
    elif args.command == "info":
        show_dependency_info()
    elif args.command == "add-remote":
        add_remote(args.remote_name, args.remote_url)
    elif args.command == "list-remotes":
        list_remotes()
    elif args.command == "remove-remote":
        remove_remote(args.remote_name)
    elif args.command == "build":
        build_project(args.build_type)
    elif args.command == "test":
        run_tests()
    elif args.command == "docs":
        generate_docs()
    else:
        parser.print_help()


if __name__ == "__main__":
    main()
