# python
#!/usr/bin/env python
"""
GitHub Release Script

This script automates the creation or update of a GitHub release
based on the current Travis CI build environment. It can also upload
assets to the release.

Environment Variables:
- MAX_PR_WALKING: Maximum number of PRs to check when looking for a commit (default: 5)
- GITHUB_OAUTH_TOKEN: GitHub OAuth token for authentication
- TRAVIS_REPO_SLUG: Repository slug in the format "owner/repo"
- PROJECT_VERSION: Project version, used for tagging releases
- TRAVIS_COMMIT_MESSAGE: Commit message for the current build
- TRAVIS_COMMIT: Commit SHA for the current build
- GITHUB_DRAFT_RELEASE: Flag indicating if the release is a draft (default: 0)
- GITHUB_PRERELEASE: Flag indicating if the release is a pre-release (default: 1)
- TRAVIS_PULL_REQUEST: Pull request number if the build is for a PR (default: 'false')

Usage:
    python release_script.py [assets...]
"""

import os
import sys
import time
import traceback
import threading
from typing import Optional
from github import Github, Repository, PullRequest
from github.GithubException import UnknownObjectException
from rich.console import Console
from rich.traceback import install

# Initialize rich traceback
install()
console = Console()

# Constants
MAX_PR_WALKING: int = int(os.getenv('MAX_PR_WALKING', '5'))
GITHUB_OAUTH_TOKEN: str = os.getenv('GITHUB_OAUTH_TOKEN', '')
TRAVIS_REPO_SLUG: str = os.getenv('TRAVIS_REPO_SLUG', '')
PROJECT_VERSION: str = os.getenv('PROJECT_VERSION', '')
TRAVIS_COMMIT_MESSAGE: str = os.getenv('TRAVIS_COMMIT_MESSAGE', '')
TRAVIS_COMMIT: str = os.getenv('TRAVIS_COMMIT', '')
GITHUB_DRAFT_RELEASE: str = os.getenv('GITHUB_DRAFT_RELEASE', '0')
GITHUB_PRERELEASE: str = os.getenv('GITHUB_PRERELEASE', '1')
TRAVIS_PULL_REQUEST: str = os.getenv('TRAVIS_PULL_REQUEST', 'false')


def get_github_repo() -> Repository.Repository:
    """
    Retrieve the GitHub repository object using the provided OAuth token and repository slug.

    Raises:
        ValueError: If the GitHub token or repository slug is not provided.

    Returns:
        Repository.Repository: The GitHub repository object.
    """
    if not GITHUB_OAUTH_TOKEN or not TRAVIS_REPO_SLUG:
        raise ValueError("GitHub token or repository slug not provided")
    github = Github(GITHUB_OAUTH_TOKEN)
    return github.get_repo(TRAVIS_REPO_SLUG)


def is_true(env_var: str) -> bool:
    """
    Determine if the environment variable represents a true value.

    Args:
        env_var (str): The environment variable value.

    Returns:
        bool: True if the value is '1' or 'true' (case-insensitive), else False.
    """
    return env_var == '1' or env_var.lower() == 'true'


def find_pr(repo: Repository.Repository, commit_id: str) -> Optional[PullRequest.PullRequest]:
    """
    Find the pull request associated with a given commit ID.

    Args:
        repo (Repository.Repository): The GitHub repository object.
        commit_id (str): The commit SHA to search for.

    Returns:
        Optional[PullRequest.PullRequest]: The pull request object if found, else None.
    """
    all_pulls = list(repo.get_pulls(state='closed', sort='updated'))[::-1]
    for index, merged_pr in enumerate(all_pulls):
        if index >= MAX_PR_WALKING:
            break
        if merged_pr.merge_commit_sha == commit_id:
            return merged_pr
    return None


