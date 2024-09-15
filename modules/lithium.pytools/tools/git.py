#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Git Utility Functions

This module provides a set of utility functions to interact with Git repositories.

Features:
- Clone, pull, fetch, and push changes
- Create, switch, merge, and list branches
- Add, commit, and reset changes
- Stash and apply stashed changes
- Create and delete tags
- Add, remove, and set remotes
- View repository status and log

Usage:
    import git_utils

Author:
    Max Qian <lightapt.com>

License:
    GPL-3.0-or-later
"""

import subprocess
import os
import argparse


def run_git_command(command):
    """
    Run a Git command and print its output.

    Args:
        command (List[str]): The Git command and its arguments.

    Returns:
        int: The return code of the command.
    """
    result = subprocess.run(command, capture_output=True, text=True)
    if result.returncode != 0:
        print(f"Error: {result.stderr.strip()}")
    else:
        print(f"Success: {result.stdout.strip()}")
    return result.returncode


def clone_repository(repo_url, clone_dir):
    """
    Clone a Git repository.

    Args:
        repo_url (str): URL of the repository to clone.
        clone_dir (str): Directory to clone the repository into.
    """
    if os.path.exists(clone_dir):
        print(f"Directory {clone_dir} already exists.")
        return
    command = ["git", "clone", repo_url, clone_dir]
    run_git_command(command)


def pull_latest_changes(repo_dir):
    """
    Pull the latest changes from the remote repository.

    Args:
        repo_dir (str): Path to the local repository directory.
    """
    if not os.path.exists(repo_dir):
        print(f"Directory {repo_dir} does not exist.")
        return
    os.chdir(repo_dir)
    command = ["git", "pull"]
    run_git_command(command)


def fetch_changes(repo_dir):
    """
    Fetch the latest changes from the remote repository without merging.

    Args:
        repo_dir (str): Path to the local repository directory.
    """
    if not os.path.exists(repo_dir):
        print(f"Directory {repo_dir} does not exist.")
        return
    os.chdir(repo_dir)
    command = ["git", "fetch"]
    run_git_command(command)


def add_changes(repo_dir):
    """
    Add all changes to the staging area.

    Args:
        repo_dir (str): Path to the local repository directory.
    """
    if not os.path.exists(repo_dir):
        print(f"Directory {repo_dir} does not exist.")
        return
    os.chdir(repo_dir)
    command = ["git", "add", "."]
    run_git_command(command)


def commit_changes(repo_dir, message):
    """
    Commit the staged changes with a message.

    Args:
        repo_dir (str): Path to the local repository directory.
        message (str): Commit message.
    """
    if not os.path.exists(repo_dir):
        print(f"Directory {repo_dir} does not exist.")
        return
    os.chdir(repo_dir)
    command = ["git", "commit", "-m", message]
    run_git_command(command)


def push_changes(repo_dir):
    """
    Push the committed changes to the remote repository.

    Args:
        repo_dir (str): Path to the local repository directory.
    """
    if not os.path.exists(repo_dir):
        print(f"Directory {repo_dir} does not exist。")
        return
    os.chdir(repo_dir)
    command = ["git", "push"]
    run_git_command(command)


def create_branch(repo_dir, branch_name):
    """
    Create a new branch.

    Args:
        repo_dir (str): Path to the local repository directory.
        branch_name (str): Name of the new branch.
    """
    if not os.path.exists(repo_dir):
        print(f"Directory {repo_dir} does not exist.")
        return
    os.chdir(repo_dir)
    command = ["git", "checkout", "-b", branch_name]
    run_git_command(command)


def switch_branch(repo_dir, branch_name):
    """
    Switch to an existing branch.

    Args:
        repo_dir (str): Path to the local repository directory。
        branch_name (str): Name of the branch to switch to.
    """
    if not os.path.exists(repo_dir):
        print(f"Directory {repo_dir} does not exist。")
        return
    os.chdir(repo_dir)
    command = ["git", "checkout", branch_name]
    run_git_command(command)


def merge_branch(repo_dir, branch_name):
    """
    Merge a branch into the current branch.

    Args:
        repo_dir (str): Path to the local repository directory。
        branch_name (str): Name of the branch to merge.
    """
    if not os.path.exists(repo_dir):
        print(f"Directory {repo_dir} does not exist。")
        return
    os.chdir(repo_dir)
    command = ["git", "merge", branch_name]
    run_git_command(command)


def list_branches(repo_dir):
    """
    List all branches in the repository.

    Args:
        repo_dir (str): Path to the local repository directory。
    """
    if not os.path.exists(repo_dir):
        print(f"Directory {repo_dir} does not exist。")
        return
    os.chdir(repo_dir)
    command = ["git", "branch"]
    run_git_command(command)


def reset_changes(repo_dir, commit="HEAD"):
    """
    Reset the repository to a specific commit.

    Args:
        repo_dir (str): Path to the local repository directory。
        commit (str): Commit to reset to (default is HEAD).
    """
    if not os.path.exists(repo_dir):
        print(f"Directory {repo_dir} does not exist。")
        return
    os.chdir(repo_dir)
    command = ["git", "reset", "--hard", commit]
    run_git_command(command)


def stash_changes(repo_dir):
    """
    Stash the current changes.

    Args:
        repo_dir (str): Path to the local repository directory。
    """
    if not os.path.exists(repo_dir):
        print(f"Directory {repo_dir} does not exist。")
        return
    os.chdir(repo_dir)
    command = ["git", "stash"]
    run_git_command(command)


def apply_stash(repo_dir):
    """
    Apply the latest stashed changes.

    Args:
        repo_dir (str): Path to the local repository directory。
    """
    if not os.path.exists(repo_dir):
        print(f"Directory {repo_dir} does not exist。")
        return
    os.chdir(repo_dir)
    command = ["git", "stash", "apply"]
    run_git_command(command)


def view_status(repo_dir):
    """
    View the current status of the repository.

    Args:
        repo_dir (str): Path to the local repository directory。
    """
    if not os.path.exists(repo_dir):
        print(f"Directory {repo_dir} does not exist。")
        return
    os.chdir(repo_dir)
    command = ["git", "status"]
    run_git_command(command)


def view_log(repo_dir):
    """
    View the commit log。

    Args:
        repo_dir (str): Path to the local repository directory。
    """
    if not os.path.exists(repo_dir):
        print(f"Directory {repo_dir} does not exist。")
        return
    os.chdir(repo_dir)
    command = ["git", "log", "--oneline"]
    run_git_command(command)


def add_remote(repo_dir, remote_name, remote_url):
    """
    Add a remote repository.

    Args:
        repo_dir (str): Path to the local repository directory。
        remote_name (str): Name of the remote repository。
        remote_url (str): URL of the remote repository。
    """
    if not os.path.exists(repo_dir):
        print(f"Directory {repo_dir} does not exist。")
        return
    os.chdir(repo_dir)
    command = ["git", "remote", "add", remote_name, remote_url]
    run_git_command(command)


def remove_remote(repo_dir, remote_name):
    """Remove a remote repository."""
    if not os.path.exists(repo_dir):
        print(f"Directory {repo_dir} does not exist。")
        return
    os.chdir(repo_dir)
    command = ["git", "remote", "remove", remote_name]
    run_git_command(command)


def create_tag(repo_dir, tag_name):
    """Create a new tag。"""
    if not os.path.exists(repo_dir):
        print(f"Directory {repo_dir} does not exist。")
        return
    os.chdir(repo_dir)
    command = ["git", "tag", tag_name]
    run_git_command(command)


def delete_tag(repo_dir, tag_name):
    """Delete a tag。"""
    if not os.path.exists(repo_dir):
        print(f"Directory {repo_dir} does not exist。")
        return
    os.chdir(repo_dir)
    command = ["git", "tag", "-d", tag_name]
    run_git_command(command)


def set_user_info(repo_dir, name, email):
    """Set the user name and email for the repository。"""
    if not os.path.exists(repo_dir):
        print(f"Directory {repo_dir} does not exist。")
        return
    os.chdir(repo_dir)
    run_git_command(["git", "config", "user.name", name])
    run_git_command(["git", "config", "user.email", email])


def main():
    parser = argparse.ArgumentParser(
        description="Git Repository Management Tool")
    subparsers = parser.add_subparsers(
        dest="command", help="Git command to run")

    # Clone command
    parser_clone = subparsers.add_parser("clone", help="Clone a repository")
    parser_clone.add_argument(
        "repo_url", help="URL of the repository to clone")
    parser_clone.add_argument(
        "clone_dir", help="Directory to clone the repository into")

    # Pull command
    parser_pull = subparsers.add_parser("pull", help="Pull the latest changes")
    parser_pull.add_argument("repo_dir", help="Directory of the repository")

    # Fetch command
    parser_fetch = subparsers.add_parser(
        "fetch", help="Fetch the latest changes")
    parser_fetch.add_argument("repo_dir", help="Directory of the repository")

    # Add command
    parser_add = subparsers.add_parser(
        "add", help="Add changes to the staging area")
    parser_add.add_argument("repo_dir", help="Directory of the repository")

    # Commit command
    parser_commit = subparsers.add_parser(
        "commit", help="Commit changes with a message")
    parser_commit.add_argument("repo_dir", help="Directory of the repository")
    parser_commit.add_argument("message", help="Commit message")

    # Push command
    parser_push = subparsers.add_parser(
        "push", help="Push changes to the remote repository")
    parser_push.add_argument("repo_dir", help="Directory of the repository")

    # Branch commands
    parser_create_branch = subparsers.add_parser(
        "create-branch", help="Create a new branch")
    parser_create_branch.add_argument(
        "repo_dir", help="Directory of the repository")
    parser_create_branch.add_argument(
        "branch_name", help="Name of the new branch")

    parser_switch_branch = subparsers.add_parser(
        "switch-branch", help="Switch to an existing branch")
    parser_switch_branch.add_argument(
        "repo_dir", help="Directory of the repository")
    parser_switch_branch.add_argument(
        "branch_name", help="Name of the branch to switch to")

    parser_merge_branch = subparsers.add_parser(
        "merge-branch", help="Merge a branch into the current branch")
    parser_merge_branch.add_argument(
        "repo_dir", help="Directory of the repository")
    parser_merge_branch.add_argument(
        "branch_name", help="Name of the branch to merge")

    parser_list_branches = subparsers.add_parser(
        "list-branches", help="List all branches")
    parser_list_branches.add_argument(
        "repo_dir", help="Directory of the repository")

    # Reset command
    parser_reset = subparsers.add_parser(
        "reset", help="Reset the repository to a specific commit")
    parser_reset.add_argument("repo_dir", help="Directory of the repository")
    parser_reset.add_argument(
        "commit", nargs="?", default="HEAD", help="Commit to reset to (default is HEAD)")

    # Stash commands
    parser_stash = subparsers.add_parser(
        "stash", help="Stash the current changes")
    parser_stash.add_argument("repo_dir", help="Directory of the repository")

    parser_apply_stash = subparsers.add_parser(
        "apply-stash", help="Apply the latest stashed changes")
    parser_apply_stash.add_argument(
        "repo_dir", help="Directory of the repository")

    # Status command
    parser_status = subparsers.add_parser(
        "status", help="View the current status of the repository")
    parser_status.add_argument("repo_dir", help="Directory of the repository")

    # Log command
    parser_log = subparsers.add_parser("log", help="View the commit log")
    parser_log.add_argument("repo_dir", help="Directory of the repository")

    # Remote commands
    parser_add_remote = subparsers.add_parser(
        "add-remote", help="Add a new remote repository")
    parser_add_remote.add_argument(
        "repo_dir", help="Directory of the repository")
    parser_add_remote.add_argument(
        "remote_name", help="Name of the remote repository")
    parser_add_remote.add_argument(
        "remote_url", help="URL of the remote repository")

    parser_remove_remote = subparsers.add_parser(
        "remove-remote", help="Remove a remote repository")
    parser_remove_remote.add_argument(
        "repo_dir", help="Directory of the repository")
    parser_remove_remote.add_argument(
        "remote_name", help="Name of the remote repository")

    # Tag commands
    parser_create_tag = subparsers.add_parser(
        "create-tag", help="Create a new tag")
    parser_create_tag.add_argument(
        "repo_dir", help="Directory of the repository")
    parser_create_tag.add_argument("tag_name", help="Name of the new tag")

    parser_delete_tag = subparsers.add_parser(
        "delete-tag", help="Delete a tag")
    parser_delete_tag.add_argument(
        "repo_dir", help="Directory of the repository")
    parser_delete_tag.add_argument(
        "tag_name", help="Name of the tag to delete")

    # Config command
    parser_set_user_info = subparsers.add_parser(
        "set-user-info", help="Set the user name and email for the repository")
    parser_set_user_info.add_argument(
        "repo_dir", help="Directory of the repository")
    parser_set_user_info.add_argument("name", help="User name")
    parser_set_user_info.add_argument("email", help="User email")

    args = parser.parse_args()

    if args.command == "clone":
        clone_repository(args.repo_url, args.clone_dir)
    elif args.command == "pull":
        pull_latest_changes(args.repo_dir)
    elif args.command == "fetch":
        fetch_changes(args.repo_dir)
    elif args.command == "add":
        add_changes(args.repo_dir)
    elif args.command == "commit":
        commit_changes(args.repo_dir, args.message)
    elif args.command == "push":
        push_changes(args.repo_dir)
    elif args.command == "create-branch":
        create_branch(args.repo_dir, args.branch_name)
    elif args.command == "switch-branch":
        switch_branch(args.repo_dir, args.branch_name)
    elif args.command == "merge-branch":
        merge_branch(args.repo_dir, args.branch_name)
    elif args.command == "list-branches":
        list_branches(args.repo_dir)
    elif args.command == "reset":
        reset_changes(args.repo_dir, args.commit)
    elif args.command == "stash":
        stash_changes(args.repo_dir)
    elif args.command == "apply-stash":
        apply_stash(args.repo_dir)
    elif args.command == "status":
        view_status(args.repo_dir)
    elif args.command == "log":
        view_log(args.repo_dir)
    elif args.command == "add-remote":
        add_remote(args.repo_dir, args.remote_name, args.remote_url)
    elif args.command == "remove-remote":
        remove_remote(args.repo_dir, args.remote_name)
    elif args.command == "create-tag":
        create_tag(args.repo_dir, args.tag_name)
    elif args.command == "delete-tag":
        delete_tag(args.repo_dir, args.tag_name)
    elif args.command == "set-user-info":
        set_user_info(args.repo_dir, args.name, args.email)
    else:
        parser.print_help()


if __name__ == "__main__":
    main()
