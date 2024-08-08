#include "git.hpp"
#include "git_impl.hpp"

namespace lithium {
GitManager::GitManager(const std::string& repoPath)
    : impl(new Impl(repoPath)) {}
GitManager::~GitManager() = default;

auto GitManager::initRepository() -> bool { return impl->initRepository(); }

auto GitManager::cloneRepository(const std::string& url) -> bool {
    return impl->cloneRepository(url);
}

auto GitManager::createBranch(const std::string& branchName) -> bool {
    return impl->createBranch(branchName);
}

auto GitManager::checkoutBranch(const std::string& branchName) -> bool {
    return impl->checkoutBranch(branchName);
}

auto GitManager::mergeBranch(const std::string& branchName) -> bool {
    return impl->mergeBranch(branchName);
}

auto GitManager::addFile(const std::string& filePath) -> bool {
    return impl->addFile(filePath);
}

auto GitManager::commitChanges(const std::string& message) -> bool {
    return impl->commitChanges(message);
}

auto GitManager::pull(const std::string& remoteName,
                      const std::string& branchName) -> bool {
    return impl->pull(remoteName, branchName);
}

auto GitManager::push(const std::string& remoteName,
                      const std::string& branchName) -> bool {
    return impl->push(remoteName, branchName);
}

}  // namespace lithium
