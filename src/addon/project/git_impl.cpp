#include "git_impl.hpp"

#include <git2/strarray.h>

#include "atom/log/loguru.hpp"

namespace lithium {
GitManager::Impl::Impl(const std::string& repoPath)
    : repoPath(repoPath), repo(nullptr) {
    LOG_F(INFO, "Initializing GitManager for repository path: {}", repoPath);
    git_libgit2_init();
    LOG_F(INFO, "libgit2 initialized.");
}

GitManager::Impl::~Impl() {
    LOG_F(INFO, "Shutting down GitManager.");
    if (repo != nullptr) {
        git_repository_free(repo);
        LOG_F(INFO, "Repository freed.");
    }
    git_libgit2_shutdown();
    LOG_F(INFO, "libgit2 shutdown completed.");
}

void GitManager::Impl::printError(int error) {
    const git_error* err = git_error_last();
    if (err != nullptr) {
        LOG_F(ERROR, "Error {}: {}", error, err->message);
    } else {
        LOG_F(ERROR, "Unknown error: {}", error);
    }
}

auto GitManager::Impl::initRepository() -> bool {
    LOG_F(INFO, "Entering initRepository()");
    LOG_F(INFO, "Initializing repository at: {}", repoPath);
    if (int error = git_repository_init(&repo, repoPath.c_str(), 0);
        error < 0) {
        printError(error);
        LOG_F(ERROR, "Failed to initialize repository at: {}", repoPath);
        return false;
    }
    LOG_F(INFO, "Repository successfully initialized at: {}", repoPath);
    LOG_F(INFO, "Exiting initRepository()");
    return true;
}

auto GitManager::Impl::cloneRepository(const std::string& url) -> bool {
    LOG_F(INFO, "Entering cloneRepository()");
    LOG_F(INFO, "Cloning repository from URL: {} to path: {}", url, repoPath);
    if (int error = git_clone(&repo, url.c_str(), repoPath.c_str(), nullptr);
        error < 0) {
        printError(error);
        LOG_F(ERROR, "Failed to clone repository from URL: {}", url);
        return false;
    }
    LOG_F(INFO, "Repository successfully cloned from URL: {}", url);
    LOG_F(INFO, "Exiting cloneRepository()");
    return true;
}

auto GitManager::Impl::createBranch(const std::string& branchName) -> bool {
    LOG_F(INFO, "Entering createBranch()");
    LOG_F(INFO, "Creating new branch: {}", branchName);
    git_reference* newBranchRef = nullptr;
    git_oid commitOid;

    int error = git_reference_name_to_id(&commitOid, repo, "HEAD");
    if (error < 0) {
        printError(error);
        LOG_F(ERROR, "Failed to get HEAD reference for branch creation.");
        return false;
    }

    git_commit* targetCommit = nullptr;
    error = git_commit_lookup(&targetCommit, repo, &commitOid);
    if (error < 0) {
        printError(error);
        LOG_F(ERROR, "Failed to lookup commit for branch creation.");
        return false;
    }

    error = git_branch_create(&newBranchRef, repo, branchName.c_str(),
                              targetCommit, 0);
    git_commit_free(targetCommit);
    if (error < 0) {
        printError(error);
        LOG_F(ERROR, "Failed to create branch: {}", branchName);
        return false;
    }
    LOG_F(INFO, "Branch {} successfully created.", branchName);
    LOG_F(INFO, "Exiting createBranch()");
    return true;
}

auto GitManager::Impl::checkoutBranch(const std::string& branchName) -> bool {
    LOG_F(INFO, "Entering checkoutBranch()");
    LOG_F(INFO, "Checking out branch: {}", branchName);
    git_object* treeish = nullptr;
    int error = git_revparse_single(&treeish, repo,
                                    ("refs/heads/" + branchName).c_str());
    if (error < 0) {
        printError(error);
        LOG_F(ERROR, "Failed to parse branch: {}", branchName);
        return false;
    }

    error = git_checkout_tree(repo, treeish, nullptr);
    git_object_free(treeish);
    if (error < 0) {
        printError(error);
        LOG_F(ERROR, "Failed to checkout tree for branch: {}", branchName);
        return false;
    }

    error = git_repository_set_head(repo, ("refs/heads/" + branchName).c_str());
    if (error < 0) {
        printError(error);
        LOG_F(ERROR, "Failed to set HEAD to branch: {}", branchName);
        return false;
    }
    LOG_F(INFO, "Branch {} checked out.", branchName);
    LOG_F(INFO, "Exiting checkoutBranch()");
    return true;
}

auto GitManager::Impl::mergeBranch(const std::string& branchName) -> bool {
    LOG_F(INFO, "Entering mergeBranch()");
    LOG_F(INFO, "Merging branch: {}", branchName);
    git_reference* branchRef;
    int error = git_branch_lookup(&branchRef, repo, branchName.c_str(),
                                  GIT_BRANCH_LOCAL);
    if (error < 0) {
        printError(error);
        LOG_F(ERROR, "Failed to lookup branch: {}", branchName);
        return false;
    }

    git_commit* branchCommit;
    error =
        git_commit_lookup(&branchCommit, repo, git_reference_target(branchRef));
    if (error < 0) {
        git_reference_free(branchRef);
        printError(error);
        LOG_F(ERROR, "Failed to lookup commit for branch: {}", branchName);
        return false;
    }

    git_merge_analysis_t analysis;
    git_merge_preference_t preference;
    error = git_merge_analysis(&analysis, &preference, repo,
                               (const git_annotated_commit**)&branchCommit, 1);
    if (error < 0) {
        git_commit_free(branchCommit);
        git_reference_free(branchRef);
        printError(error);
        LOG_F(ERROR, "Merge analysis failed for branch: {}", branchName);
        return false;
    }

    if ((analysis & GIT_MERGE_ANALYSIS_UP_TO_DATE) != 0) {
        LOG_F(INFO, "Branch {} is already up-to-date.", branchName);
        git_commit_free(branchCommit);
        git_reference_free(branchRef);
        LOG_F(INFO, "Exiting mergeBranch()");
        return true;
    }

    if ((analysis & GIT_MERGE_ANALYSIS_FASTFORWARD) != 0) {
        LOG_F(INFO, "Performing fast-forward merge for branch: {}", branchName);
        git_checkout_options opts = GIT_CHECKOUT_OPTIONS_INIT;
        opts.checkout_strategy = GIT_CHECKOUT_SAFE;

        error = git_checkout_tree(repo, (const git_object*)branchCommit, &opts);
        if (error < 0) {
            git_commit_free(branchCommit);
            git_reference_free(branchRef);
            printError(error);
            LOG_F(ERROR, "Fast-forward checkout failed for branch: {}",
                  branchName);
            return false;
        }

        error = git_repository_set_head(repo, git_reference_name(branchRef));
        if (error < 0) {
            git_commit_free(branchCommit);
            git_reference_free(branchRef);
            printError(error);
            LOG_F(ERROR,
                  "Failed to set HEAD during fast-forward merge for branch: {}",
                  branchName);
            return false;
        }
    } else {
        LOG_F(INFO, "Performing non-fast-forward merge for branch: {}",
              branchName);
        // Perform a non-fast-forward merge
        git_index* index;
        error = git_merge(repo, (const git_annotated_commit**)&branchCommit, 1,
                          nullptr, nullptr);
        if (error < 0) {
            printError(error);
            git_commit_free(branchCommit);
            git_reference_free(branchRef);
            LOG_F(ERROR, "Merge failed for branch: {}", branchName);
            return false;
        }

        error = git_repository_index(&index, repo);
        if (error < 0) {
            printError(error);
            git_commit_free(branchCommit);
            git_reference_free(branchRef);
            LOG_F(ERROR, "Failed to get repository index for merge.");
            return false;
        }

        if (git_index_has_conflicts(index) != 0) {
            LOG_F(ERROR, "Merge conflicts detected when merging branch: {}",
                  branchName);
            git_index_free(index);
            git_commit_free(branchCommit);
            git_reference_free(branchRef);
            LOG_F(ERROR, "Please resolve merge conflicts.");
            return false;
        }

        git_oid treeOid;
        error = git_index_write_tree(&treeOid, index);
        git_index_free(index);
        if (error < 0) {
            printError(error);
            git_commit_free(branchCommit);
            git_reference_free(branchRef);
            LOG_F(ERROR, "Failed to write tree during merge for branch: {}",
                  branchName);
            return false;
        }

        git_tree* tree;
        error = git_tree_lookup(&tree, repo, &treeOid);
        if (error < 0) {
            printError(error);
            git_commit_free(branchCommit);
            git_reference_free(branchRef);
            LOG_F(ERROR, "Failed to lookup tree during merge for branch: {}",
                  branchName);
            return false;
        }

        git_commit* headCommit;
        error = git_reference_name_to_id(&treeOid, repo, "HEAD");
        if (error < 0) {
            git_tree_free(tree);
            git_commit_free(branchCommit);
            git_reference_free(branchRef);
            printError(error);
            LOG_F(ERROR,
                  "Failed to get HEAD commit during merge for branch: {}",
                  branchName);
            return false;
        }

        error = git_commit_lookup(&headCommit, repo, &treeOid);
        if (error < 0) {
            git_tree_free(tree);
            git_commit_free(branchCommit);
            git_reference_free(branchRef);
            printError(error);
            LOG_F(ERROR,
                  "Failed to lookup HEAD commit during merge for branch: {}",
                  branchName);
            return false;
        }

        const git_commit* parents[] = {headCommit, branchCommit};
        git_signature* signature;
        error =
            git_signature_now(&signature, "Author Name", "email@example.com");
        if (error < 0) {
            git_tree_free(tree);
            git_commit_free(branchCommit);
            git_commit_free(headCommit);
            git_reference_free(branchRef);
            printError(error);
            LOG_F(ERROR,
                  "Failed to create signature during merge for branch: {}",
                  branchName);
            return false;
        }

        git_oid commitOid;
        error =
            git_commit_create_v(&commitOid, repo, "HEAD", signature, signature,
                                nullptr, "Merge commit", tree, 2, parents);
        git_signature_free(signature);
        git_tree_free(tree);
        git_commit_free(branchCommit);
        git_commit_free(headCommit);
        git_reference_free(branchRef);
        if (error < 0) {
            printError(error);
            LOG_F(ERROR, "Failed to create merge commit for branch: {}",
                  branchName);
            return false;
        }
    }
    LOG_F(INFO, "Merge of branch {} completed successfully.", branchName);
    LOG_F(INFO, "Exiting mergeBranch()");
    return true;
}

auto GitManager::Impl::addFile(const std::string& filePath) -> bool {
    LOG_F(INFO, "Entering addFile()");
    LOG_F(INFO, "Adding file: {}", filePath);
    git_index* index;
    int error = git_repository_index(&index, repo);
    if (error < 0) {
        printError(error);
        LOG_F(ERROR, "Failed to get repository index for adding file: {}",
              filePath);
        return false;
    }

    error = git_index_add_bypath(index, filePath.c_str());
    if (error < 0) {
        printError(error);
        git_index_free(index);
        LOG_F(ERROR, "Failed to add file to index: {}", filePath);
        return false;
    }

    error = git_index_write(index);
    if (error < 0) {
        printError(error);
        git_index_free(index);
        LOG_F(ERROR, "Failed to write index after adding file: {}", filePath);
        return false;
    }

    git_index_free(index);
    LOG_F(INFO, "File {} added successfully.", filePath);
    LOG_F(INFO, "Exiting addFile()");
    return true;
}

auto GitManager::Impl::commitChanges(const std::string& message) -> bool {
    LOG_F(INFO, "Entering commitChanges()");
    LOG_F(INFO, "Committing changes with message: {}", message);
    git_oid treeId;
    git_oid commitId;
    git_index* index;
    git_tree* tree;
    git_signature* sig;

    int error = git_repository_index(&index, repo);
    if (error < 0) {
        printError(error);
        LOG_F(ERROR, "Failed to get repository index for committing changes.");
        return false;
    }

    error = git_index_write_tree(&treeId, index);
    if (error < 0) {
        printError(error);
        git_index_free(index);
        LOG_F(ERROR, "Failed to write tree for commit.");
        return false;
    }

    error = git_tree_lookup(&tree, repo, &treeId);
    if (error < 0) {
        printError(error);
        git_index_free(index);
        LOG_F(ERROR, "Failed to lookup tree for commit.");
        return false;
    }

    error = git_signature_now(&sig, "Author Name", "email@example.com");
    if (error < 0) {
        printError(error);
        git_tree_free(tree);
        git_index_free(index);
        LOG_F(ERROR, "Failed to create signature for commit.");
        return false;
    }

    error = git_commit_create_v(&commitId, repo, "HEAD", sig, sig, nullptr,
                                message.c_str(), tree, 0, nullptr);
    if (error < 0) {
        printError(error);
        git_signature_free(sig);
        git_tree_free(tree);
        git_index_free(index);
        LOG_F(ERROR, "Failed to create commit.");
        return false;
    }

    git_signature_free(sig);
    git_tree_free(tree);
    git_index_free(index);
    LOG_F(INFO, "Changes committed successfully with ID: {}",
          git_oid_tostr_s(&commitId));
    LOG_F(INFO, "Exiting commitChanges()");
    return true;
}

auto GitManager::Impl::pull(const std::string& remoteName,
                            const std::string& branchName) -> bool {
    LOG_F(INFO, "Entering pull()");
    LOG_F(INFO, "Pulling from remote: {} branch: {}", remoteName, branchName);
    git_remote* remote = nullptr;
    int error = git_remote_lookup(&remote, repo, remoteName.c_str());
    if (error < 0) {
        printError(error);
        LOG_F(ERROR, "Failed to lookup remote: {}", remoteName);
        return false;
    }

    error = git_remote_fetch(remote, nullptr, nullptr, nullptr);
    if (error < 0) {
        printError(error);
        git_remote_free(remote);
        LOG_F(ERROR, "Failed to fetch from remote: {}", remoteName);
        return false;
    }

    git_reference* ref = nullptr;
    std::string refName = "refs/remotes/" + remoteName + "/" + branchName;
    error = git_reference_lookup(&ref, repo, refName.c_str());
    if (error < 0) {
        printError(error);
        git_remote_free(remote);
        LOG_F(ERROR, "Failed to lookup reference: {}", refName);
        return false;
    }

    git_merge_analysis_t analysis;
    git_merge_preference_t preference;
    git_annotated_commit* commit;
    error = git_annotated_commit_from_ref(&commit, repo, ref);
    if (error < 0) {
        printError(error);
        git_remote_free(remote);
        git_reference_free(ref);
        LOG_F(ERROR, "Failed to create annotated commit from ref: {}", refName);
        return false;
    }

    error =
        git_merge_analysis_for_ref(&analysis, &preference, repo, ref,
                                   (const git_annotated_commit**)&commit, 1);
    if (error < 0) {
        printError(error);
        git_annotated_commit_free(commit);
        git_remote_free(remote);
        git_reference_free(ref);
        LOG_F(ERROR, "Failed to perform merge analysis for ref: {}", refName);
        return false;
    }

    if ((analysis & GIT_MERGE_ANALYSIS_UP_TO_DATE) != 0) {
        LOG_F(INFO, "Repository is already up-to-date with remote: {}",
              remoteName);
        git_annotated_commit_free(commit);
        git_remote_free(remote);
        git_reference_free(ref);
        LOG_F(INFO, "Exiting pull()");
        return true;
    }

    if ((analysis & GIT_MERGE_ANALYSIS_FASTFORWARD) != 0) {
        LOG_F(INFO, "Performing fast-forward merge from remote: {}",
              remoteName);
        git_checkout_options opts = GIT_CHECKOUT_OPTIONS_INIT;
        opts.checkout_strategy = GIT_CHECKOUT_SAFE;

        error = git_checkout_tree(repo, (const git_object*)commit, &opts);
        if (error < 0) {
            printError(error);
            git_annotated_commit_free(commit);
            git_remote_free(remote);
            git_reference_free(ref);
            LOG_F(ERROR, "Fast-forward checkout failed for remote: {}",
                  remoteName);
            return false;
        }

        error = git_repository_set_head(repo, git_reference_name(ref));
        if (error < 0) {
            printError(error);
            git_annotated_commit_free(commit);
            git_remote_free(remote);
            git_reference_free(ref);
            LOG_F(ERROR, "Failed to set HEAD after fast-forward merge.");
            return false;
        }
    } else {
        LOG_F(INFO, "Performing non-fast-forward merge from remote: {}",
              remoteName);
        // Perform a non-fast-forward merge
        git_index* index;
        error = git_merge(repo, (const git_annotated_commit**)&commit, 1,
                          nullptr, nullptr);
        if (error < 0) {
            printError(error);
            git_annotated_commit_free(commit);
            git_remote_free(remote);
            git_reference_free(ref);
            LOG_F(ERROR, "Merge failed from remote: {}", remoteName);
            return false;
        }

        error = git_repository_index(&index, repo);
        if (error < 0) {
            printError(error);
            git_annotated_commit_free(commit);
            git_remote_free(remote);
            git_reference_free(ref);
            LOG_F(ERROR, "Failed to get repository index for merge.");
            return false;
        }

        if (git_index_has_conflicts(index) != 0) {
            LOG_F(ERROR,
                  "Merge conflicts detected when pulling from remote: {}",
                  remoteName);
            git_index_free(index);
            git_annotated_commit_free(commit);
            git_remote_free(remote);
            git_reference_free(ref);
            LOG_F(ERROR, "Please resolve merge conflicts.");
            return false;
        }

        git_oid treeOid;
        error = git_index_write_tree(&treeOid, index);
        git_index_free(index);
        if (error < 0) {
            printError(error);
            git_annotated_commit_free(commit);
            git_remote_free(remote);
            git_reference_free(ref);
            LOG_F(ERROR, "Failed to write tree during merge.");
            return false;
        }

        git_tree* tree;
        error = git_tree_lookup(&tree, repo, &treeOid);
        if (error < 0) {
            printError(error);
            git_annotated_commit_free(commit);
            git_remote_free(remote);
            git_reference_free(ref);
            LOG_F(ERROR, "Failed to lookup tree during merge.");
            return false;
        }

        git_commit* headCommit;
        error = git_reference_name_to_id(&treeOid, repo, "HEAD");
        if (error < 0) {
            git_tree_free(tree);
            git_annotated_commit_free(commit);
            git_remote_free(remote);
            git_reference_free(ref);
            printError(error);
            LOG_F(ERROR, "Failed to get HEAD commit during merge.");
            return false;
        }

        error = git_commit_lookup(&headCommit, repo, &treeOid);
        if (error < 0) {
            git_tree_free(tree);
            git_annotated_commit_free(commit);
            git_remote_free(remote);
            git_reference_free(ref);
            printError(error);
            LOG_F(ERROR, "Failed to lookup HEAD commit during merge.");
            return false;
        }

        const git_commit* parents[] = {headCommit, (const git_commit*)commit};
        git_signature* signature;
        error =
            git_signature_now(&signature, "Author Name", "email@example.com");
        if (error < 0) {
            git_tree_free(tree);
            git_commit_free(headCommit);
            git_annotated_commit_free(commit);
            git_remote_free(remote);
            git_reference_free(ref);
            printError(error);
            LOG_F(ERROR, "Failed to create signature during merge.");
            return false;
        }

        git_oid commitOid;
        error =
            git_commit_create_v(&commitOid, repo, "HEAD", signature, signature,
                                nullptr, "Merge commit", tree, 2, parents);
        git_signature_free(signature);
        git_tree_free(tree);
        git_commit_free(headCommit);
        git_annotated_commit_free(commit);
        git_remote_free(remote);
        git_reference_free(ref);
        if (error < 0) {
            printError(error);
            LOG_F(ERROR, "Failed to create merge commit from remote: {}",
                  remoteName);
            return false;
        }
    }

    LOG_F(INFO, "Pull from remote {} completed successfully.", remoteName);
    LOG_F(INFO, "Exiting pull()");
    return true;
}

auto GitManager::Impl::push(const std::string& remoteName,
                            const std::string& branchName) -> bool {
    LOG_F(INFO, "Entering push()");
    LOG_F(INFO, "Pushing to remote: {} branch: {}", remoteName, branchName);
    git_remote* remote = nullptr;
    int error = git_remote_lookup(&remote, repo, remoteName.c_str());
    if (error < 0) {
        printError(error);
        LOG_F(ERROR, "Failed to lookup remote: {}", remoteName);
        return false;
    }

    std::string refspec =
        "refs/heads/" + branchName + ":refs/heads/" + branchName;
    const char* refspecs[] = {refspec.c_str()};
    git_strarray refspecArray = {const_cast<char**>(refspecs), 1};
    error = git_remote_push(remote, &refspecArray, nullptr);
    if (error < 0) {
        printError(error);
        git_remote_free(remote);
        LOG_F(ERROR, "Failed to push to remote: {} branch: {}", remoteName,
              branchName);
        return false;
    }

    git_remote_free(remote);
    LOG_F(INFO, "Successfully pushed to remote: {} branch: {}", remoteName,
          branchName);
    LOG_F(INFO, "Exiting push()");
    return true;
}

}  // namespace lithium
