# rename.py

#!/usr/bin/env python3
"""
rename.py

A comprehensive Python-based tool for managing files and directories on Linux.
Supports renaming, listing, copying, deleting, moving, and additional advanced features.

Usage:
    python rename.py {rename|list|copy|delete|move} <RootDirectory> <OldElement> <NewElement> [--destination <DestinationDirectory>] [--dry-run] [--verbose]

Commands:
    rename  <RootDirectory> <OldElement> <NewElement>       : Rename files and directories by replacing OldElement with NewElement.
    list    <RootDirectory>                                 : List all files and directories under RootDirectory.
    copy    <RootDirectory> <DestinationDirectory>          : Copy RootDirectory to DestinationDirectory.
    delete  <RootDirectory>                                 : Delete RootDirectory and its contents.
    move    <RootDirectory> <DestinationDirectory>          : Move RootDirectory to DestinationDirectory.

Options:
    --destination <DestinationDirectory>  : Specify destination directory for copy and move operations.
    --dry-run                             : Show actions without making any changes.
    --verbose                             : Enable verbose output for detailed operation logs.

Examples:
    Rename files:
        python rename.py rename /path/to/directory old new

    List files and directories:
        python rename.py list /path/to/directory

    Copy files and directories with dry run:
        python rename.py copy /path/to/source /path/to/destination --dry-run

    Delete files and directories with confirmation:
        python rename.py delete /path/to/directory

    Move files and directories with verbose output:
        python rename.py move /path/to/source /path/to/destination --verbose
"""

import os
import sys
import argparse
import shutil
from pathlib import Path
from rich.console import Console
from rich.logging import RichHandler
import logging

# Configure Rich logging
logging.basicConfig(
    level=logging.INFO,
    format="%(message)s",
    datefmt="[%X]",
    handlers=[RichHandler()]
)
logger = logging.getLogger("rename_tool")

console = Console()


def rename_files(root_directory: str, old_element: str, new_element: str, dry_run: bool):
    """
    Recursively rename files and directories by replacing old_element with new_element.

    Args:
        root_directory (str): The root directory to start renaming.
        old_element (str): The string to be replaced in filenames.
        new_element (str): The string to replace with in filenames.
        dry_run (bool): If True, only show actions without renaming.
    """
    logger.info(
        f"[bold cyan]Starting rename operation in '{root_directory}'[/bold cyan] replacing '{old_element}' with '{new_element}'")
    for dirpath, dirnames, filenames in os.walk(root_directory, topdown=False):
        # Rename files
        for filename in filenames:
            if old_element in filename:
                old_path = Path(dirpath) / filename
                new_filename = filename.replace(old_element, new_element)
                new_path = Path(dirpath) / new_filename
                logger.debug(f"Renaming file: {old_path} -> {new_path}")
                if dry_run:
                    logger.info(
                        f"[yellow][Dry Run][/yellow] Rename file: {old_path} -> {new_path}")
                else:
                    try:
                        old_path.rename(new_path)
                        logger.info(
                            f"[green]Renamed file:[/green] {old_path} -> {new_path}")
                    except Exception as e:
                        logger.error(
                            f"[red]Error renaming file {old_path}: {e}[/red]")

        # Rename directories
        for dirname in dirnames:
            if old_element in dirname:
                old_dir = Path(dirpath) / dirname
                new_dirname = dirname.replace(old_element, new_element)
                new_dir = Path(dirpath) / new_dirname
                logger.debug(f"Renaming directory: {old_dir} -> {new_dir}")
                if dry_run:
                    logger.info(
                        f"[yellow][Dry Run][/yellow] Rename directory: {old_dir} -> {new_dir}")
                else:
                    try:
                        old_dir.rename(new_dir)
                        logger.info(
                            f"[green]Renamed directory:[/green] {old_dir} -> {new_dir}")
                    except Exception as e:
                        logger.error(
                            f"[red]Error renaming directory {old_dir}: {e}[/red]")


