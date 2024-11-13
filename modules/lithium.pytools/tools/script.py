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
import sys
from typing import List, Dict
from loguru import logger

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
        "<green>{time:YYYY-MM-DD HH:mm:ss}</green> | "
        "<level>{level}</level> | {message}"
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
        return

    logger.info("Starting installation of dependencies.")
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
        except subprocess.CalledProcessError as e:
            logger.error(
                f"Failed to install {dependency}: {e.stderr.decode().strip()}")
            raise
        except Exception as e:
            logger.exception(
                f"Unexpected error while installing {dependency}: {e}")
            raise


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
        return

    logger.info(f"Executing script: {script}")
    try:
        subprocess.run(script, shell=True, check=True)
        logger.info(f"Script executed successfully: {script}")
    except subprocess.CalledProcessError as e:
        logger.error(f"Script execution failed: {script}\nError: {e}")
        raise
    except Exception as e:
        logger.exception(f"Unexpected error during script execution: {e}")
        raise


def list_scripts(scripts: Dict[str, str]):
    """
    List all available scripts in the configuration.

    Args:
        scripts (Dict[str, str]): A dictionary of script names and their commands.

    Returns:
        None
    """
    if not scripts:
        logger.info("No scripts available to list.")
        print("No scripts available.")
        return

    logger.info("Listing available scripts.")
    print("Available scripts:")
    for name, command in scripts.items():
        print(f"{name}: {command}")
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
        return

    logger.info("Starting cleanup of temporary directories.")
    for temp_dir in temp_dirs:
        logger.debug(f"Attempting to remove: {temp_dir}")
        try:
            if os.path.exists(temp_dir):
                shutil.rmtree(temp_dir)
                logger.info(f"Removed temporary directory: {temp_dir}")
            else:
                logger.warning(f"Temporary directory not found: {temp_dir}")
        except Exception as e:
            logger.error(f"Failed to remove {temp_dir}: {e}")
            raise
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
        return

    logger.info(f"Running tests with command: {test_command}")
    try:
        subprocess.run(test_command, shell=True, check=True)
        logger.info("Tests executed successfully.")
    except subprocess.CalledProcessError as e:
        logger.error(f"Tests failed with error: {e.stderr.decode().strip()}")
        raise
    except Exception as e:
        logger.exception(f"Unexpected error during test execution: {e}")
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
        return

    logger.info(f"Deploying project with command: {deploy_command}")
    try:
        subprocess.run(deploy_command, shell=True, check=True)
        logger.info("Project deployed successfully.")
    except subprocess.CalledProcessError as e:
        logger.error(
            f"Deployment failed with error: {e.stderr.decode().strip()}")
        raise
    except Exception as e:
        logger.exception(f"Unexpected error during deployment: {e}")
        raise


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
        choices=["build", "install", "list", "clean", "test", "deploy"],
        help="Action to perform",
    )
    parser.add_argument(
        "--config",
        required=True,
        help="Path to the JSON configuration file",
    )

    args = parser.parse_args()

    try:
        config = load_config(args.config)
    except Exception:
        logger.critical("Failed to load configuration. Exiting.")
        print("Error: Failed to load configuration. Check logs for details.")
        sys.exit(1)

    try:
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
        logger.info(f"Action '{args.action}' completed successfully.")
        if args.action not in ["list"]:
            print(f"Action '{args.action}' completed successfully.")
    except Exception:
        logger.critical(f"Action '{args.action}' failed. Exiting.")
        print(f"Error: Action '{args.action}' failed. Check logs for details.")
        sys.exit(1)


if __name__ == "__main__":
    main()
