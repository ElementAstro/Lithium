#include "git_impl.hpp"

#include <git2/strarray.h>

#include "atom/log/loguru.hpp"

namespace lithium {
GitManager::Impl::Impl(const std::string& repoPath)
    : repoPath(repoPath), repo(nullptr) {
    git_libgit2_init();
}

GitManager::Impl::~Impl() {
    if (repo != nullptr) {
        git_repository_free(repo);
    }
    git_libgit2_shutdown();
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
    int error = git_repository_init(&repo, repoPath.c_str(), 0);
    if (error < 0) {
        printError(error);
        return false;
    }
    return true;
}

auto GitManager::Impl::cloneRepository(const std::string& url) -> bool {
    int error = git_clone(&repo, url.c_str(), repoPath.c_str(), nullptr);
    if (error < 0) {
        printError(error);
        return false;
    }
    return true;
}

auto GitManager::Impl::createBranch(const std::string& branchName) -> bool {
    git_reference* newBranchRef = nullptr;
    git_oid commitOid;

    int error = git_reference_name_to_id(&commitOid, repo, "HEAD");
    if (error < 0) {
        printError(error);
        return false;
    }

    git_commit* targetCommit = nullptr;
    error = git_commit_lookup(&targetCommit, repo, &commitOid);
    if (error < 0) {
        printError(error);
        return false;
    }

    error = git_branch_create(&newBranchRef, repo, branchName.c_str(),
                              targetCommit, 0);
    git_commit_free(targetCommit);
    if (error < 0) {
        printError(error);
        return false;
    }
    return true;
}

auto GitManager::Impl::checkoutBranch(const std::string& branchName) -> bool {
    git_object* treeish = nullptr;
    int error = git_revparse_single(&treeish, repo,
                                    ("refs/heads/" + branchName).c_str());
    if (error < 0) {
        printError(error);
        return false;
    }

    error = git_checkout_tree(repo, treeish, nullptr);
    git_object_free(treeish);
    if (error < 0) {
        printError(error);
        return false;
    }

    error = git_repository_set_head(repo, ("refs/heads/" + branchName).c_str());
    if (error < 0) {
        printError(error);
        return false;
    }
    return true;
}

auto GitManager::Impl::mergeBranch(const std::string& branchName) -> bool {
    git_reference* branchRef;
    int error = git_branch_lookup(&branchRef, repo, branchName.c_str(),
                                  GIT_BRANCH_LOCAL);
    if (error < 0) {
        printError(error);
        return false;
    }

    git_commit* branchCommit;
    error =
        git_commit_lookup(&branchCommit, repo, git_reference_target(branchRef));
    if (error < 0) {
        git_reference_free(branchRef);
        printError(error);
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
        return false;
    }

    if ((analysis & GIT_MERGE_ANALYSIS_UP_TO_DATE) != 0) {
        LOG_F(INFO, "Already up-to-date.");
        git_commit_free(branchCommit);
        git_reference_free(branchRef);
        return true;
    }

    if ((analysis & GIT_MERGE_ANALYSIS_FASTFORWARD) != 0) {
        git_checkout_options opts = GIT_CHECKOUT_OPTIONS_INIT;
        opts.checkout_strategy = GIT_CHECKOUT_SAFE;

        error = git_checkout_tree(repo, (const git_object*)branchCommit, &opts);
        if (error < 0) {
            git_commit_free(branchCommit);
            git_reference_free(branchRef);
            printError(error);
            return false;
        }

        error = git_repository_set_head(repo, git_reference_name(branchRef));
        if (error < 0) {
            git_commit_free(branchCommit);
            git_reference_free(branchRef);
            printError(error);
            return false;
        }
    } else {
        // Perform a non-fast-forward merge
        git_index* index;
        error = git_merge(repo, (const git_annotated_commit**)&branchCommit, 1,
                          nullptr, nullptr);
        if (error < 0) {
            git_commit_free(branchCommit);
            git_reference_free(branchRef);
            printError(error);
            return false;
        }

        error = git_repository_index(&index, repo);
        if (error < 0) {
            git_commit_free(branchCommit);
            git_reference_free(branchRef);
            printError(error);
            return false;
        }

        if (git_index_has_conflicts(index) != 0) {
            LOG_F(ERROR, "Merge conflicts detected. Please resolve them.");
            git_index_free(index);
            git_commit_free(branchCommit);
            git_reference_free(branchRef);
            return false;
        }

        git_oid treeOid;
        error = git_index_write_tree(&treeOid, index);
        git_index_free(index);
        if (error < 0) {
            git_commit_free(branchCommit);
            git_reference_free(branchRef);
            printError(error);
            return false;
        }

        git_tree* tree;
        error = git_tree_lookup(&tree, repo, &treeOid);
        if (error < 0) {
            git_commit_free(branchCommit);
            git_reference_free(branchRef);
            printError(error);
            return false;
        }

        git_commit* headCommit;
        error = git_reference_name_to_id(&treeOid, repo, "HEAD");
        if (error < 0) {
            git_tree_free(tree);
            git_commit_free(branchCommit);
            git_reference_free(branchRef);
            printError(error);
            return false;
        }

        error = git_commit_lookup(&headCommit, repo, &treeOid);
        if (error < 0) {
            git_tree_free(tree);
            git_commit_free(branchCommit);
            git_reference_free(branchRef);
            printError(error);
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
            return false;
        }
    }

    return true;
}

auto GitManager::Impl::addFile(const std::string& filePath) -> bool {
    git_index* index;
    int error = git_repository_index(&index, repo);
    if (error < 0) {
        printError(error);
        return false;
    }

    error = git_index_add_bypath(index, filePath.c_str());
    if (error < 0) {
        printError(error);
        git_index_free(index);
        return false;
    }

    error = git_index_write(index);
    if (error < 0) {
        printError(error);
        git_index_free(index);
        return false;
    }

    git_index_free(index);
    return true;
}

auto GitManager::Impl::commitChanges(const std::string& message) -> bool {
    git_oid treeId;
    git_oid commitId;
    git_index* index;
    git_tree* tree;
    git_signature* sig;

    int error = git_repository_index(&index, repo);
    if (error < 0) {
        printError(error);
        return false;
    }

    error = git_index_write_tree(&treeId, index);
    if (error < 0) {
        printError(error);
        git_index_free(index);
        return false;
    }

    error = git_tree_lookup(&tree, repo, &treeId);
    if (error < 0) {
        printError(error);
        git_index_free(index);
        return false;
    }

    error = git_signature_now(&sig, "Author Name", "email@example.com");
    if (error < 0) {
        printError(error);
        git_tree_free(tree);
        git_index_free(index);
        return false;
    }

    error = git_commit_create_v(&commitId, repo, "HEAD", sig, sig, nullptr,
                                message.c_str(), tree, 0, nullptr);
    if (error < 0) {
        printError(error);
        git_signature_free(sig);
        git_tree_free(tree);
        git_index_free(index);
        return false;
    }

    git_signature_free(sig);
    git_tree_free(tree);
    git_index_free(index);
    return true;
}

auto GitManager::Impl::pull(const std::string& remoteName,
                            const std::string& branchName) -> bool {
    git_remote* remote = nullptr;
    int error = git_remote_lookup(&remote, repo, remoteName.c_str());
    if (error < 0) {
        printError(error);
        return false;
    }

    error = git_remote_fetch(remote, nullptr, nullptr, nullptr);
    if (error < 0) {
        printError(error);
        git_remote_free(remote);
        return false;
    }

    git_reference* ref = nullptr;
    error = git_reference_lookup(
        &ref, repo, ("refs/remotes/" + remoteName + "/" + branchName).c_str());
    if (error < 0) {
        printError(error);
        git_remote_free(remote);
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
        return false;
    }

    if ((analysis & GIT_MERGE_ANALYSIS_UP_TO_DATE) != 0) {
        LOG_F(INFO, "Already up-to-date.");
        git_annotated_commit_free(commit);
        git_remote_free(remote);
        git_reference_free(ref);
        return true;
    }

    if ((analysis & GIT_MERGE_ANALYSIS_FASTFORWARD) != 0) {
        git_checkout_options opts = GIT_CHECKOUT_OPTIONS_INIT;
        opts.checkout_strategy = GIT_CHECKOUT_SAFE;

        error = git_checkout_tree(repo, (const git_object*)commit, &opts);
        if (error < 0) {
            printError(error);
            git_annotated_commit_free(commit);
            git_remote_free(remote);
            git_reference_free(ref);
            return false;
        }

        error = git_repository_set_head(repo, git_reference_name(ref));
        if (error < 0) {
            printError(error);
            git_annotated_commit_free(commit);
            git_remote_free(remote);
            git_reference_free(ref);
            return false;
        }
    } else {
        // Perform a non-fast-forward merge
        git_index* index;
        error = git_merge(repo, (const git_annotated_commit**)&commit, 1,
                          nullptr, nullptr);
        if (error < 0) {
            printError(error);
            git_annotated_commit_free(commit);
            git_remote_free(remote);
            git_reference_free(ref);
            return false;
        }

        error = git_repository_index(&index, repo);
        if (error < 0) {
            printError(error);
            git_annotated_commit_free(commit);
            git_remote_free(remote);
            git_reference_free(ref);
            return false;
        }

        if (git_index_has_conflicts(index) != 0) {
            LOG_F(ERROR, "Merge conflicts detected.");
            git_index_free(index);
            git_annotated_commit_free(commit);
            git_remote_free(remote);
            git_reference_free(ref);
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
            return false;
        }

        git_tree* tree;
        error = git_tree_lookup(&tree, repo, &treeOid);
        if (error < 0) {
            printError(error);
            git_annotated_commit_free(commit);
            git_remote_free(remote);
            git_reference_free(ref);
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
            return false;
        }

        error = git_commit_lookup(&headCommit, repo, &treeOid);
        if (error < 0) {
            git_tree_free(tree);
            git_annotated_commit_free(commit);
            git_remote_free(remote);
            git_reference_free(ref);
            printError(error);
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
            return false;
        }
    }

    return true;
}

auto GitManager::Impl::push(const std::string& remoteName,
                            const std::string& branchName) -> bool {
    git_remote* remote = nullptr;
    int error = git_remote_lookup(&remote, repo, remoteName.c_str());
    if (error < 0) {
        printError(error);
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
        return false;
    }

    git_remote_free(remote);
    return true;
}

}  // namespace lithium
