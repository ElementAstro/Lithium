#ifndef LITHIUM_ADDON_VCSMANAGER_HPP
#define LITHIUM_ADDON_VCSMANAGER_HPP

#include <chrono>
#include <optional>
#include <string>
#include <vector>

#include "macro.hpp"

namespace lithium {

/**
 * @brief Represents information about a commit in a version control system.
 */
struct CommitInfo {
    std::string id;       ///< The unique identifier of the commit.
    std::string author;   ///< The author of the commit.
    std::string message;  ///< The commit message.
    std::chrono::system_clock::time_point
        timestamp;  ///< The timestamp of the commit.
} ATOM_ALIGNAS(128);

/**
 * @brief Abstract base class for version control system managers.
 *
 * This class defines the interface for various version control system
 * operations.
 */
class VcsManager {
public:
    /**
     * @brief Virtual destructor for VcsManager.
     */
    virtual ~VcsManager() = default;

    /**
     * @brief Initializes a new repository.
     *
     * @return True if the repository was successfully initialized, false
     * otherwise.
     */
    virtual auto initRepository() -> bool = 0;

    /**
     * @brief Clones a repository from a given URL.
     *
     * @param url The URL of the repository to clone.
     * @return True if the repository was successfully cloned, false otherwise.
     */
    virtual auto cloneRepository(const std::string& url) -> bool = 0;

    /**
     * @brief Creates a new branch.
     *
     * @param branchName The name of the branch to create.
     * @return True if the branch was successfully created, false otherwise.
     */
    virtual auto createBranch(const std::string& branchName) -> bool = 0;

    /**
     * @brief Checks out an existing branch.
     *
     * @param branchName The name of the branch to check out.
     * @return True if the branch was successfully checked out, false otherwise.
     */
    virtual auto checkoutBranch(const std::string& branchName) -> bool = 0;

    /**
     * @brief Merges a branch into the current branch.
     *
     * @param branchName The name of the branch to merge.
     * @return True if the branch was successfully merged, false otherwise.
     */
    virtual auto mergeBranch(const std::string& branchName) -> bool = 0;

    /**
     * @brief Adds a file to the repository.
     *
     * @param filePath The path to the file to add.
     * @return True if the file was successfully added, false otherwise.
     */
    virtual auto addFile(const std::string& filePath) -> bool = 0;

    /**
     * @brief Commits changes to the repository.
     *
     * @param message The commit message.
     * @return True if the changes were successfully committed, false otherwise.
     */
    virtual auto commitChanges(const std::string& message) -> bool = 0;

    /**
     * @brief Pulls changes from a remote repository.
     *
     * @param remoteName The name of the remote repository.
     * @param branchName The name of the branch to pull.
     * @return True if the changes were successfully pulled, false otherwise.
     */
    virtual auto pull(const std::string& remoteName,
                      const std::string& branchName) -> bool = 0;

    /**
     * @brief Pushes changes to a remote repository.
     *
     * @param remoteName The name of the remote repository.
     * @param branchName The name of the branch to push.
     * @return True if the changes were successfully pushed, false otherwise.
     */
    virtual auto push(const std::string& remoteName,
                      const std::string& branchName) -> bool = 0;

    /**
     * @brief Gets the commit log.
     *
     * @param limit The maximum number of commits to return.
     * @return A vector of CommitInfo objects representing the commit log.
     */
    virtual auto getLog(int limit = 10) -> std::vector<CommitInfo> = 0;

    /**
     * @brief Gets the name of the current branch.
     *
     * @return An optional string containing the name of the current branch, or
     * std::nullopt if not on any branch.
     */
    virtual auto getCurrentBranch() -> std::optional<std::string> = 0;

    /**
     * @brief Gets a list of all branches.
     *
     * @return A vector of strings representing the names of all branches.
     */
    virtual auto getBranches() -> std::vector<std::string> = 0;

    /**
     * @brief Gets the status of the repository.
     *
     * @return A vector of pairs, where each pair contains a file path and its
     * status.
     */
    virtual auto getStatus()
        -> std::vector<std::pair<std::string, std::string>> = 0;

    /**
     * @brief Reverts a commit.
     *
     * @param commitId The ID of the commit to revert.
     * @return True if the commit was successfully reverted, false otherwise.
     */
    virtual auto revertCommit(const std::string& commitId) -> bool = 0;

    /**
     * @brief Creates a tag.
     *
     * @param tagName The name of the tag.
     * @param message The tag message.
     * @return True if the tag was successfully created, false otherwise.
     */
    virtual auto createTag(const std::string& tagName,
                           const std::string& message) -> bool = 0;
};

}  // namespace lithium

#endif  // LITHIUM_ADDON_VCSMANAGER_HPP