def list_files(root_directory: str):
    """
    List all files and directories under the root_directory.

    Args:
        root_directory (str): The directory to list contents of.
    """
    logger.info(
        f"[bold cyan]Listing contents of '{root_directory}'[/bold cyan]")
    for path in Path(root_directory).rglob('*'):
        if path.is_dir():
            console.print(f"[blue]{path}[/blue]")
        else:
            console.print(f"[green]{path}[/green]")


def copy_files(root_directory: str, destination_directory: str, dry_run: bool):
    """
    Copy the root_directory to the destination_directory.

    Args:
        root_directory (str): The directory to copy.
        destination_directory (str): The directory to copy to.
        dry_run (bool): If True, only show actions without copying.
    """
    if dry_run:
        logger.info(
            f"[yellow][Dry Run][/yellow] Would copy '{root_directory}' to '{destination_directory}'")
        return

    try:
        shutil.copytree(root_directory, destination_directory)
        logger.info(
            f"[green]Copied '{root_directory}' to '{destination_directory}'[/green]")
    except FileExistsError:
        logger.error(
            f"[red]Destination directory '{destination_directory}' already exists.[/red]")
    except Exception as e:
        logger.error(f"[red]Error copying directory: {e}[/red]")


def delete_files(root_directory: str, dry_run: bool):
    """
    Delete the root_directory and all its contents.

    Args:
        root_directory (str): The directory to delete.
        dry_run (bool): If True, only show actions without deleting.
    """
    if dry_run:
        logger.info(
            f"[yellow][Dry Run][/yellow] Would delete '{root_directory}'")
        return

    confirm = console.input(
        f"Are you sure you want to delete '{root_directory}'? [y/N]: ")
    if confirm.lower() == 'y':
        try:
            shutil.rmtree(root_directory)
            logger.info(f"[green]Deleted '{root_directory}'[/green]")
        except FileNotFoundError:
            logger.error(
                f"[red]Directory '{root_directory}' does not exist.[/red]")
        except Exception as e:
            logger.error(f"[red]Error deleting directory: {e}[/red]")
    else:
        logger.info("Deletion cancelled.")


def move_files(root_directory: str, destination_directory: str, dry_run: bool):
    """
    Move the root_directory to the destination_directory.

    Args:
        root_directory (str): The directory to move.
        destination_directory (str): The directory to move to.
        dry_run (bool): If True, only show actions without moving.
    """
    if dry_run:
        logger.info(
            f"[yellow][Dry Run][/yellow] Would move '{root_directory}' to '{destination_directory}'")
        return

    try:
        shutil.move(root_directory, destination_directory)
        logger.info(
            f"[green]Moved '{root_directory}' to '{destination_directory}'[/green]")
    except FileNotFoundError:
        logger.error(
            f"[red]Source directory '{root_directory}' does not exist.[/red]")
    except Exception as e:
        logger.error(f"[red]Error moving directory: {e}[/red]")