def get_pr(repo: Repository.Repository, pr_number: str, commit_id: str) -> Optional[PullRequest.PullRequest]:
    """
    Retrieve the pull request object either by PR number or commit ID.

    Args:
        repo (Repository.Repository): The GitHub repository object.
        pr_number (str): The pull request number.
        commit_id (str): The commit SHA to search for if PR number is invalid.

    Returns:
        Optional[PullRequest.PullRequest]: The pull request object if found, else None.
    """
    pr: Optional[PullRequest.PullRequest] = None
    if pr_number and pr_number != 'false':
        try:
            pr = repo.get_pull(int(pr_number))
        except (ValueError, UnknownObjectException):
            console.print(
                "[red]Invalid or non-existent pull request number.[/red]")
    if not pr:
        retry = 0
        while not pr and retry < 5:
            time.sleep(retry * 10)
            try:
                pr = find_pr(repo, commit_id)
                if pr:
                    break
            except Exception:
                traceback.print_exc()
                retry += 1
    return pr


def create_or_update_release(repo: Repository.Repository, tag: str, name: str, body: str,
                             is_draft: bool, is_prerelease: bool, commit: str):
    """
    Create a new release or update an existing one on GitHub.

    Args:
        repo (Repository.Repository): The GitHub repository object.
        tag (str): The tag name for the release.
        name (str): The name of the release.
        body (str): The body content of the release.
        is_draft (bool): Flag indicating if the release is a draft.
        is_prerelease (bool): Flag indicating if the release is a pre-release.
        commit (str): The commit SHA the release is based on.

    Returns:
        GitRelease.GitRelease: The created or updated release object.
    """
    try:
        release = repo.get_release(tag)
        release.update_release(name=name, message=body, draft=is_draft,
                               prerelease=is_prerelease, target_commitish=commit)
        console.print(f"[green]Updated release: {tag}[/green]")
    except UnknownObjectException:
        release = repo.create_git_release(
            tag, name, body, draft=is_draft, prerelease=is_prerelease, target_commitish=commit)
        console.print(f"[green]Created release: {tag}[/green]")
    return release


def upload_asset(release, asset: str):
    """
    Upload an asset to the specified GitHub release.

    Args:
        release (GitRelease.GitRelease): The GitHub release object.
        asset (str): The file path of the asset to upload.
    """
    try:
        release.upload_asset(asset)
        console.print(f"[blue]Successfully uploaded asset: {asset}[/blue]")
    except Exception as e:
        console.print(
            f"[red]Failed to upload asset: {asset}, error: {e}[/red]")


def main():
    """
    Main script execution.
    """
    try:
        print(f'Running {" ".join(sys.argv)}')

        # Get the repository object
        repo = get_github_repo()
        release_tag = f'v{PROJECT_VERSION}'
        commit_id = TRAVIS_COMMIT

        # Convert flags to boolean
        is_draft_release = is_true(GITHUB_DRAFT_RELEASE)
        is_prerelease = is_true(GITHUB_PRERELEASE)

        print(f'release_tag={release_tag}, commit_id={commit_id}')

        # Get the associated pull request
        pr = get_pr(repo, TRAVIS_PULL_REQUEST, commit_id)
        release_body = TRAVIS_COMMIT_MESSAGE

        # If a pull request is found, format the release body
        if pr:
            release_body = f'''## {pr.title}\n\n{
                pr.body}\n\n### Commit message:\n```\n{release_body}\n```\n'''

        # Create or update the release
        release = create_or_update_release(
            repo, release_tag, PROJECT_VERSION, release_body, is_draft_release, is_prerelease, commit_id)

        print(f'Release created/updated: {release_tag}')

        # Upload assets using multithreading for better performance
        threads = []
        for asset in sys.argv[1:]:
            print(f' - deploying asset: {asset}')
            thread = threading.Thread(
                target=upload_asset, args=(release, asset))
            threads.append(thread)
            thread.start()

        for thread in threads:
            thread.join()

    except Exception as e:
        print(f'An error occurred: {e}')
        traceback.print_exc()
        sys.exit(1)


if __name__ == '__main__':
    main()
