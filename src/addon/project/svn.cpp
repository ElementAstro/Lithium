#include "svn.hpp"
#include <memory>
#include "svn_impl.hpp"

namespace lithium {
SvnManager::SvnManager(const std::string& repoPath)
    : impl_(std::make_unique<Impl>(repoPath)) {}
SvnManager::~SvnManager() = default;

bool SvnManager::checkout(const std::string& url, const std::string& revision) {
    return impl_->checkout(url, revision);
}

bool SvnManager::addFile(const std::string& filePath) {
    return impl_->addFile(filePath);
}

bool SvnManager::commitChanges(const std::string& message) {
    return impl_->commitChanges(message);
}

bool SvnManager::update() { return impl_->update(); }

bool SvnManager::createBranch(const std::string& branchName) {
    return impl_->createBranch(branchName);
}

bool SvnManager::mergeBranch(const std::string& branchName) {
    return impl_->mergeBranch(branchName);
}

std::vector<std::string> SvnManager::getLog(int limit) {
    return impl_->getLog(limit);
}

}  // namespace lithium
