#ifndef LITHIUM_ADDON_PROJECT_GITMANAGER_HPP
#define LITHIUM_ADDON_PROJECT_GITMANAGER_HPP

#include <memory>
#include <string>

namespace lithium {
/**
 * @class GitManager
 * @brief A class to manage Git repositories.
 *
 * This class provides basic Git operations such as initializing a repository,
 * cloning a repository, creating branches, checking out branches, merging
 * branches, adding files, committing changes, pulling, and pushing.
 */
class GitManager {
public:
    /**
     * @brief Constructor to initialize the GitManager object.
     * @param repoPath The path to the repository.
     */
    explicit GitManager(const std::string& repoPath);

    /**
     * @brief Destructor to clean up resources.
     */
    ~GitManager();

    /**
     * @brief Initializes a new Git repository.
     * @return Returns true if the repository was successfully initialized,
     * otherwise false.
     */
    auto initRepository() -> bool;

    /**
     * @brief Clones a remote Git repository.
     * @param url The URL of the remote repository.
     * @return Returns true if the repository was successfully cloned, otherwise
     * false.
     */
    auto cloneRepository(const std::string& url) -> bool;

    /**
     * @brief Creates a new branch.
     * @param branchName The name of the branch to create.
     * @return Returns true if the branch was successfully created, otherwise
     * false.
     */
    auto createBranch(const std::string& branchName) -> bool;

    /**
     * @brief Checks out the specified branch.
     * @param branchName The name of the branch to check out.
     * @return Returns true if the branch was successfully checked out,
     * otherwise false.
     */
    auto checkoutBranch(const std::string& branchName) -> bool;

    /**
     * @brief Merges the specified branch into the current branch.
     * @param branchName The name of the branch to merge.
     * @return Returns true if the branch was successfully merged, otherwise
     * false.
     */
    auto mergeBranch(const std::string& branchName) -> bool;

    /**
     * @brief Adds a file to the Git index (staging area).
     * @param filePath The path to the file to add.
     * @return Returns true if the file was successfully added, otherwise false.
     */
    auto addFile(const std::string& filePath) -> bool;

    /**
     * @brief Commits the staged changes.
     * @param message The commit message.
     * @return Returns true if the changes were successfully committed,
     * otherwise false.
     */
    auto commitChanges(const std::string& message) -> bool;

    /**
     * @brief Pulls changes from the specified remote repository and branch.
     * @param remoteName The name of the remote repository.
     * @param branchName The name of the branch to pull from.
     * @return Returns true if the changes were successfully pulled, otherwise
     * false.
     */
    auto pull(const std::string& remoteName,
              const std::string& branchName) -> bool;

    /**
     * @brief Pushes the current branch to the specified remote repository.
     * @param remoteName The name of the remote repository.
     * @param branchName The name of the branch to push.
     * @return Returns true if the changes were successfully pushed, otherwise
     * false.
     */
    auto push(const std::string& remoteName,
              const std::string& branchName) -> bool;

    /**
     * @brief Deleted copy constructor to prevent copying.
     */
    GitManager(const GitManager&) = delete;

    /**
     * @brief Deleted copy assignment operator to prevent copying.
     */
    GitManager& operator=(const GitManager&) = delete;

    /**
     * @brief Defaulted move constructor to allow moving.
     */
    GitManager(GitManager&&) = default;

    /**
     * @brief Defaulted move assignment operator to allow moving.
     */
    GitManager& operator=(GitManager&&) = default;

private:
    /**
     * @class Impl
     * @brief Forward declaration of the implementation class.
     */
    class Impl;

    /**
     * @brief Pointer to the implementation class.
     */
    std::unique_ptr<Impl> impl_;
};

}  // namespace lithium

#endif  // LITHIUM_ADDON_PROJECT_GITMANAGER_HPP