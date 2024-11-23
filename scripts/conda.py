"""
Advanced Conda Environment Management Tool with Rich Enhancements
"""

import sys
import subprocess
import json
import argparse
from typing import Union, List, Optional
from dataclasses import dataclass, asdict
from rich.table import Table
from rich.console import Console
from rich.progress import Progress, SpinnerColumn, TextColumn
from rich import box
from rich.prompt import Prompt
from rich.text import Text

console = Console()


@dataclass
class ContainerInfo:
    id: str
    name: str
    status: str
    image: str
    ports: dict
    cpu_usage: float
    memory_usage: float


def run_command(command: str) -> Optional[str]:
    """
    Execute a shell command and return its output.

    Args:
        command (str): The command to execute.

    Returns:
        Optional[str]: The standard output of the command if successful, None otherwise.
    """
    try:
        result = subprocess.run(
            command,
            shell=True,
            capture_output=True,
            text=True,
            check=True
        )
        console.log(f"[green]Command succeeded:[/green] {command}")
        return result.stdout.strip()
    except subprocess.CalledProcessError as e:
        console.log(f"[red]Command failed:[/red] {command}\nError: {e.stderr}")
        return None


def list_environments() -> List[str]:
    """
    Retrieve a list of all available Conda environments.

    Returns:
        List[str]: A list of environment paths.
    """
    envs = run_command("conda env list --json")
    if envs:
        try:
            envs_dict = json.loads(envs)
            environments = envs_dict.get("envs", [])
            return environments
        except json.JSONDecodeError as e:
            console.log(f"[red]Failed to parse environment list:[/red] {e}")
            return []
    return []


def create_environment(env_name: str, packages: Optional[List[str]] = None) -> Optional[str]:
    """
    Create a new Conda environment with optional packages.

    Args:
        env_name (str): The name of the environment to create.
        packages (Optional[List[str]]): A list of packages to install in the environment.

    Returns:
        Optional[str]: The command output if successful, None otherwise.
    """
    cmd = f"conda create -n {env_name} -y"
    if packages:
        cmd += f" {' '.join(packages)}"
    return run_command(cmd)


def remove_environment(env_name: str) -> Optional[str]:
    """
    Remove an existing Conda environment.

    Args:
        env_name (str): The name of the environment to remove.

    Returns:
        Optional[str]: The command output if successful, None otherwise.
    """
    return run_command(f"conda env remove -n {env_name} -y")


def install_package(package_name: str, env_name: Optional[str] = None) -> Optional[str]:
    """
    Install a package into a specified Conda environment.

    Args:
        package_name (str): The name of the package to install.
        env_name (Optional[str]): The target environment name. If None, installs in the current environment.

    Returns:
        Optional[str]: The command output if successful, None otherwise.
    """
    cmd = f"conda install {package_name} -y"
    if env_name:
        cmd = f"conda run -n {env_name} {cmd}"
    return run_command(cmd)


def list_packages(env_name: Optional[str] = None) -> Optional[str]:
    """
    List all packages in a specified Conda environment.

    Args:
        env_name (Optional[str]): The name of the environment. If None, lists packages in the current environment.

    Returns:
        Optional[str]: The command output if successful, None otherwise.
    """
    cmd = "conda list"
    if env_name:
        cmd = f"conda run -n {env_name} {cmd}"
    return run_command(cmd)


def update_package(package_name: str, env_name: Optional[str] = None) -> Optional[str]:
    """
    Update a specific package in a Conda environment.

    Args:
        package_name (str): The name of the package to update.
        env_name (Optional[str]): The target environment name. If None, updates in the current environment.

    Returns:
        Optional[str]: The command output if successful, None otherwise.
    """
    cmd = f"conda update {package_name} -y"
    if env_name:
        cmd = f"conda run -n {env_name} {cmd}"
    return run_command(cmd)


def export_environment(env_name: str, output_file: str) -> Optional[str]:
    """
    Export a Conda environment to a YAML file.

    Args:
        env_name (str): The name of the environment to export.
        output_file (str): The file path to save the exported environment.

    Returns:
        Optional[str]: The command output if successful, None otherwise.
    """
    cmd = f"conda env export -n {env_name} > {output_file}"
    return run_command(cmd)


def import_environment(input_file: str, env_name: str) -> Optional[str]:
    """
    Import a Conda environment from a YAML file.

    Args:
        input_file (str): The path to the YAML file.
        env_name (str): The name of the new environment to create.

    Returns:
        Optional[str]: The command output if successful, None otherwise.
    """
    cmd = f"conda env create -f {input_file} -n {env_name}"
    return run_command(cmd)


def update_all_packages(env_name: Optional[str] = None) -> Optional[str]:
    """
    Update all packages in a specified Conda environment.

    Args:
        env_name (Optional[str]): The target environment name. If None, updates in the current environment.

    Returns:
        Optional[str]: The command output if successful, None otherwise.
    """
    cmd = "conda update --all -y"
    if env_name:
        cmd = f"conda run -n {env_name} {cmd}"
    return run_command(cmd)


