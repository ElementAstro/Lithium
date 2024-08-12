#ifndef LITHIUM_ADDON_GITMANAGERIMPL_HPP
#define LITHIUM_ADDON_GITMANAGERIMPL_HPP

#include <git2.h>
#include <string>

#include "git.hpp"

namespace lithium {
class GitManager::Impl {
public:
    explicit Impl(const std::string& repoPath);
    ~Impl();

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

private:
    std::string repoPath;
    git_repository* repo;

    void printError(int error);
};
}  // namespace lithium

#endif  // LITHIUM_ADDON_GITMANAGERIMPL_HPP
