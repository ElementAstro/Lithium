#ifndef LITHIUM_ADDON_GITMANAGER_HPP
#define LITHIUM_ADDON_GITMANAGER_HPP

#include <memory>
#include <string>

namespace lithium {
class GitManager {
public:
    explicit GitManager(const std::string& repoPath);
    ~GitManager();

    auto initRepository() -> bool;
    auto cloneRepository(const std::string& url) -> bool;
    auto createBranch(const std::string& branchName) -> bool;
    auto checkoutBranch(const std::string& branchName) -> bool;
    auto mergeBranch(const std::string& branchName) -> bool;
    auto addFile(const std::string& filePath) -> bool;
    auto commitChanges(const std::string& message) -> bool;
    auto pull(const std::string& remoteName,
              const std::string& branchName) -> bool;
    auto push(const std::string& remoteName,
              const std::string& branchName) -> bool;

    GitManager(const GitManager&) = delete;
    GitManager& operator=(const GitManager&) = delete;
    GitManager(GitManager&&) = default;
    GitManager& operator=(GitManager&&) = default;

private:
    class Impl;
    std::unique_ptr<Impl> p_impl;  // Renamed to p_impl
};
}  // namespace lithium

#endif  // LITHIUM_ADDON_GITMANAGER_HPP