def clone_environment(source_env: str, dest_env: str) -> Optional[str]:
    """
    Clone an existing Conda environment to a new environment.

    Args:
        source_env (str): The name of the source environment.
        dest_env (str): The name of the destination environment.

    Returns:
        Optional[str]: The command output if successful, None otherwise.
    """
    cmd = f"conda create --name {dest_env} --clone {source_env} -y"
    return run_command(cmd)


def handle_list_command() -> None:
    """
    Handle the 'list' command to display all Conda environments.
    """
    console.print("[bold cyan]Listing Conda environments...[/bold cyan]")
    environments = list_environments()
    if environments:
        table = Table(title="Conda Environments", box=box.MINIMAL_DOUBLE_EDGE)
        table.add_column("Environment Path", style="cyan")
        for env in environments:
            table.add_row(env)
        console.print(table)
    else:
        console.print("[red]No environments found.[/red]")


def handle_create_command(args: argparse.Namespace) -> None:
    """
    Handle the 'create' command to create a new Conda environment.

    Args:
        args (argparse.Namespace): The parsed command-line arguments.
    """
    console.print(
        f"[bold green]Creating environment '{args.name}'...[/bold green]")
    with Progress(SpinnerColumn(), TextColumn("[progress.description]{task.description}")) as progress:
        task = progress.add_task("Creating environment...", start=False)
        progress.start_task(task.id)
        result = create_environment(args.name, args.packages)
    if result:
        console.print(
            f"[green]Environment '{args.name}' created successfully.[/green]")
    else:
        console.print(
            f"[red]Failed to create environment '{args.name}'.[/red]")


def handle_remove_command(args: argparse.Namespace) -> None:
    """
    Handle the 'remove' command to delete an existing Conda environment.

    Args:
        args (argparse.Namespace): The parsed command-line arguments.
    """
    console.print(
        f"[bold red]Removing environment '{args.name}'...[/bold red]")
    with Progress(SpinnerColumn(), TextColumn("[progress.description]{task.description}")) as progress:
        task = progress.add_task("Removing environment...", start=False)
        progress.start_task(task.id)
        result = remove_environment(args.name)
    if result:
        console.print(
            f"[green]Environment '{args.name}' removed successfully.[/green]")
    else:
        console.print(
            f"[red]Failed to remove environment '{args.name}'.[/red]")


def handle_install_command(args: argparse.Namespace) -> None:
    """
    Handle the 'install' command to add a package to a Conda environment.

    Args:
        args (argparse.Namespace): The parsed command-line arguments.
    """
    console.print(
        f"[bold green]Installing package '{args.package}'...[/bold green]")
    with Progress(SpinnerColumn(), TextColumn("[progress.description]{task.description}")) as progress:
        task = progress.add_task("Installing package...", start=False)
        progress.start_task(task.id)
        result = install_package(args.package, args.env)
    if result:
        console.print(
            f"[green]Package '{args.package}' installed successfully.[/green]")
    else:
        console.print(
            f"[red]Failed to install package '{args.package}'.[/red]")


def handle_list_packages_command(args: argparse.Namespace) -> None:
    """
    Handle the 'list-packages' command to display all packages in a Conda environment.

    Args:
        args (argparse.Namespace): The parsed command-line arguments.
    """
    env = args.env if args.env else "current"
    console.print(
        f"[bold cyan]Listing packages in environment '{env}'...[/bold cyan]")
    result = list_packages(args.env)
    if result:
        table = Table(
            title=f"Packages in '{env}'", box=box.MINIMAL_DOUBLE_EDGE)
        for line in result.splitlines():
            parts = line.split()
            if len(parts) >= 3:
                table.add_row(parts[0], parts[1], parts[2])
        console.print(table)
    else:
        console.print(f"[red]No packages found in environment '{env}'.[/red]")


def handle_update_command(args: argparse.Namespace) -> None:
    """
    Handle the 'update' command to update a specific package in a Conda environment.

    Args:
        args (argparse.Namespace): The parsed command-line arguments.
    """
    console.print(
        f"[bold green]Updating package '{args.package}'...[/bold green]")
    with Progress(SpinnerColumn(), TextColumn("[progress.description]{task.description}")) as progress:
        task = progress.add_task("Updating package...", start=False)
        progress.start_task(task.id)
        result = update_package(args.package, args.env)
    if result:
        console.print(
            f"[green]Package '{args.package}' updated successfully.[/green]")
    else:
        console.print(f"[red]Failed to update package '{args.package}'.[/red]")


def handle_export_command(args: argparse.Namespace) -> None:
    """
    Handle the 'export' command to export a Conda environment to a YAML file.

    Args:
        args (argparse.Namespace): The parsed command-line arguments.
    """
    console.print(
        f"[bold cyan]Exporting environment '{args.name}' to '{args.output}'...[/bold cyan]")
    with Progress(SpinnerColumn(), TextColumn("[progress.description]{task.description}")) as progress:
        task = progress.add_task("Exporting environment...", start=False)
        progress.start_task(task.id)
        result = export_environment(args.name, args.output)
    if result:
        console.print(
            f"[green]Environment '{args.name}' exported to '{args.output}' successfully.[/green]")
    else:
        console.print(
            f"[red]Failed to export environment '{args.name}'.[/red]")


