"""
Advanced Conda Environment Management Tool
"""

import sys
import subprocess
import json
import argparse
from loguru import logger


def run_command(command):
    """Run a shell command and return the output with error handling and logging."""
    result = subprocess.run(command, shell=True,
                            capture_output=True, text=True, check=True)
    if result.returncode != 0:
        logger.error(f"Command failed: {result.stderr}")
        return None
    logger.debug(f"Command succeeded: {result.stdout}")
    return result.stdout.strip()


def list_environments():
    """List all available Conda environments."""
    envs = run_command("conda env list --json")
    if envs:
        try:
            return json.loads(envs)["envs"]
        except json.JSONDecodeError as e:
            logger.error(f"Failed to parse environment list: {e}")
            return []
    return []


def create_environment(env_name, packages=None):
    """Create a new Conda environment with optional specified packages."""
    cmd = f"conda create -n {env_name} -y"
    if packages:
        cmd += f" {' '.join(packages)}"
    return run_command(cmd)


def remove_environment(env_name):
    """Remove a specified Conda environment."""
    return run_command(f"conda env remove -n {env_name} -y")


def install_package(package_name, env_name=None):
    """Install a package in a specified Conda environment."""
    cmd = f"conda install {package_name} -y"
    if env_name:
        cmd = f"conda run -n {env_name} {cmd}"
    return run_command(cmd)


def list_packages(env_name=None):
    """List packages in the specified or current Conda environment."""
    cmd = "conda list"
    if env_name:
        cmd = f"conda run -n {env_name} {cmd}"
    return run_command(cmd)


def update_package(package_name, env_name=None):
    """Update a package in the specified or current Conda environment."""
    cmd = f"conda update {package_name} -y"
    if env_name:
        cmd = f"conda run -n {env_name} {cmd}"
    return run_command(cmd)


def export_environment(env_name, output_file):
    """Export the specified environment to a YAML file."""
    cmd = f"conda env export -n {env_name} > {output_file}"
    return run_command(cmd)


def import_environment(input_file, env_name):
    """Create a Conda environment from a provided YAML file."""
    cmd = f"conda env create -f {input_file} -n {env_name}"
    return run_command(cmd)


def update_all_packages(env_name=None):
    """Update all packages in the specified or current Conda environment."""
    cmd = "conda update --all -y"
    if env_name:
        cmd = f"conda run -n {env_name} {cmd}"
    return run_command(cmd)


def clone_environment(source_env, dest_env):
    """Clone an existing Conda environment to a new environment name."""
    cmd = f"conda create --name {dest_env} --clone {source_env} -y"
    return run_command(cmd)


def handle_list_command():
    """Handle the 'list' command."""
    logger.info("Listing Conda environments...")
    environments = list_environments()
    for env in environments:
        logger.info(env)


def handle_create_command(args):
    """Handle the 'create' command."""
    logger.info(f"Creating environment '{args.name}'...")
    result = create_environment(args.name, args.packages)
    logger.info(result if result else f"Environment '{
                args.name}' created successfully")


def handle_remove_command(args):
    """Handle the 'remove' command."""
    logger.info(f"Removing environment '{args.name}'...")
    result = remove_environment(args.name)
    logger.info(result if result else f"Environment '{
                args.name}' removed successfully")


def handle_install_command(args):
    """Handle the 'install' command."""
    logger.info(f"Installing package '{args.package}'...")
    result = install_package(args.package, args.env)
    logger.info(result if result else f"Package '{
                args.package}' installed successfully")


def handle_list_packages_command(args):
    """Handle the 'list-packages' command."""
    logger.info("Listing packages...")
    result = list_packages(args.env)
    logger.info(result if result else f"No packages found in environment '{
                args.env or 'current'}'")


def handle_update_command(args):
    """Handle the 'update' command."""
    logger.info(f"Updating package '{args.package}'...")
    result = update_package(args.package, args.env)
    logger.info(result if result else f"Package '{
                args.package}' updated successfully")


