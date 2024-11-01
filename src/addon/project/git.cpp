#include "git.hpp"
#include "git_impl.hpp"

namespace lithium {
GitManager::GitManager(const std::string& repoPath)
    : impl_(std::make_unique<Impl>(repoPath)) {}
GitManager::~GitManager() = default;

auto GitManager::initRepository() -> bool { return impl_->initRepository(); }

auto GitManager::cloneRepository(const std::string& url) -> bool {
    return impl_->cloneRepository(url);
}

auto GitManager::createBranch(const std::string& branchName) -> bool {
    return impl_->createBranch(branchName);
}

auto GitManager::checkoutBranch(const std::string& branchName) -> bool {
    return impl_->checkoutBranch(branchName);
}

auto GitManager::mergeBranch(const std::string& branchName) -> bool {
    return impl_->mergeBranch(branchName);
}

auto GitManager::addFile(const std::string& filePath) -> bool {
    return impl_->addFile(filePath);
}

auto GitManager::commitChanges(const std::string& message) -> bool {
    return impl_->commitChanges(message);
}

auto GitManager::pull(const std::string& remoteName,
                      const std::string& branchName) -> bool {
    return impl_->pull(remoteName, branchName);
}

auto GitManager::push(const std::string& remoteName,
                      const std::string& branchName) -> bool {
    return impl_->push(remoteName, branchName);
}

}  // namespace lithium