def handle_import_command(args: argparse.Namespace) -> None:
    """
    Handle the 'import' command to create a Conda environment from a YAML file.

    Args:
        args (argparse.Namespace): The parsed command-line arguments.
    """
    console.print(
        f"[bold cyan]Importing environment from '{args.input}' to '{args.name}'...[/bold cyan]")
    with Progress(SpinnerColumn(), TextColumn("[progress.description]{task.description}")) as progress:
        task = progress.add_task("Importing environment...", start=False)
        progress.start_task(task.id)
        result = import_environment(args.input, args.name)
    if result:
        console.print(
            f"[green]Environment '{args.name}' imported successfully.[/green]")
    else:
        console.print(
            f"[red]Failed to import environment '{args.name}'.[/red]")


def handle_update_all_command(args: argparse.Namespace) -> None:
    """
    Handle the 'update-all' command to update all packages in a Conda environment.

    Args:
        args (argparse.Namespace): The parsed command-line arguments.
    """
    env = args.env if args.env else "current"
    console.print(
        f"[bold green]Updating all packages in environment '{env}'...[/bold green]")
    with Progress(SpinnerColumn(), TextColumn("[progress.description]{task.description}")) as progress:
        task = progress.add_task("Updating all packages...", start=False)
        progress.start_task(task.id)
        result = update_all_packages(args.env)
    if result:
        console.print(
            f"[green]All packages in environment '{env}' updated successfully.[/green]")
    else:
        console.print(
            f"[red]Failed to update packages in environment '{env}'.[/red]")


def handle_clone_command(args: argparse.Namespace) -> None:
    """
    Handle the 'clone' command to duplicate a Conda environment.

    Args:
        args (argparse.Namespace): The parsed command-line arguments.
    """
    console.print(
        f"[bold cyan]Cloning environment '{args.source}' to '{args.destination}'...[/bold cyan]")
    with Progress(SpinnerColumn(), TextColumn("[progress.description]{task.description}")) as progress:
        task = progress.add_task("Cloning environment...", start=False)
        progress.start_task(task.id)
        result = clone_environment(args.source, args.destination)
    if result:
        console.print(
            f"[green]Environment '{args.source}' cloned to '{args.destination}' successfully.[/green]")
    else:
        console.print(
            f"[red]Failed to clone environment '{args.source}'.[/red]")


def main() -> None:
    """
    Main function to parse command-line arguments and execute corresponding commands.
    """
    parser = argparse.ArgumentParser(
        description="Advanced Conda Environment Management Tool with Rich Enhancements"
    )
    subparsers = parser.add_subparsers(
        dest="command", help="Available commands"
    )

    # Define subparsers for various commands
    subparsers.add_parser("list", help="List all Conda environments")

    create_parser = subparsers.add_parser(
        "create", help="Create a new Conda environment"
    )
    create_parser.add_argument("name", help="Environment name")
    create_parser.add_argument(
        "-p", "--packages", nargs="+", help="Packages to install"
    )

    remove_parser = subparsers.add_parser(
        "remove", help="Remove a Conda environment"
    )
    remove_parser.add_argument("name", help="Environment name")

    install_parser = subparsers.add_parser(
        "install", help="Install a package in an environment"
    )
    install_parser.add_argument("package", help="Package name")
    install_parser.add_argument("-e", "--env", help="Environment name")

    list_packages_parser = subparsers.add_parser(
        "list-packages", help="List packages in an environment"
    )
    list_packages_parser.add_argument("-e", "--env", help="Environment name")

    update_parser = subparsers.add_parser(
        "update", help="Update a package in an environment"
    )
    update_parser.add_argument("package", help="Package name")
    update_parser.add_argument("-e", "--env", help="Environment name")

    export_parser = subparsers.add_parser(
        "export", help="Export an environment to a YAML file"
    )
    export_parser.add_argument("name", help="Environment name")
    export_parser.add_argument("output", help="Output file name")

    import_parser = subparsers.add_parser(
        "import", help="Create an environment from a YAML file"
    )
    import_parser.add_argument("input", help="Input YAML file")
    import_parser.add_argument("name", help="New environment name")

    update_all_parser = subparsers.add_parser(
        "update-all", help="Update all packages in an environment"
    )
    update_all_parser.add_argument("-e", "--env", help="Environment name")

    clone_parser = subparsers.add_parser(
        "clone", help="Clone an existing environment"
    )
    clone_parser.add_argument("source", help="Source environment name")
    clone_parser.add_argument(
        "destination", help="Destination environment name"
    )

    args = parser.parse_args()

    # Execute the appropriate handler based on the command
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
