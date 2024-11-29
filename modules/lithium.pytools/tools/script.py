#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Project Management Script

This script provides a utility to manage Python projects using a JSON configuration file.
It can install dependencies, execute predefined scripts, list available scripts, clean temporary files,
run tests, deploy the project, and generate project documentation.

Features:
- Install dependencies from a JSON configuration file
- Execute predefined scripts
- List available scripts with descriptions
- Clean temporary files and directories
- Run project tests
- Deploy the project
- Generate project documentation
- Enhanced logging with Loguru
- Beautified terminal outputs using Rich
- Supports configuration via environment variables

Usage Examples:
    python project_manager.py build --config project.json
    python project_manager.py --list --config project.json
    python project_manager.py install --config project.json
    python project_manager.py clean --config project.json
    python project_manager.py test --config project.json
    python project_manager.py deploy --config project.json
    python project_manager.py docs --config project.json

Author:
    Max Qian <lightapt.com>

License:
    GPL-3.0-or-later
"""

import argparse
import json
import subprocess
import os
import shutil
import sys
from typing import List, Dict, Optional
from loguru import logger
from rich.console import Console
from rich.table import Table
from rich.prompt import Prompt
from rich.progress import Progress, SpinnerColumn, TextColumn
from dotenv import load_dotenv

# Load environment variables from .env file if exists
load_dotenv()

# Initialize Rich console
console = Console()

# Configure Loguru for logging
logger.remove()  # Remove the default logger
logger.add(
    "project_manager.log",
    rotation="10 MB",
    retention="30 days",
    compression="zip",
    enqueue=True,
    encoding="utf-8",
    level="DEBUG",
    format=(
        "<green>{time:YYYY-MM-DD HH:mm:ss.SSS}</green> | "
        "<level>{level: <8}</level> | "
        "<cyan>{module}</cyan>:<cyan>{line}</cyan> - <level>{message}</level>"
    ),
)
logger.add(
    sys.stderr,
    level="INFO",
    format="<level>{message}</level>",
)


def load_config(config_file: str) -> Dict:
    """
    Load the JSON configuration file.

    Args:
        config_file (str): Path to the JSON configuration file.

    Returns:
        dict: The loaded configuration as a dictionary.

    Raises:
        FileNotFoundError: If the configuration file is not found.
        json.JSONDecodeError: If the configuration file contains invalid JSON.
    """
    logger.debug(f"Loading configuration file: {config_file}")
    try:
        with open(config_file, 'r', encoding='utf-8') as file:
            config = json.load(file)
        logger.info(f"Configuration loaded successfully from {config_file}")
        return config
    except FileNotFoundError:
        logger.error(f"Configuration file not found: {config_file}")
        raise
    except json.JSONDecodeError as e:
        logger.error(f"Invalid JSON in configuration file: {e}")
        raise
    except Exception as e:
        logger.exception(f"Unexpected error while loading configuration: {e}")
        raise


def install_dependencies(dependencies: List[str]):
    """
    Install the project dependencies using pip.

    Args:
        dependencies (List[str]): A list of dependencies to install.

    Returns:
        None
    """
    if not dependencies:
        logger.info("No dependencies to install.")
        console.print("[bold yellow]No dependencies to install.[/bold yellow]")
        return

    logger.info("Starting installation of dependencies.")
    with Progress(SpinnerColumn(), TextColumn("[progress.description]{task.description}")) as progress:
        task = progress.add_task(
            "[green]Installing dependencies...", total=None)
        for dependency in dependencies:
            logger.debug(f"Installing dependency: {dependency}")
            try:
                subprocess.run(
                    [sys.executable, "-m", "pip", "install", dependency],
                    check=True,
                    stdout=subprocess.PIPE,
                    stderr=subprocess.PIPE,
                )
                logger.info(f"Successfully installed: {dependency}")
                console.print(
                    f"[bold green]Successfully installed:[/bold green] {dependency}")
            except subprocess.CalledProcessError as e:
                error_msg = e.stderr.decode().strip()
                logger.error(f"Failed to install {dependency}: {error_msg}")
                console.print(
                    f"[bold red]Failed to install {dependency}:[/bold red] {error_msg}")
                raise
            except Exception as e:
                logger.exception(
                    f"Unexpected error while installing {dependency}: {e}")
                console.print(
                    f"[bold red]Unexpected error while installing {dependency}:[/bold red] {e}")
                raise
        progress.update(
            task, description="[bold green]Dependencies installed successfully[/bold green]")
    logger.debug("Installation of dependencies completed.")


def execute_script(script: str):
    """
    Execute a predefined script.

    Args:
        script (str): The script to execute.

    Returns:
        None
    """
    if not script:
        logger.warning("No script command provided to execute.")
        console.print(
            "[bold yellow]No script command provided to execute.[/bold yellow]")
        return

    logger.info(f"Executing script: {script}")
    try:
        result = subprocess.run(
            script, shell=True, check=True, capture_output=True, text=True)
        logger.info(f"Script executed successfully: {script}")
        console.print(
            f"[bold green]Script executed successfully:[/bold green] {script}")
        if result.stdout:
            console.print(f"[cyan]{result.stdout}[/cyan]")
    except subprocess.CalledProcessError as e:
        logger.error(
            f"Script execution failed: {script}\nError: {e.stderr.strip()}")
        console.print(
            f"[bold red]Script execution failed:[/bold red] {script}\nError: {e.stderr.strip()}")
        raise
    except Exception as e:
        logger.exception(f"Unexpected error during script execution: {e}")
        console.print(
            f"[bold red]Unexpected error during script execution:[/bold red] {e}")
        raise


def list_scripts(scripts: Dict[str, Dict[str, str]]):
    """
    List all available scripts in the configuration.

    Args:
        scripts (Dict[str, Dict[str, str]]): A dictionary of script names and their details.

    Returns:
        None
    """
    if not scripts:
        logger.info("No scripts available to list.")
        console.print("[bold yellow]No scripts available.[/bold yellow]")
        return

    logger.info("Listing available scripts.")
    table = Table(title="Available Scripts")
    table.add_column("Name", style="cyan", no_wrap=True)
    table.add_column("Description", style="magenta")
    table.add_column("Command", style="green")

    for name, details in scripts.items():
        description = details.get("description", "No description")
        command = details.get("command", "")
        table.add_row(name, description, command)
    console.print(table)
    logger.debug("Scripts listed successfully.")


def clean_project(temp_dirs: List[str]):
    """
    Clean temporary files and directories in the project.

    Args:
        temp_dirs (List[str]): A list of temporary directories to clean.

    Returns:
        None
    """
    if not temp_dirs:
        logger.info("No temporary directories specified for cleaning.")
        console.print(
            "[bold yellow]No temporary directories specified for cleaning.[/bold yellow]")
        return

    logger.info("Starting cleanup of temporary directories.")
    with Progress(SpinnerColumn(), TextColumn("[progress.description]{task.description}")) as progress:
        task = progress.add_task(
            "[green]Cleaning temporary directories...", total=None)
        for temp_dir in temp_dirs:
            logger.debug(f"Attempting to remove: {temp_dir}")
            try:
                if os.path.exists(temp_dir):
                    shutil.rmtree(temp_dir)
                    logger.info(f"Removed temporary directory: {temp_dir}")
                    console.print(
                        f"[bold green]Removed temporary directory:[/bold green] {temp_dir}")
                else:
                    logger.warning(
                        f"Temporary directory not found: {temp_dir}")
                    console.print(
                        f"[bold yellow]Temporary directory not found:[/bold yellow] {temp_dir}")
            except Exception as e:
                logger.error(f"Failed to remove {temp_dir}: {e}")
                console.print(
                    f"[bold red]Failed to remove {temp_dir}:[/bold red] {e}")
                raise
        progress.update(
            task, description="[bold green]Cleanup completed successfully[/bold green]")
    logger.debug("Cleanup of temporary directories completed.")


def run_tests(test_command: str):
    """
    Run the project tests.

    Args:
        test_command (str): The command to run the tests.

    Returns:
        None
    """
    if not test_command:
        logger.warning("No test command provided to execute.")
        console.print(
            "[bold yellow]No test command provided to execute.[/bold yellow]")
        return

    logger.info(f"Running tests with command: {test_command}")
    try:
        result = subprocess.run(test_command, shell=True,
                                check=True, capture_output=True, text=True)
        logger.info("Tests executed successfully.")
        console.print("[bold green]Tests executed successfully.[/bold green]")
        if result.stdout:
            console.print(f"[cyan]{result.stdout}[/cyan]")
    except subprocess.CalledProcessError as e:
        logger.error(f"Tests failed with error: {e.stderr.strip()}")
        console.print(
            f"[bold red]Tests failed with error:[/bold red] {e.stderr.strip()}")
        raise
    except Exception as e:
        logger.exception(f"Unexpected error during test execution: {e}")
        console.print(
            f"[bold red]Unexpected error during test execution:[/bold red] {e}")
        raise


def deploy_project(deploy_command: str):
    """
    Deploy the project.

    Args:
        deploy_command (str): The command to deploy the project.

    Returns:
        None
    """
    if not deploy_command:
        logger.warning("No deploy command provided to execute.")
        console.print(
            "[bold yellow]No deploy command provided to execute.[/bold yellow]")
        return

    logger.info(f"Deploying project with command: {deploy_command}")
    try:
        result = subprocess.run(
            deploy_command, shell=True, check=True, capture_output=True, text=True)
        logger.info("Project deployed successfully.")
        console.print(
            "[bold green]Project deployed successfully.[/bold green]")
        if result.stdout:
            console.print(f"[cyan]{result.stdout}[/cyan]")
    except subprocess.CalledProcessError as e:
        logger.error(f"Deployment failed with error: {e.stderr.strip()}")
        console.print(
            f"[bold red]Deployment failed with error:[/bold red] {e.stderr.strip()}")
        raise
    except Exception as e:
        logger.exception(f"Unexpected error during deployment: {e}")
        console.print(
            f"[bold red]Unexpected error during deployment:[/bold red] {e}")
        raise


def generate_docs(doc_command: str):
    """
    Generate project documentation.

    Args:
        doc_command (str): The command to generate documentation.

    Returns:
        None
    """
    if not doc_command:
        logger.warning("No documentation command provided to execute.")
        console.print(
            "[bold yellow]No documentation command provided to execute.[/bold yellow]")
        return

    logger.info(f"Generating documentation with command: {doc_command}")
    try:
        result = subprocess.run(doc_command, shell=True,
                                check=True, capture_output=True, text=True)
        logger.info("Documentation generated successfully.")
        console.print(
            "[bold green]Documentation generated successfully.[/bold green]")
        if result.stdout:
            console.print(f"[cyan]{result.stdout}[/cyan]")
    except subprocess.CalledProcessError as e:
        logger.error(
            f"Documentation generation failed with error: {e.stderr.strip()}")
        console.print(
            f"[bold red]Documentation generation failed with error:[/bold red] {e.stderr.strip()}")
        raise
    except Exception as e:
        logger.exception(
            f"Unexpected error during documentation generation: {e}")
        console.print(
            f"[bold red]Unexpected error during documentation generation:[/bold red] {e}")
        raise


def prompt_for_missing_config(config: Dict):
    """
    Prompt the user for missing configuration items.

    Args:
        config (Dict): The current configuration dictionary.

    Returns:
        Dict: The updated configuration dictionary.

    """
    logger.info("Prompting for missing configuration items.")
    if 'dependencies' not in config:
        deps = Prompt.ask("Enter dependencies (comma-separated)", default="")
        config['dependencies'] = [dep.strip()
                                  for dep in deps.split(",") if dep.strip()]
    if 'scripts' not in config:
        config['scripts'] = {}
    logger.debug("Configuration updated with user input.")
    return config


def main():
    """
    Main function to parse command-line arguments and execute the corresponding actions.

    Returns:
        None
    """
    parser = argparse.ArgumentParser(
        description="Python Project Management Utility"
    )
    parser.add_argument(
        "action",
        choices=["build", "install", "list",
                 "clean", "test", "deploy", "docs"],
        help="Action to perform",
    )
    parser.add_argument(
        "--config",
        required=False,
        help="Path to the JSON configuration file",
    )
    parser.add_argument(
        "--env",
        default=".env",
        help="Path to the environment variables file",
    )

    args = parser.parse_args()

    # Load configuration from file or environment variable
    config_file = args.config or os.getenv(
        'PROJECT_CONFIG_FILE', 'project.json')

    try:
        config = load_config(config_file)
    except Exception:
        logger.warning(
            "Failed to load configuration. Attempting to prompt for missing items.")
        config = prompt_for_missing_config({})
        if not config:
            logger.critical("No configuration available. Exiting.")
            console.print(
                "[bold red]Error: No configuration available. Exiting.[/bold red]")
            sys.exit(1)

    try:
        if args.action == "install":
            install_dependencies(config.get("dependencies", []))
        elif args.action == "build":
            build_script = config.get("scripts", {}).get(
                "build", {}).get("command", "")
            execute_script(build_script)
        elif args.action == "list":
            list_scripts(config.get("scripts", {}))
        elif args.action == "clean":
            clean_project(config.get("temp_dirs", []))
        elif args.action == "test":
            test_script = config.get("scripts", {}).get(
                "test", {}).get("command", "")
            run_tests(test_script)
        elif args.action == "deploy":
            deploy_script = config.get("scripts", {}).get(
                "deploy", {}).get("command", "")
            deploy_project(deploy_script)
        elif args.action == "docs":
            doc_script = config.get("scripts", {}).get(
                "docs", {}).get("command", "")
            generate_docs(doc_script)
        logger.info(f"Action '{args.action}' completed successfully.")
        if args.action not in ["list"]:
            console.print(
                f"[bold green]Action '{args.action}' completed successfully.[/bold green]")
    except Exception as e:
        logger.critical(f"Action '{args.action}' failed. Exiting.")
        console.print(
            f"[bold red]Error: Action '{args.action}' failed. Check logs for details.[/bold red]")
        sys.exit(1)


if __name__ == "__main__":
    main()
