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
from github import Github
from github.GithubException import UnknownObjectException

# Constants
MAX_PR_WALKING = int(os.getenv('MAX_PR_WALKING', '5'))
GITHUB_OAUTH_TOKEN = os.getenv('GITHUB_OAUTH_TOKEN')
TRAVIS_REPO_SLUG = os.getenv('TRAVIS_REPO_SLUG')
PROJECT_VERSION = os.getenv('PROJECT_VERSION')
TRAVIS_COMMIT_MESSAGE = os.getenv('TRAVIS_COMMIT_MESSAGE')
TRAVIS_COMMIT = os.getenv('TRAVIS_COMMIT')
GITHUB_DRAFT_RELEASE = os.getenv('GITHUB_DRAFT_RELEASE', '0')
GITHUB_PRERELEASE = os.getenv('GITHUB_PRERELEASE', '1')
TRAVIS_PULL_REQUEST = os.getenv('TRAVIS_PULL_REQUEST', 'false')


def get_github_repo():
    """
    Get the GitHub repository object.

    Returns:
        github.Repository.Repository: The repository object.

    Raises:
        ValueError: If the GitHub token or repository slug is not provided.
    """
    if not GITHUB_OAUTH_TOKEN or not TRAVIS_REPO_SLUG:
        raise ValueError("GitHub token or repository slug not provided")
    github = Github(GITHUB_OAUTH_TOKEN)
    return github.get_repo(TRAVIS_REPO_SLUG)


def is_true(env_var):
    """
    Convert an environment variable to a boolean value.

    Args:
        env_var (str): The environment variable value.

    Returns:
        bool: True if the environment variable represents a true value, False otherwise.
    """
    return env_var == '1' or env_var.lower() == 'true'


def find_pr(repo, commit_id):
    """
    Find the pull request associated with a given commit ID.

    Args:
        repo (github.Repository.Repository): The repository object.
        commit_id (str): The commit SHA to search for.

    Returns:
        github.PullRequest.PullRequest or None: The pull request object if found, None otherwise.
    """
    all_pulls = list(repo.get_pulls(state='closed', sort='updated'))[::-1]
    for index, merged_pr in enumerate(all_pulls):
        if index >= MAX_PR_WALKING:
            break
        if merged_pr.merge_commit_sha == commit_id:
            return merged_pr
    return None


def get_pr(repo, pr_number, commit_id):
    """
    Get the pull request object.

    Args:
        repo (github.Repository.Repository): The repository object.
        pr_number (str): The pull request number.
        commit_id (str): The commit SHA to search for if the PR number is not valid.

    Returns:
        github.PullRequest.PullRequest or None: The pull request object if found, None otherwise.
    """
    pr = None
    if pr_number and pr_number != 'false':
        try:
            pr = repo.get_pull(int(pr_number))
        except ValueError:
            print("Invalid pull request number")
    if not pr:
        retry = 0
        while not pr and retry < 5:
            time.sleep(retry * 10)
            try:
                pr = find_pr(repo, commit_id)
                break
            except Exception:
                traceback.print_exc()
                retry += 1
    return pr


def create_or_update_release(repo, tag, name, body, is_draft, is_prerelease, commit):
    """
    Create or update a GitHub release.

    Args:
        repo (github.Repository.Repository): The repository object.
        tag (str): The tag for the release.
        name (str): The name of the release.
        body (str): The body description of the release.
        is_draft (bool): Whether the release is a draft.
        is_prerelease (bool): Whether the release is a pre-release.
        commit (str): The commit SHA for the release.

    Returns:
        github.GitRelease.GitRelease: The created or updated release object.
    """
    try:
        release = repo.get_release(tag)
        release.update_release(name=name, message=body, draft=is_draft,
                               prerelease=is_prerelease, target_commitish=commit)
    except UnknownObjectException:
        release = repo.create_git_release(
            tag, name, body, draft=is_draft, prerelease=is_prerelease, target_commitish=commit)
    return release


def upload_asset(release, asset):
    """
    Upload an asset to the GitHub release.

    Args:
        release (github.GitRelease.GitRelease): The release object.
        asset (str): The file path of the asset to upload.
    """
    try:
        release.upload_asset(asset)
        print(f'Successfully uploaded asset: {asset}')
    except Exception as e:
        print(f'Failed to upload asset: {asset}, error: {e}')


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
