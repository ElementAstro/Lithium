import pytest
import subprocess
import os
from unittest.mock import patch, call
from ..tools.git import (
    run_git_command, clone_repository, pull_latest_changes, fetch_changes,
    add_changes, commit_changes, push_changes, create_branch, switch_branch,
    merge_branch, list_branches, reset_changes, stash_changes, apply_stash,
    view_status, view_log, add_remote, remove_remote, create_tag, delete_tag,
    set_user_info
)


@pytest.fixture
def repo_dir(tmp_path):
    repo = tmp_path / "repo"
    repo.mkdir()
    return str(repo)


@pytest.fixture
def mock_subprocess_run():
    with patch('subprocess.run') as mock_run:
        yield mock_run


def test_run_git_command_success(mock_subprocess_run):
    mock_subprocess_run.return_value.returncode = 0
    mock_subprocess_run.return_value.stdout = "Success"
    assert run_git_command(["git", "status"]) == 0


def test_run_git_command_failure(mock_subprocess_run):
    mock_subprocess_run.return_value.returncode = 1
    mock_subprocess_run.return_value.stderr = "Error"
    assert run_git_command(["git", "status"]) == 1


def test_clone_repository(repo_dir, mock_subprocess_run):
    clone_repository("https://example.com/repo.git", repo_dir)
    mock_subprocess_run.assert_called_once_with(
        ["git", "clone", "https://example.com/repo.git", repo_dir], capture_output=True, text=True)


def test_pull_latest_changes(repo_dir, mock_subprocess_run):
    pull_latest_changes(repo_dir)
    mock_subprocess_run.assert_called_once_with(
        ["git", "pull"], capture_output=True, text=True)


def test_fetch_changes(repo_dir, mock_subprocess_run):
    fetch_changes(repo_dir)
    mock_subprocess_run.assert_called_once_with(
        ["git", "fetch"], capture_output=True, text=True)


def test_add_changes(repo_dir, mock_subprocess_run):
    add_changes(repo_dir)
    mock_subprocess_run.assert_called_once_with(
        ["git", "add", "."], capture_output=True, text=True)


def test_commit_changes(repo_dir, mock_subprocess_run):
    commit_changes(repo_dir, "Initial commit")
    mock_subprocess_run.assert_called_once_with(
        ["git", "commit", "-m", "Initial commit"], capture_output=True, text=True)


def test_push_changes(repo_dir, mock_subprocess_run):
    push_changes(repo_dir)
    mock_subprocess_run.assert_called_once_with(
        ["git", "push"], capture_output=True, text=True)


def test_create_branch(repo_dir, mock_subprocess_run):
    create_branch(repo_dir, "new-branch")
    mock_subprocess_run.assert_called_once_with(
        ["git", "checkout", "-b", "new-branch"], capture_output=True, text=True)


def test_switch_branch(repo_dir, mock_subprocess_run):
    switch_branch(repo_dir, "main")
    mock_subprocess_run.assert_called_once_with(
        ["git", "checkout", "main"], capture_output=True, text=True)


def test_merge_branch(repo_dir, mock_subprocess_run):
    merge_branch(repo_dir, "feature-branch")
    mock_subprocess_run.assert_called_once_with(
        ["git", "merge", "feature-branch"], capture_output=True, text=True)


def test_list_branches(repo_dir, mock_subprocess_run):
    list_branches(repo_dir)
    mock_subprocess_run.assert_called_once_with(
        ["git", "branch"], capture_output=True, text=True)


def test_reset_changes(repo_dir, mock_subprocess_run):
    reset_changes(repo_dir, "HEAD~1")
    mock_subprocess_run.assert_called_once_with(
        ["git", "reset", "--hard", "HEAD~1"], capture_output=True, text=True)


def test_stash_changes(repo_dir, mock_subprocess_run):
    stash_changes(repo_dir)
    mock_subprocess_run.assert_called_once_with(
        ["git", "stash"], capture_output=True, text=True)


def test_apply_stash(repo_dir, mock_subprocess_run):
    apply_stash(repo_dir)
    mock_subprocess_run.assert_called_once_with(
        ["git", "stash", "apply"], capture_output=True, text=True)


def test_view_status(repo_dir, mock_subprocess_run):
    view_status(repo_dir)
    mock_subprocess_run.assert_called_once_with(
        ["git", "status"], capture_output=True, text=True)


def test_view_log(repo_dir, mock_subprocess_run):
    view_log(repo_dir)
    mock_subprocess_run.assert_called_once_with(
        ["git", "log", "--oneline"], capture_output=True, text=True)


def test_add_remote(repo_dir, mock_subprocess_run):
    add_remote(repo_dir, "origin", "https://example.com/repo.git")
    mock_subprocess_run.assert_called_once_with(
        ["git", "remote", "add", "origin", "https://example.com/repo.git"], capture_output=True, text=True)


def test_remove_remote(repo_dir, mock_subprocess_run):
    remove_remote(repo_dir, "origin")
    mock_subprocess_run.assert_called_once_with(
        ["git", "remote", "remove", "origin"], capture_output=True, text=True)


def test_create_tag(repo_dir, mock_subprocess_run):
    create_tag(repo_dir, "v1.0")
    mock_subprocess_run.assert_called_once_with(
        ["git", "tag", "v1.0"], capture_output=True, text=True)


def test_delete_tag(repo_dir, mock_subprocess_run):
    delete_tag(repo_dir, "v1.0")
    mock_subprocess_run.assert_called_once_with(
        ["git", "tag", "-d", "v1.0"], capture_output=True, text=True)


def test_set_user_info(repo_dir, mock_subprocess_run):
    set_user_info(repo_dir, "John Doe", "john@example.com")
    mock_subprocess_run.assert_has_calls([
        call(["git", "config", "user.name", "John Doe"],
             capture_output=True, text=True),
        call(["git", "config", "user.email", "john@example.com"],
             capture_output=True, text=True)
    ])