def handle_export_command(args):
    """Handle the 'export' command."""
    logger.info(f"Exporting environment '{args.name}'...")
    result = export_environment(args.name, args.output)
    logger.info(result if result else f"Environment '{
                args.name}' exported to '{args.output}' successfully")


def handle_import_command(args):
    """Handle the 'import' command."""
    logger.info(f"Importing environment from '{args.input}'...")
    result = import_environment(args.input, args.name)
    logger.info(result if result else f"Environment '{
                args.name}' imported successfully")


def handle_update_all_command(args):
    """Handle the 'update-all' command."""
    logger.info("Updating all packages...")
    result = update_all_packages(args.env)
    logger.info(result if result else "All packages updated successfully")


def handle_clone_command(args):
    """Handle the 'clone' command."""
    logger.info(f"Cloning environment '{
                args.source}' to '{args.destination}'...")
    result = clone_environment(args.source, args.destination)
    logger.info(result if result else f"Environment '{
                args.source}' cloned to '{args.destination}' successfully")


def main():
    """Main entry point for the Conda environment management tool."""
    parser = argparse.ArgumentParser(
        description="Advanced Conda Environment Management Tool")
    subparsers = parser.add_subparsers(
        dest="command", help="Available commands")

    # Define subparsers for various commands
    subparsers.add_parser("list", help="List all Conda environments")

    create_parser = subparsers.add_parser(
        "create", help="Create a new Conda environment")
    create_parser.add_argument("name", help="Environment name")
    create_parser.add_argument(
        "-p", "--packages", nargs="+", help="Packages to install")

    remove_parser = subparsers.add_parser(
        "remove", help="Remove a Conda environment")
    remove_parser.add_argument("name", help="Environment name")

    install_parser = subparsers.add_parser(
        "install", help="Install a package in an environment")
    install_parser.add_argument("package", help="Package name")
    install_parser.add_argument("-e", "--env", help="Environment name")

    list_packages_parser = subparsers.add_parser(
        "list-packages", help="List packages in an environment")
    list_packages_parser.add_argument("-e", "--env", help="Environment name")

    update_parser = subparsers.add_parser(
        "update", help="Update a package in an environment")
    update_parser.add_argument("package", help="Package name")
    update_parser.add_argument("-e", "--env", help="Environment name")

    export_parser = subparsers.add_parser(
        "export", help="Export an environment to a YAML file")
    export_parser.add_argument("name", help="Environment name")
    export_parser.add_argument("output", help="Output file name")

    import_parser = subparsers.add_parser(
        "import", help="Create an environment from a YAML file")
    import_parser.add_argument("input", help="Input YAML file")
    import_parser.add_argument("name", help="New environment name")

    update_all_parser = subparsers.add_parser(
        "update-all", help="Update all packages in an environment")
    update_all_parser.add_argument("-e", "--env", help="Environment name")

    clone_parser = subparsers.add_parser(
        "clone", help="Clone an existing environment")
    clone_parser.add_argument("source", help="Source environment name")
    clone_parser.add_argument(
        "destination", help="Destination environment name")

    args = parser.parse_args()

    # Command execution using parsed arguments
    if args.command == "list":
        handle_list_command()
    elif args.command == "create":
        handle_create_command(args)
    elif args.command == "remove":
        handle_remove_command(args)
    elif args.command == "install":
        handle_install_command(args)
    elif args.command == "list-packages":
        handle_list_packages_command(args)
    elif args.command == "update":
        handle_update_command(args)
    elif args.command == "export":
        handle_export_command(args)
    elif args.command == "import":
        handle_import_command(args)
    elif args.command == "update-all":
        handle_update_all_command(args)
    elif args.command == "clone":
        handle_clone_command(args)
    else:
        parser.print_help()
        sys.exit(1)


if __name__ == "__main__":
    main()
