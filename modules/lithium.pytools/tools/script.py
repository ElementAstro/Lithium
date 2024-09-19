#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Project Management Script

This script provides a utility to manage Python projects using a JSON configuration file.
It can install dependencies, execute predefined scripts, list available scripts, clean temporary files,
run tests, and deploy the project.

Features:
- Install dependencies from a JSON configuration file
- Execute predefined scripts
- List available scripts
- Clean temporary files and directories
- Run project tests
- Deploy the project

Usage:
    python project_manager.py build --config project.json
    python project_manager.py --list --config project.json
    python project_manager.py install --config project.json
    python project_manager.py clean --config project.json
    python project_manager.py test --config project.json
    python project_manager.py deploy --config project.json

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
from typing import List, Dict


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
    with open(config_file, 'r', encoding='utf-8') as file:
        return json.load(file)


def install_dependencies(dependencies: List[str]):
    """
    Install the project dependencies using pip.

    Args:
        dependencies (List[str]): A list of dependencies to install.

    Returns:
        None
    """
    for dependency in dependencies:
        subprocess.run([sys.executable, "-m", "pip",
                       "install", dependency], check=True)


def execute_script(script: str):
    """
    Execute a predefined script.

    Args:
        script (str): The script to execute.

    Returns:
        None
    """
    subprocess.run(script, shell=True, check=True)


def list_scripts(scripts: Dict[str, str]):
    """
    List all available scripts in the configuration.

    Args:
        scripts (Dict[str, str]): A dictionary of script names and their commands.

    Returns:
        None
    """
    print("Available scripts:")
    for name, command in scripts.items():
        print(f"{name}: {command}")


def clean_project(temp_dirs: List[str]):
    """
    Clean temporary files and directories in the project.

    Args:
        temp_dirs (List[str]): A list of temporary directories to clean.

    Returns:
        None
    """
    for temp_dir in temp_dirs:
        if os.path.exists(temp_dir):
            shutil.rmtree(temp_dir)
            print(f"Removed {temp_dir}")


def run_tests(test_command: str):
    """
    Run the project tests.

    Args:
        test_command (str): The command to run the tests.

    Returns:
        None
    """
    subprocess.run(test_command, shell=True, check=True)


def deploy_project(deploy_command: str):
    """
    Deploy the project.

    Args:
        deploy_command (str): The command to deploy the project.

    Returns:
        None
    """
    subprocess.run(deploy_command, shell=True, check=True)


def main():
    """
    Main function to parse command-line arguments and execute the corresponding actions.

    Returns:
        None
    """
    parser = argparse.ArgumentParser(
        description="Python Project Management Utility")
    parser.add_argument("action", choices=[
                        "build", "install", "list", "clean", "test", "deploy"], help="Action to perform")
    parser.add_argument("--config", required=True,
                        help="Path to the JSON configuration file")

    args = parser.parse_args()

    config = load_config(args.config)

    if args.action == "install":
        install_dependencies(config.get("dependencies", []))
    elif args.action == "build":
        execute_script(config.get("scripts", {}).get("build", ""))
    elif args.action == "list":
        list_scripts(config.get("scripts", {}))
    elif args.action == "clean":
        clean_project(config.get("temp_dirs", []))
    elif args.action == "test":
        run_tests(config.get("scripts", {}).get("test", ""))
    elif args.action == "deploy":
        deploy_project(config.get("scripts", {}).get("deploy", ""))


if __name__ == "__main__":
    main()
