#include "svn.hpp"
#include "svn_impl.hpp"

namespace lithium {

SvnManager::SvnManager(const std::string& repoPath)
    : impl_(std::make_unique<Impl>(repoPath)) {}

SvnManager::~SvnManager() = default;

bool SvnManager::initRepository() { return impl_->initRepository(); }

bool SvnManager::cloneRepository(const std::string& url) {
    return impl_->cloneRepository(url);
}

bool SvnManager::createBranch(const std::string& branchName) {
    return impl_->createBranch(branchName);
}

bool SvnManager::checkoutBranch(const std::string& branchName) {
    return impl_->checkoutBranch(branchName);
}

bool SvnManager::mergeBranch(const std::string& branchName) {
    return impl_->mergeBranch(branchName);
}

bool SvnManager::addFile(const std::string& filePath) {
    return impl_->addFile(filePath);
}

bool SvnManager::commitChanges(const std::string& message) {
    return impl_->commitChanges(message);
}

bool SvnManager::pull(const std::string& remoteName,
                      const std::string& branchName) {
    return impl_->pull(remoteName, branchName);
}

bool SvnManager::push(const std::string& remoteName,
                      const std::string& branchName) {
    return impl_->push(remoteName, branchName);
}

std::vector<CommitInfo> SvnManager::getLog(int limit) {
    return impl_->getLog(limit);
}

std::optional<std::string> SvnManager::getCurrentBranch() {
    return impl_->getCurrentBranch();
}

std::vector<std::string> SvnManager::getBranches() {
    return impl_->getBranches();
}

std::vector<std::pair<std::string, std::string>> SvnManager::getStatus() {
    return impl_->getStatus();
}

bool SvnManager::revertCommit(const std::string& commitId) {
    return impl_->revertCommit(commitId);
}

bool SvnManager::createTag(const std::string& tagName,
                           const std::string& message) {
    return impl_->createTag(tagName, message);
}

}  // namespace lithium