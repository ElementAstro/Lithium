#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Git Utility Script

This script provides a comprehensive set of utility functions to interact with Git repositories.
It supports various Git operations with enhanced logging and beautiful terminal outputs.

Features:
- Clone, pull, fetch, and push changes
- Create, switch, merge, and list branches
- Add, commit, and reset changes
- Stash and apply stashed changes
- Create and delete tags
- Add, remove, and set remotes
- View repository status and log
- Enhanced logging with loguru
- Beautiful terminal output with rich
- Detailed inline comments and docstrings

Usage:
    python git_utils.py --help

Author:
    Max Qian <lightapt.com>

License:
    GPL-3.0-or-later
"""

import subprocess
import os
import sys
import argparse
from pathlib import Path
from typing import List, Optional

from loguru import logger
from rich.console import Console
from rich.table import Table
from rich.prompt import Confirm
from rich.panel import Panel
from rich.logging import RichHandler

# Configure loguru and rich logging
logger.remove()
console = Console()
logger.add(RichHandler(console=console), level="DEBUG", format="{message}")

# Define Git utilities


class GitRepository:
    """
    A class representing a Git repository and various Git operations.
    """

    def __init__(self, repo_dir: Path):
        """
        Initialize the GitRepository with the given directory.

        Args:
            repo_dir (Path): Path to the local repository directory.
        """
        self.repo_dir = repo_dir.resolve()
        if not self.repo_dir.exists():
            logger.error(f"The directory {self.repo_dir} does not exist.")
            sys.exit(1)
        logger.debug(f"Initialized GitRepository with path: {self.repo_dir}")

    def run_git_command(self, command: List[str]) -> subprocess.CompletedProcess:
        """
        Run a Git command in the repository directory.

        Args:
            command (List[str]): The Git command and its arguments.

        Returns:
            subprocess.CompletedProcess: The result of the subprocess command.
        """
        full_command = ['git'] + command
        logger.debug(f"Running command: {' '.join(full_command)}")
        try:
            result = subprocess.run(
                full_command,
                cwd=self.repo_dir,
                capture_output=True,
                text=True,
                check=True
            )
            logger.debug(result.stdout.strip())
            return result
        except subprocess.CalledProcessError as e:
            logger.error(e.stderr.strip())
            sys.exit(1)

    def clone_repository(self, repo_url: str, clone_dir: Optional[Path] = None):
        """
        Clone a Git repository to the specified directory.

        Args:
            repo_url (str): URL of the repository to clone.
            clone_dir (Optional[Path]): Directory to clone the repository into. Defaults to current directory.
        """
        clone_dir = clone_dir or self.repo_dir
        if clone_dir.exists():
            logger.error(f"Directory {clone_dir} already exists.")
            sys.exit(1)
        command = ['clone', repo_url, str(clone_dir)]
        self.run_git_command(command)
        logger.info(f"Repository cloned to {clone_dir}")

    def pull_latest_changes(self):
        """
        Pull the latest changes from the remote repository.
        """
        command = ['pull']
        self.run_git_command(command)
        logger.info("Pulled latest changes.")

    def fetch_changes(self):
        """
        Fetch the latest changes from the remote repository without merging.
        """
        command = ['fetch']
        self.run_git_command(command)
        logger.info("Fetched latest changes.")

    def add_changes(self, paths: List[str] = ['.']):
        """
        Add changes to the staging area.

        Args:
            paths (List[str]): List of file paths or '.' for all changes.
        """
        command = ['add'] + paths
        self.run_git_command(command)
        logger.info(f"Changes added to staging area: {paths}")

    def commit_changes(self, message: str):
        """
        Commit the staged changes with a message.

        Args:
            message (str): Commit message.
        """
        command = ['commit', '-m', message]
        self.run_git_command(command)
        logger.info(f"Committed changes with message: '{message}'")

    def push_changes(self, remote: str = 'origin', branch: str = 'main'):
        """
        Push the committed changes to the remote repository.

        Args:
            remote (str): Name of the remote repository.
            branch (str): Name of the branch to push.
        """
        command = ['push', remote, branch]
        self.run_git_command(command)
        logger.info(f"Pushed changes to {remote}/{branch}")

    def create_branch(self, branch_name: str):
        """
        Create a new branch and switch to it.

        Args:
            branch_name (str): Name of the new branch.
        """
        command = ['checkout', '-b', branch_name]
        self.run_git_command(command)
        logger.info(f"Created and switched to branch: {branch_name}")

    def switch_branch(self, branch_name: str):
        """
        Switch to an existing branch.

        Args:
            branch_name (str): Name of the branch to switch to.
        """
        command = ['checkout', branch_name]
        self.run_git_command(command)
        logger.info(f"Switched to branch: {branch_name}")

    def merge_branch(self, branch_name: str):
        """
        Merge a branch into the current branch.

        Args:
            branch_name (str): Name of the branch to merge.
        """
        command = ['merge', branch_name]
        self.run_git_command(command)
        logger.info(f"Merged branch '{branch_name}' into current branch.")

    def list_branches(self):
        """
        List all branches in the repository.
        """
        command = ['branch', '--list']
        result = self.run_git_command(command)
        branches = result.stdout.strip().split('\n')
        table = Table(title="Branches", show_header=False)
        for branch in branches:
            table.add_row(branch.strip())
        console.print(table)

    def reset_changes(self, commit: str = 'HEAD'):
        """
        Reset the repository to a specific commit.

        Args:
            commit (str): Commit to reset to.
        """
        if not Confirm.ask("This will reset your repository and discard uncommitted changes. Continue?"):
            logger.info("Operation cancelled.")
            sys.exit(0)
        command = ['reset', '--hard', commit]
        self.run_git_command(command)
        logger.info(f"Repository reset to commit: {commit}")

    def stash_changes(self):
        """
        Stash the current changes.
        """
        command = ['stash']
        self.run_git_command(command)
        logger.info("Changes have been stashed.")

    def apply_stash(self):
        """
        Apply the latest stashed changes.
        """
        command = ['stash', 'apply']
        self.run_git_command(command)
        logger.info("Stashed changes have been applied.")

    def view_status(self):
        """
        View the current status of the repository.
        """
        command = ['status']
        result = self.run_git_command(command)
        console.print(Panel(result.stdout.strip(), title="Git Status"))

    def view_log(self, num_entries: int = 10):
        """
        View the commit log.

        Args:
            num_entries (int): Number of log entries to show.
        """
        command = ['log', f'--oneline', f'-{num_entries}']
        result = self.run_git_command(command)
        logs = result.stdout.strip().split('\n')
        table = Table(title="Commit Log", show_header=False)
        for log_entry in logs:
            table.add_row(log_entry)
        console.print(table)

    def add_remote(self, remote_name: str, remote_url: str):
        """
        Add a remote repository.

        Args:
            remote_name (str): Name of the remote repository.
            remote_url (str): URL of the remote repository.
        """
        command = ['remote', 'add', remote_name, remote_url]
        self.run_git_command(command)
        logger.info(f"Added remote '{remote_name}': {remote_url}")

    def remove_remote(self, remote_name: str):
        """
        Remove a remote repository.

        Args:
            remote_name (str): Name of the remote repository.
        """
        command = ['remote', 'remove', remote_name]
        self.run_git_command(command)
        logger.info(f"Removed remote '{remote_name}'")

    def create_tag(self, tag_name: str):
        """
        Create a new tag.

        Args:
            tag_name (str): Name of the new tag.
        """
        command = ['tag', tag_name]
        self.run_git_command(command)
        logger.info(f"Created tag: {tag_name}")

    def delete_tag(self, tag_name: str):
        """
        Delete a tag.

        Args:
            tag_name (str): Name of the tag to delete.
        """
        command = ['tag', '-d', tag_name]
        self.run_git_command(command)
        logger.info(f"Deleted tag: {tag_name}")

    def set_user_info(self, name: str, email: str):
        """
        Set the user name and email for the repository.

        Args:
            name (str): User name.
            email (str): User email.
        """
        self.run_git_command(['config', 'user.name', name])
        self.run_git_command(['config', 'user.email', email])
        logger.info(f"Set user name to '{name}' and email to '{email}'")

    def blame_file(self, file_path: str):
        """
        Show what revision and author last modified each line of a file.

        Args:
            file_path (str): Path to the file to blame.
        """
        command = ['blame', file_path]
        result = self.run_git_command(command)
        console.print(Panel(result.stdout.strip(),
                      title=f"Blame for {file_path}"))

    def revert_commit(self, commit_hash: str):
        """
        Revert a specific commit.

        Args:
            commit_hash (str): Hash of the commit to revert.
        """
        command = ['revert', commit_hash]
        self.run_git_command(command)
        logger.info(f"Reverted commit: {commit_hash}")

    def cherry_pick(self, commit_hash: str):
        """
        Apply the changes introduced by some existing commits.

        Args:
            commit_hash (str): Hash of the commit to cherry-pick.
        """
        command = ['cherry-pick', commit_hash]
        self.run_git_command(command)
        logger.info(f"Cherry-picked commit: {commit_hash}")

    def rebase_branch(self, branch_name: str):
        """
        Rebase the current branch onto the specified branch.

        Args:
            branch_name (str): Name of the branch to rebase onto.
        """
        command = ['rebase', branch_name]
        self.run_git_command(command)
        logger.info(f"Rebased current branch onto '{branch_name}'")

    def show_remotes(self):
        """
        Show the list of remotes.
        """
        command = ['remote', '-v']
        result = self.run_git_command(command)
        remotes = result.stdout.strip().split('\n')
        table = Table(title="Remotes", show_header=False)
        for remote in remotes:
            table.add_row(remote)
        console.print(table)

    def show_diff(self, commit: Optional[str] = None):
        """
        Show changes between commits, commit and working tree, etc.

        Args:
            commit (Optional[str]): Commit hash to compare. If None, compares working tree with last commit.
        """
        command = ['diff']
        if commit:
            command.append(commit)
        result = self.run_git_command(command)
        console.print(Panel(result.stdout.strip(), title="Git Diff"))

    def clean_untracked_files(self):
        """
        Remove untracked files from the working tree.
        """
        if not Confirm.ask("This will remove all untracked files. Continue?"):
            logger.info("Operation cancelled.")
            sys.exit(0)
        command = ['clean', '-fd']
        self.run_git_command(command)
        logger.info("Untracked files have been removed.")


def create_parser() -> argparse.ArgumentParser:
    """
    Create an argument parser for command-line options.

    Returns:
        argparse.ArgumentParser: Configured argument parser.
    """
    parser = argparse.ArgumentParser(
        description="Git Repository Management Tool with Enhanced Features"
    )
    subparsers = parser.add_subparsers(
        dest="command", help="Git command to run")

    # Common arguments
    parser.add_argument(
        "--repo-dir",
        type=Path,
        default=Path.cwd(),
        help="Path to the local repository (default: current directory)",
    )

    # Clone command
    parser_clone = subparsers.add_parser("clone", help="Clone a repository")
    parser_clone.add_argument(
        "repo_url", help="URL of the repository to clone")
    parser_clone.add_argument(
        "clone_dir", nargs="?", type=Path, help="Directory to clone into (optional)"
    )

    # Pull command
    subparsers.add_parser("pull", help="Pull the latest changes")

    # Fetch command
    subparsers.add_parser("fetch", help="Fetch the latest changes")

    # Add command
    parser_add = subparsers.add_parser(
        "add", help="Add changes to the staging area"
    )
    parser_add.add_argument(
        "paths", nargs="*", default=["."], help="File paths to add (default: all)"
    )

    # Commit command
    parser_commit = subparsers.add_parser(
        "commit", help="Commit changes with a message")
    parser_commit.add_argument("message", help="Commit message")

    # Push command
    parser_push = subparsers.add_parser(
        "push", help="Push changes to the remote repository")
    parser_push.add_argument("--remote", default="origin",
                             help="Remote name (default: origin)")
    parser_push.add_argument("--branch", default="main",
                             help="Branch name (default: main)")

    # Branch commands
    parser_create_branch = subparsers.add_parser(
        "create-branch", help="Create a new branch")
    parser_create_branch.add_argument(
        "branch_name", help="Name of the new branch")

    parser_switch_branch = subparsers.add_parser(
        "switch-branch", help="Switch to an existing branch")
    parser_switch_branch.add_argument(
        "branch_name", help="Name of the branch to switch to")

    parser_merge_branch = subparsers.add_parser(
        "merge-branch", help="Merge a branch into the current branch")
    parser_merge_branch.add_argument(
        "branch_name", help="Name of the branch to merge")

    subparsers.add_parser("list-branches", help="List all branches")

    # Reset command
    parser_reset = subparsers.add_parser(
        "reset", help="Reset the repository to a specific commit")
    parser_reset.add_argument(
        "--commit", default="HEAD", help="Commit to reset to (default: HEAD)"
    )

    # Stash commands
    subparsers.add_parser("stash", help="Stash the current changes")
    subparsers.add_parser(
        "apply-stash", help="Apply the latest stashed changes")

    # Status command
    subparsers.add_parser(
        "status", help="View the current status of the repository")

    # Log command
    parser_log = subparsers.add_parser("log", help="View the commit log")
    parser_log.add_argument(
        "--num", type=int, default=10, help="Number of log entries to show (default: 10)"
    )

    # Remote commands
    parser_add_remote = subparsers.add_parser(
        "add-remote", help="Add a new remote repository")
    parser_add_remote.add_argument(
        "remote_name", help="Name of the remote repository")
    parser_add_remote.add_argument(
        "remote_url", help="URL of the remote repository")

    parser_remove_remote = subparsers.add_parser(
        "remove-remote", help="Remove a remote repository")
    parser_remove_remote.add_argument(
        "remote_name", help="Name of the remote repository")

    subparsers.add_parser("show-remotes", help="Show the list of remotes")

    # Tag commands
    parser_create_tag = subparsers.add_parser(
        "create-tag", help="Create a new tag")
    parser_create_tag.add_argument("tag_name", help="Name of the new tag")

    parser_delete_tag = subparsers.add_parser(
        "delete-tag", help="Delete a tag")
    parser_delete_tag.add_argument(
        "tag_name", help="Name of the tag to delete")

    # Config command
    parser_set_user_info = subparsers.add_parser(
        "set-user-info", help="Set user name and email for the repository")
    parser_set_user_info.add_argument("name", help="User name")
    parser_set_user_info.add_argument("email", help="User email")

    # Blame command
    parser_blame = subparsers.add_parser(
        "blame", help="Show what revision and author last modified each line of a file")
    parser_blame.add_argument("file_path", help="Path to the file to blame")

    # Revert command
    parser_revert = subparsers.add_parser(
        "revert", help="Revert a specific commit")
    parser_revert.add_argument(
        "commit_hash", help="Hash of the commit to revert")

    # Cherry-pick command
    parser_cherry_pick = subparsers.add_parser(
        "cherry-pick", help="Apply changes introduced by existing commits")
    parser_cherry_pick.add_argument(
        "commit_hash", help="Hash of the commit to cherry-pick")

    # Rebase command
    parser_rebase = subparsers.add_parser(
        "rebase", help="Rebase current branch onto another")
    parser_rebase.add_argument(
        "branch_name", help="Name of the branch to rebase onto")

    # Diff command
    parser_diff = subparsers.add_parser(
        "diff", help="Show changes between commits")
    parser_diff.add_argument("--commit", help="Commit hash to compare")

    # Clean command
    subparsers.add_parser(
        "clean", help="Remove untracked files from the working tree")

    return parser


def main():
    """
    Main function to parse arguments and execute Git operations.
    """
    parser = create_parser()
    args = parser.parse_args()

    if not args.command:
        parser.print_help()
        return

    repo_dir = args.repo_dir
    git_repo = GitRepository(repo_dir)

    if args.command == "clone":
        git_repo.clone_repository(args.repo_url, args.clone_dir)
    elif args.command == "pull":
        git_repo.pull_latest_changes()
    elif args.command == "fetch":
        git_repo.fetch_changes()
    elif args.command == "add":
        git_repo.add_changes(args.paths)
    elif args.command == "commit":
        git_repo.commit_changes(args.message)
    elif args.command == "push":
        git_repo.push_changes(args.remote, args.branch)
    elif args.command == "create-branch":
        git_repo.create_branch(args.branch_name)
    elif args.command == "switch-branch":
        git_repo.switch_branch(args.branch_name)
    elif args.command == "merge-branch":
        git_repo.merge_branch(args.branch_name)
    elif args.command == "list-branches":
        git_repo.list_branches()
    elif args.command == "reset":
        git_repo.reset_changes(args.commit)
    elif args.command == "stash":
        git_repo.stash_changes()
    elif args.command == "apply-stash":
        git_repo.apply_stash()
    elif args.command == "status":
        git_repo.view_status()
    elif args.command == "log":
        git_repo.view_log(args.num)
    elif args.command == "add-remote":
        git_repo.add_remote(args.remote_name, args.remote_url)
    elif args.command == "remove-remote":
        git_repo.remove_remote(args.remote_name)
    elif args.command == "show-remotes":
        git_repo.show_remotes()
    elif args.command == "create-tag":
        git_repo.create_tag(args.tag_name)
    elif args.command == "delete-tag":
        git_repo.delete_tag(args.tag_name)
    elif args.command == "set-user-info":
        git_repo.set_user_info(args.name, args.email)
    elif args.command == "blame":
        git_repo.blame_file(args.file_path)
    elif args.command == "revert":
        git_repo.revert_commit(args.commit_hash)
    elif args.command == "cherry-pick":
        git_repo.cherry_pick(args.commit_hash)
    elif args.command == "rebase":
        git_repo.rebase_branch(args.branch_name)
    elif args.command == "diff":
        git_repo.show_diff(args.commit)
    elif args.command == "clean":
        git_repo.clean_untracked_files()
    else:
        parser.print_help()


if __name__ == "__main__":
    main()
