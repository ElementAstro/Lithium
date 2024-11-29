# Git Utility Script Documentation

## Overview

The **Git Utility Script** is a Python tool designed to provide a comprehensive set of utility functions for interacting with Git repositories. It supports various Git operations, including cloning, pulling, pushing, branching, committing, stashing, and more. The script features enhanced logging using Loguru and beautiful terminal output with the Rich library, making it easy to manage Git repositories effectively.

---

## Features

- **Clone, Pull, Fetch, and Push Changes**: Simplifies common Git operations.
- **Branch Management**: Create, switch, merge, and list branches.
- **Change Management**: Add, commit, reset, stash, and apply stashed changes.
- **Tag Management**: Create and delete tags.
- **Remote Management**: Add, remove, and set remotes.
- **Repository Status and Log**: View the status of the repository and the commit log.
- **Enhanced Logging**: Utilizes Loguru for structured logging.
- **Beautiful Terminal Output**: Uses Rich for styled console output, including tables and panels.
- **Detailed Inline Comments and Docstrings**: Provides clear documentation within the code for better understanding.

---

## Requirements

- Python 3.x
- Required Python packages:
  - `loguru`: For logging.
  - `rich`: For styled console output.

Install the necessary packages using:

```bash
pip install loguru rich
```

---

## Usage

To run the script, use the following command:

```bash
python git_utils.py --help
```

### Command-Line Arguments

- **`--repo-dir`**: (Optional) Path to the local repository (default: current directory).

### Subcommands

The script supports various subcommands for performing different operations:

- **`clone`**: Clone a Git repository.

  - `repo_url`: URL of the repository to clone.
  - `clone_dir`: (Optional) Directory to clone into.

- **`pull`**: Pull the latest changes from the remote repository.

- **`fetch`**: Fetch the latest changes from the remote repository without merging.

- **`add`**: Add changes to the staging area.

  - `paths`: (Optional) List of file paths to add (default: all changes).

- **`commit`**: Commit changes with a message.

  - `message`: Commit message.

- **`push`**: Push changes to the remote repository.

  - `--remote`: (Optional) Name of the remote repository (default: `origin`).
  - `--branch`: (Optional) Name of the branch to push (default: `main`).

- **`create-branch`**: Create a new branch.

  - `branch_name`: Name of the new branch.

- **`switch-branch`**: Switch to an existing branch.

  - `branch_name`: Name of the branch to switch to.

- **`merge-branch`**: Merge a branch into the current branch.

  - `branch_name`: Name of the branch to merge.

- **`list-branches`**: List all branches in the repository.

- **`reset`**: Reset the repository to a specific commit.

  - `--commit`: (Optional) Commit to reset to (default: `HEAD`).

- **`stash`**: Stash the current changes.

- **`apply-stash`**: Apply the latest stashed changes.

- **`status`**: View the current status of the repository.

- **`log`**: View the commit log.

  - `--num`: (Optional) Number of log entries to show (default: 10).

- **`add-remote`**: Add a new remote repository.

  - `remote_name`: Name of the remote repository.
  - `remote_url`: URL of the remote repository.

- **`remove-remote`**: Remove a remote repository.

  - `remote_name`: Name of the remote repository.

- **`show-remotes`**: Show the list of remotes.

- **`create-tag`**: Create a new tag.

  - `tag_name`: Name of the new tag.

- **`delete-tag`**: Delete a tag.

  - `tag_name`: Name of the tag to delete.

- **`set-user-info`**: Set user name and email for the repository.

  - `name`: User name.
  - `email`: User email.

- **`blame`**: Show what revision and author last modified each line of a file.

  - `file_path`: Path to the file to blame.

- **`revert`**: Revert a specific commit.

  - `commit_hash`: Hash of the commit to revert.

- **`cherry-pick`**: Apply changes introduced by existing commits.

  - `commit_hash`: Hash of the commit to cherry-pick.

- **`rebase`**: Rebase current branch onto another.

  - `branch_name`: Name of the branch to rebase onto.

- **`diff`**: Show changes between commits.

  - `--commit`: (Optional) Commit hash to compare.

- **`clean`**: Remove untracked files from the working tree.

---

## Structure of the Script

### Key Classes and Functions

1. **`GitRepository` Class**  
   The main class responsible for handling Git operations.

   - **`__init__`**: Initializes the Git repository with the specified directory.
   - **`run_git_command`**: Executes a Git command in the repository directory.
   - **Various Methods**: Implements Git operations such as cloning, pulling, pushing, branching, committing, stashing, and more.

2. **`create_parser` Function**  
   Creates and configures the argument parser for command-line options.

3. **`main` Function**  
   The entry point of the script that handles command parsing and execution of the selected operation.

---

## Example Commands

Here are some examples of how to use the script:

### Clone a Repository

```bash
python git_utils.py clone https://github.com/user/repo.git
```

### Pull Latest Changes

```bash
python git_utils.py pull
```

### Add Changes to Staging Area

```bash
python git_utils.py add path/to/file.txt
```

### Commit Changes

```bash
python git_utils.py commit "Initial commit"
```

### Push Changes to Remote

```bash
python git_utils.py push --remote origin --branch main
```

### Create a New Branch

```bash
python git_utils.py create-branch new-feature
```

### List All Branches

```bash
python git_utils.py list-branches
```

### View Repository Status

```bash
python git_utils.py status
```

---

## Error Handling

The script includes robust error handling:

- If the specified repository directory does not exist, an error message is logged, and the script exits.
- Each Git operation is wrapped in try-except blocks to catch and log exceptions.
- User confirmations are prompted for potentially destructive operations (e.g., resetting changes, cleaning untracked files).

---

## Conclusion

The **Git Utility Script** is a powerful and user-friendly tool for managing Git repositories through the command line. With support for a wide range of Git operations, enhanced logging, and beautiful terminal output, it simplifies the process of working with Git, making it an invaluable addition to any developer's toolkit.

This script serves as a versatile solution for anyone needing to interact with Git repositories programmatically, facilitating efficient version control and collaboration.
