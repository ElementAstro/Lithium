#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Project Management Script

This script provides a utility to manage Python projects using a JSON configuration file.
It can install dependencies, execute predefined scripts, and list available scripts.

Features:
- Install dependencies from a JSON configuration file
- Execute predefined scripts
- List available scripts

Usage:
    python project_manager.py build --config project.json
    python project_manager.py --list --config project.json
    python project_manager.py install --config project.json

Author:
    Max Qian <lightapt.com>

License:
    GPL-3.0-or-later
"""

import json
import subprocess
import sys
from pathlib import Path
from typing import Dict

def load_config(file_path: Path) -> Dict:
    """
    Load the JSON configuration file.

    Args:
        file_path (Path): Path to the JSON configuration file.

    Returns:
        Dict: Parsed JSON data as a dictionary.

    Raises:
        SystemExit: If the file is not found, cannot be read, or contains invalid JSON.
    """
    try:
        with file_path.open(encoding='utf-8') as file:
            return json.load(file)
    except json.JSONDecodeError as e:
        print(f"Invalid JSON: {e}")
        sys.exit(1)
    except FileNotFoundError:
        print(f"File not found: {file_path}")
        sys.exit(1)
    except IOError as e:
        print(f"Error reading file: {e}")
        sys.exit(1)

def run_command(command: str):
    """
    Run a shell command.

    Args:
        command (str): The command to run.

    Raises:
        SystemExit: If the command returns a non-zero exit code.
    """
    print(f"Running command: {command}")
    result = subprocess.run(command, shell=True)
    if result.returncode != 0:
        print(f"Command failed with exit code {result.returncode}")
        sys.exit(result.returncode)

def install_dependencies(dependencies: Dict[str, str]):
    """
    Install dependencies specified in the JSON file.

    Args:
        dependencies (Dict[str, str]): A dictionary of package names and versions.
    """
    for package, version in dependencies.items():
        if version.startswith("^"):
            version = version[1:]
        command = f"{sys.executable} -m pip install {package}=={version}"
        run_command(command)

def execute_script(config: Dict, script_name: str):
    """
    Execute a script based on the script name from the JSON configuration.

    Args:
        config (Dict): Parsed JSON data as a dictionary.
        script_name (str): Name of the script to run.

    Raises:
        SystemExit: If the script name is not found in the configuration.
    """
    scripts = config.get("scripts", {})
    command = scripts.get(script_name)

    if command is None:
        print(f"No such script: {script_name}")
        sys.exit(1)

    run_command(command)

def list_scripts(config: Dict):
    """
    List all available scripts in the configuration file.

    Args:
        config (Dict): Parsed JSON data as a dictionary.
    """
    scripts = config.get("scripts", {})
    print("Available scripts:")
    for script in scripts:
        print(f"- {script}")

def main():
    """
    Main function to run the project management script.
    """
    import argparse

    parser = argparse.ArgumentParser(description="Project Management Script")
    parser.add_argument("script", type=str, help="Script to run (e.g., build, lint, test, clean, install)")
    parser.add_argument("--config", type=Path, default=Path("project.json"), help="Path to the JSON configuration file")
    parser.add_argument("--list", action="store_true", help="List all available scripts")

    args = parser.parse_args()
    config = load_config(args.config)

    if args.list:
        list_scripts(config)
    elif args.script == "install":
        dependencies = config.get("dependencies", {})
        install_dependencies(dependencies)
    else:
        execute_script(config, args.script)

if __name__ == "__main__":
    main()