def parse_arguments():
    """
    Parse command-line arguments.

    Returns:
        argparse.Namespace: Parsed arguments.
    """
    parser = argparse.ArgumentParser(
        description="Manage files and directories: rename, list, copy, delete, move.",
        formatter_class=argparse.RawTextHelpFormatter,
        epilog="""
Examples:
    Rename files:
        python rename.py rename /path/to/directory old new

    List files and directories:
        python rename.py list /path/to/directory

    Copy files and directories with dry run:
        python rename.py copy /path/to/source /path/to/destination --dry-run

    Delete files and directories with confirmation:
        python rename.py delete /path/to/directory

    Move files and directories with verbose output:
        python rename.py move /path/to/source /path/to/destination --verbose
        """
    )

    subparsers = parser.add_subparsers(
        title="Commands", dest="command", required=True)

    # Rename command
    rename_parser = subparsers.add_parser(
        "rename", help="Rename files and directories by replacing a string.")
    rename_parser.add_argument(
        "RootDirectory", type=str, help="The root directory to perform rename operations.")
    rename_parser.add_argument(
        "OldElement", type=str, help="The string to be replaced in filenames.")
    rename_parser.add_argument(
        "NewElement", type=str, help="The string to replace with in filenames.")
    rename_parser.add_argument(
        "--dry-run", action="store_true", help="Show actions without making any changes.")
    rename_parser.add_argument(
        "--verbose", action="store_true", help="Enable verbose output.")

    # List command
    list_parser = subparsers.add_parser(
        "list", help="List all files and directories under the root directory.")
    list_parser.add_argument(
        "RootDirectory", type=str, help="The directory to list contents of.")

    # Copy command
    copy_parser = subparsers.add_parser(
        "copy", help="Copy files and directories to a destination.")
    copy_parser.add_argument(
        "RootDirectory", type=str, help="The directory to copy.")
    copy_parser.add_argument(
        "DestinationDirectory", type=str, help="The directory to copy to.")
    copy_parser.add_argument(
        "--dry-run", action="store_true", help="Show actions without copying.")
    copy_parser.add_argument(
        "--verbose", action="store_true", help="Enable verbose output.")

    # Delete command
    delete_parser = subparsers.add_parser(
        "delete", help="Delete files and directories.")
    delete_parser.add_argument(
        "RootDirectory", type=str, help="The directory to delete.")
    delete_parser.add_argument(
        "--dry-run", action="store_true", help="Show actions without deleting.")
    delete_parser.add_argument(
        "--verbose", action="store_true", help="Enable verbose output.")

    # Move command
    move_parser = subparsers.add_parser(
        "move", help="Move files and directories to a destination.")
    move_parser.add_argument(
        "RootDirectory", type=str, help="The directory to move.")
    move_parser.add_argument(
        "DestinationDirectory", type=str, help="The directory to move to.")
    move_parser.add_argument(
        "--dry-run", action="store_true", help="Show actions without moving.")
    move_parser.add_argument(
        "--verbose", action="store_true", help="Enable verbose output.")

    return parser.parse_args()


def main():
    args = parse_arguments()

    # Set logging level
    if hasattr(args, 'verbose') and args.verbose:
        logger.setLevel(logging.DEBUG)

    if args.command == "rename":
        if not Path(args.RootDirectory).is_dir():
            logger.error(
                f"[red]'{args.RootDirectory}' is not a valid directory.[/red]")
            sys.exit(1)
        rename_files(args.RootDirectory, args.OldElement,
                     args.NewElement, args.dry_run)

    elif args.command == "list":
        if not Path(args.RootDirectory).is_dir():
            logger.error(
                f"[red]'{args.RootDirectory}' is not a valid directory.[/red]")
            sys.exit(1)
        list_files(args.RootDirectory)

    elif args.command == "copy":
        src = Path(args.RootDirectory)
        dest = Path(args.DestinationDirectory)
        if not src.is_dir():
            logger.error(
                f"[red]Source directory '{args.RootDirectory}' does not exist.[/red]")
            sys.exit(1)
        if dest.exists():
            logger.error(
                f"[red]Destination directory '{args.DestinationDirectory}' already exists.[/red]")
            sys.exit(1)
        copy_files(args.RootDirectory, args.DestinationDirectory, args.dry_run)

    elif args.command == "delete":
        target = Path(args.RootDirectory)
        if not target.is_dir():
            logger.error(
                f"[red]Directory '{args.RootDirectory}' does not exist.[/red]")
            sys.exit(1)
        delete_files(args.RootDirectory, args.dry_run)

    elif args.command == "move":
        src = Path(args.RootDirectory)
        dest = Path(args.DestinationDirectory)
        if not src.is_dir():
            logger.error(
                f"[red]Source directory '{args.RootDirectory}' does not exist.[/red]")
            sys.exit(1)
        if dest.exists():
            logger.error(
                f"[red]Destination directory '{args.DestinationDirectory}' already exists.[/red]")
            sys.exit(1)
        move_files(args.RootDirectory, args.DestinationDirectory, args.dry_run)

    else:
        logger.error("[red]Invalid command.[/red]")
        sys.exit(1)


if __name__ == "__main__":
    main()
