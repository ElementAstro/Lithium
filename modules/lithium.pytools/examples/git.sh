# Example 1: Clone a repository
# This example clones the specified repository URL into the current directory.
$ python git.py clone https://github.com/user/repo.git

# Example 2: Clone a repository into a specific directory
# This example clones the specified repository URL into the specified directory.
$ python git.py clone https://github.com/user/repo.git /path/to/clone/dir

# Example 3: Pull the latest changes
# This example pulls the latest changes from the remote repository.
$ python git.py pull

# Example 4: Fetch the latest changes
# This example fetches the latest changes from the remote repository without merging.
$ python git.py fetch

# Example 5: Add changes to the staging area
# This example adds all changes in the current directory to the staging area.
$ python git.py add

# Example 6: Add specific files to the staging area
# This example adds the specified files to the staging area.
$ python git.py add file1.txt file2.txt

# Example 7: Commit changes with a message
# This example commits the staged changes with the specified commit message.
$ python git.py commit "Initial commit"

# Example 8: Push changes to the remote repository
# This example pushes the committed changes to the specified remote and branch.
$ python git.py push --remote origin --branch main

# Example 9: Create a new branch
# This example creates a new branch with the specified name and switches to it.
$ python git.py create-branch new-feature

# Example 10: Switch to an existing branch
# This example switches to the specified existing branch.
$ python git.py switch-branch new-feature

# Example 11: Merge a branch into the current branch
# This example merges the specified branch into the current branch.
$ python git.py merge-branch new-feature

# Example 12: List all branches
# This example lists all branches in the repository.
$ python git.py list-branches

# Example 13: Reset the repository to a specific commit
# This example resets the repository to the specified commit, discarding uncommitted changes.
$ python git.py reset --commit abc123

# Example 14: Stash the current changes
# This example stashes the current changes.
$ python git.py stash

# Example 15: Apply the latest stashed changes
# This example applies the latest stashed changes.
$ python git.py apply-stash

# Example 16: View the current status of the repository
# This example displays the current status of the repository.
$ python git.py status

# Example 17: View the commit log
# This example displays the commit log with the specified number of entries.
$ python git.py log --num 10

# Example 18: Add a new remote repository
# This example adds a new remote repository with the specified name and URL.
$ python git.py add-remote origin https://github.com/user/repo.git

# Example 19: Remove a remote repository
# This example removes the specified remote repository.
$ python git.py remove-remote origin

# Example 20: Show the list of remotes
# This example displays the list of remotes.
$ python git.py show-remotes

# Example 21: Create a new tag
# This example creates a new tag with the specified name.
$ python git.py create-tag v1.0

# Example 22: Delete a tag
# This example deletes the specified tag.
$ python git.py delete-tag v1.0

# Example 23: Set user name and email for the repository
# This example sets the user name and email for the repository.
$ python git.py set-user-info "Your Name" "your.email@example.com"

# Example 24: Show what revision and author last modified each line of a file
# This example displays the blame information for the specified file.
$ python git.py blame file.txt

# Example 25: Revert a specific commit
# This example reverts the specified commit.
$ python git.py revert abc123

# Example 26: Apply changes introduced by existing commits
# This example cherry-picks the specified commit.
$ python git.py cherry-pick abc123

# Example 27: Rebase the current branch onto another branch
# This example rebases the current branch onto the specified branch.
$ python git.py rebase main

# Example 28: Show changes between commits
# This example displays the changes between the specified commit and the working tree.
$ python git.py diff --commit abc123

# Example 29: Remove untracked files from the working tree
# This example removes all untracked files from the working tree.
$ python git.py clean